/*-
 * Copyright (c) 2015-2018 Ruslan Bukin <br@bsdpad.com>
 * All rights reserved.
 *
 * Portions of this software were developed by SRI International and the
 * University of Cambridge Computer Laboratory under DARPA/AFRL contract
 * FA8750-10-C-0237 ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * Portions of this software were developed by the University of Cambridge
 * Computer Laboratory as part of the CTSRD Project, with support from the
 * UK Higher Education Innovation Fund (HEIF).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/limits.h>
#include <sys/proc.h>
#include <sys/sf_buf.h>
#include <sys/signal.h>
#include <sys/sysent.h>
#include <sys/unistd.h>

#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>
#include <vm/uma.h>
#include <vm/uma_int.h>

#include <machine/riscvreg.h>
#include <machine/cpu.h>
#include <machine/pcb.h>
#include <machine/frame.h>
#include <machine/sbi.h>

/* sizeof(struct tcb) */
#define	TP_OFFSET	(2 * sizeof(void * __capability))
#ifdef COMPAT_FREEBSD64
#define	TP_OFFSET64	(2 * sizeof(uint64_t))
#endif

/*
 * Finish a fork operation, with process p2 nearly set up.
 * Copy and update the pcb, set up the stack so that the child
 * ready to run and return to user mode.
 */
void
cpu_fork(struct thread *td1, struct proc *p2, struct thread *td2, int flags)
{
	struct pcb *pcb2;
	struct trapframe *tf;
	char *p;

	if ((flags & RFPROC) == 0)
		return;

	/* RISCVTODO: save the FPU state here */

	pcb2 = (struct pcb *)(td2->td_kstack +
	    td2->td_kstack_pages * PAGE_SIZE) - 1;

	td2->td_pcb = pcb2;
	bcopy(td1->td_pcb, pcb2, sizeof(*pcb2));

#if __has_feature(capabilities)
	p2->p_md.md_sigcode = td1->td_proc->p_md.md_sigcode;
#endif

	/*
	 * The "top-most" trapframe uses a "hole" between the pcb and
	 * trapframe to save the kernel's tp register used for per-CPU
	 * data.  Leave a gap for the "hole" then align the stack and
	 * use that as the top of the trapframe.
	 */
#if __has_feature(capabilities)
	p = (char *)pcb2 - sizeof(register_t);
#else
	p = (char *)pcb2 - sizeof(uintcap_t);
#endif
	tf = (struct trapframe *)STACKALIGN(p) - 1;
	bcopy(td1->td_frame, tf, sizeof(*tf));

	/* Clear syscall error flag */
	tf->tf_t[0] = 0;

	/* Arguments for child */
	tf->tf_a[0] = 0;
	tf->tf_a[1] = 0;
	tf->tf_sstatus |= (SSTATUS_SPIE); /* Enable interrupts. */
	tf->tf_sstatus &= ~(SSTATUS_SPP); /* User mode. */

	td2->td_frame = tf;

	/* Set the return value registers for fork() */
	td2->td_pcb->pcb_s[0] = (uintptr_t)fork_return;
	td2->td_pcb->pcb_s[1] = (uintptr_t)td2;
	td2->td_pcb->pcb_ra = (uintptr_t)fork_trampoline;
	td2->td_pcb->pcb_sp = (uintptr_t)td2->td_frame;

	/* Setup to release spin count in fork_exit(). */
	td2->td_md.md_spinlock_count = 1;
	td2->td_md.md_saved_sstatus_ie = (SSTATUS_SIE);
}

void
cpu_reset(void)
{

	sbi_system_reset(SBI_SRST_TYPE_COLD_REBOOT, SBI_SRST_REASON_NONE);

	while(1);
}

void
cpu_thread_swapin(struct thread *td)
{
}

void
cpu_thread_swapout(struct thread *td)
{
}

void
cpu_set_syscall_retval(struct thread *td, int error)
{
	struct trapframe *frame;

	frame = td->td_frame;

	if (__predict_true(error == 0)) {
		frame->tf_a[0] = td->td_retval[0];
		frame->tf_a[1] = td->td_retval[1];
		frame->tf_t[0] = 0;		/* syscall succeeded */
		return;
	}

	switch (error) {
	case ERESTART:
		frame->tf_sepc -= 4;		/* prev instruction */
		break;
	case EJUSTRETURN:
		break;
	default:
		frame->tf_a[0] = error;
		frame->tf_t[0] = 1;		/* syscall error */
		break;
	}
}

