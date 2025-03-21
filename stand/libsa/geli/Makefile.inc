# $FreeBSD$
# Extra stuff for GELI

.PATH: ${SASRC}/geli

# Our password input method
SRCS+=		pwgets.c

# sha256 and sha512 from sys/crypto
.PATH: ${SYSDIR}/crypto/sha2
CFLAGS.sha256.c+=	-DWEAK_REFS
CFLAGS.sha512.c+=	-DWEAK_REFS
SRCS+=		sha256c.c sha512c.c

# md5 from libmd
.PATH: ${SRCTOP}/lib/libmd
SRCS+=		md5c.c

# AES implementation from sys/crypto
.PATH: ${SYSDIR}/crypto/rijndael
.for i in rijndael-alg-fst.c rijndael-api-fst.c rijndael-api.c
# Remove asserts XXX BAD
CFLAGS.${i}+=	-DNDEBUG
SRCS+=		${i}
.endfor

# local GELI Implementation
.PATH: ${SYSDIR}/geom/eli
.for i in geliboot.c geliboot_crypto.c gelidev.c geli_metadata.c
CFLAGS.${i}+=	-I${LDRSRC}
SRCS+=		${i}
.endfor
SRCS+=		\
		g_eli_hmac.c \
		g_eli_key.c \
		g_eli_key_cache.c \
		pkcs5v2.c \

# aes
.PATH:		${SYSDIR}/opencrypto
SRCS+=		xform_aes_xts.c
