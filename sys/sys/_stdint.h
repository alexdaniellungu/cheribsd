/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2011 David E. O'Brien <obrien@FreeBSD.org>
 * Copyright (c) 2001 Mike Barcroft <mike@FreeBSD.org>
 * All rights reserved.
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
 *
 * $FreeBSD$
 */

#ifndef _SYS__STDINT_H_
#define _SYS__STDINT_H_

#ifndef _INT8_T_DECLARED
typedef	__int8_t		int8_t;
#define	_INT8_T_DECLARED
#endif

#ifndef _INT16_T_DECLARED
typedef	__int16_t		int16_t;
#define	_INT16_T_DECLARED
#endif

#ifndef _INT32_T_DECLARED
typedef	__int32_t		int32_t;
#define	_INT32_T_DECLARED
#endif

#ifndef _INT64_T_DECLARED
typedef	__int64_t		int64_t;
#define	_INT64_T_DECLARED
#endif

#ifndef _UINT8_T_DECLARED
typedef	__uint8_t		uint8_t;
#define	_UINT8_T_DECLARED
#endif

#ifndef _UINT16_T_DECLARED
typedef	__uint16_t		uint16_t;
#define	_UINT16_T_DECLARED
#endif

#ifndef _UINT32_T_DECLARED
typedef	__uint32_t		uint32_t;
#define	_UINT32_T_DECLARED
#endif

#ifndef _UINT64_T_DECLARED
typedef	__uint64_t		uint64_t;
#define	_UINT64_T_DECLARED
#endif

#ifndef _INTCAP_T_DECLARED
typedef	__intcap_t		intcap_t;
#define	_INTCAP_T_DECLARED
#endif
#ifndef _UINTCAP_T_DECLARED
typedef	__uintcap_t		uintcap_t;
#define	_UINTCAP_T_DECLARED
#endif
#ifndef _INTPTR_T_DECLARED
typedef	__intptr_t		intptr_t;
#define	_INTPTR_T_DECLARED
#endif
#ifndef _UINTPTR_T_DECLARED
typedef	__uintptr_t		uintptr_t;
#define	_UINTPTR_T_DECLARED
#endif
#ifndef	_INT64PTR_T_DECLARED
typedef	__int64ptr_t            int64ptr_t;
#define	_INT64PTR_T_DECLARED
#endif
#ifndef _UINT64PTR_T_DECLARED
typedef	__uint64ptr_t           uint64ptr_t;
#define	_UINT64PTR_T_DECLARED
#endif
#ifndef _INTMAX_T_DECLARED
typedef	__intmax_t		intmax_t;
#define	_INTMAX_T_DECLARED
#endif
#ifndef _UINTMAX_T_DECLARED
typedef	__uintmax_t		uintmax_t;
#define	_UINTMAX_T_DECLARED
#endif
#ifndef _KINTCAP_T_DECLARED
#ifdef _KERNEL
typedef __intcap_t	kintcap_t;
#else
typedef __intptr_t	kintcap_t;
#endif
#define	_KINTCAP_T_DECLARED
#endif
#ifndef _KUINTCAP_T_DECLARED
#ifdef _KERNEL
typedef __uintcap_t	kuintcap_t;
#else
typedef __uintptr_t	kuintcap_t;
#endif
#define	_KUINTCAP_T_DECLARED
#endif
#ifndef _KUINT64CAP_T_DECLARED
#ifdef __ILP32__
typedef	uint64_t	kuint64cap_t;
#else
typedef	kuintcap_t	kuint64cap_t;
#endif
#define	_KUINT64CAP_T_DECLARED
#endif

#ifndef _PTRADDR_T_DECLARED
typedef	__ptraddr_t	ptraddr_t;
#define	_PTRADDR_T_DECLARED
#endif

#ifndef _VADDR_T_DECLARED
#ifndef __CHERI_PURE_CAPABILITY__
typedef	__uintptr_t		vaddr_t;
#else
typedef	__uint64_t		vaddr_t;
#endif
#define _VADDR_T_DECLARED
#endif

#endif /* !_SYS__STDINT_H_ */
// CHERI CHANGES START
// {
//   "updated": 20181121,
//   "target_type": "header",
//   "changes": [
//     "integer_provenance",
//     "virtual_address"
//   ]
// }
// CHERI CHANGES END