/*
 * Initialize machine state, mostly pcb and trap frame for a new
 * thread, about to return to userspace.  Put enough state in the new
 * thread's PCB to get it to go back to the fork_return(), which
 * finalizes the thread state and handles peculiarities of the first
 * return to userspace for the new thread.
 */
void
cpu_copy_thread(struct thread *td, struct thread *td0)
{

	bcopy(td0->td_frame, td->td_frame, sizeof(struct trapframe));
	bcopy(td0->td_pcb, td->td_pcb, sizeof(struct pcb));

	td->td_pcb->pcb_s[0] = (uintptr_t)fork_return;
	td->td_pcb->pcb_s[1] = (uintptr_t)td;
	td->td_pcb->pcb_ra = (uintptr_t)fork_trampoline;
	td->td_pcb->pcb_sp = (uintptr_t)td->td_frame;

	/* Setup to release spin count in fork_exit(). */
	td->td_md.md_spinlock_count = 1;
	td->td_md.md_saved_sstatus_ie = (SSTATUS_SIE);
}

/*
 * Set that machine state for performing an upcall that starts
 * the entry function with the given argument.
 */
void
cpu_set_upcall(struct thread *td, void (* __capability entry)(void *),
    void * __capability arg, stack_t *stack)
{
	struct trapframe *tf;

	tf = td->td_frame;

	tf->tf_sp = STACKALIGN((uintcap_t)stack->ss_sp + stack->ss_size);
#if __has_feature(capabilities)
	if (SV_PROC_FLAG(td->td_proc, SV_CHERI) == 0) {
		tf->tf_sp = (uintcap_t)(__cheri_addr ptraddr_t)tf->tf_sp;
		hybridabi_thread_setregs(td, (__cheri_addr unsigned long)entry);
	} else
#endif
		tf->tf_sepc = (uintcap_t)entry;
	tf->tf_a[0] = (uintcap_t)arg;
}

int
cpu_set_user_tls(struct thread *td, void * __capability tls_base)
{

	if ((__cheri_addr ptraddr_t)tls_base >= VM_MAXUSER_ADDRESS)
		return (EINVAL);

	/*
	 * The user TLS is set by modifying the trapframe's tp value, which
	 * will be restored when returning to userspace.
	 */
#ifdef COMPAT_FREEBSD64
	if (SV_PROC_FLAG(td->td_proc, SV_CHERI | SV_LP64) == SV_LP64)
		td->td_frame->tf_tp = (__cheri_addr ptraddr_t)tls_base +
		    TP_OFFSET64;
	else
#endif
		td->td_frame->tf_tp = (uintcap_t)tls_base + TP_OFFSET;

	return (0);
}

void
cpu_thread_exit(struct thread *td)
{
}

void
cpu_thread_alloc(struct thread *td)
{
	char *p;

	td->td_pcb = (struct pcb *)(td->td_kstack +
	    td->td_kstack_pages * PAGE_SIZE) - 1;

	/* See comment in cpu_fork(). */
#if __has_feature(capabilities)
	p = (char *)td->td_pcb - sizeof(register_t);
#else
	p = (char *)td->td_pcb - sizeof(uintcap_t);
#endif
	td->td_frame = (struct trapframe *)STACKALIGN(p) - 1;
}

void
cpu_thread_free(struct thread *td)
{
}

void
cpu_thread_clean(struct thread *td)
{
}

/*
 * Intercept the return address from a freshly forked process that has NOT
 * been scheduled yet.
 *
 * This is needed to make kernel threads stay in kernel mode.
 */
void
cpu_fork_kthread_handler(struct thread *td, void (*func)(void *), void *arg)
{

	td->td_pcb->pcb_s[0] = (uintptr_t)func;
	td->td_pcb->pcb_s[1] = (uintptr_t)arg;
	td->td_pcb->pcb_ra = (uintptr_t)fork_trampoline;
	td->td_pcb->pcb_sp = (uintptr_t)td->td_frame;
}

void
cpu_exit(struct thread *td)
{
}

bool
cpu_exec_vmspace_reuse(struct proc *p __unused, vm_map_t map __unused)
{

	return (true);
}

int
cpu_procctl(struct thread *td __unused, int idtype __unused, id_t id __unused,
    int com __unused, void * __capability data __unused)
{

	return (EINVAL);
}
