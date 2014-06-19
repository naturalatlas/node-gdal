/* expat_config.h.in.  Generated from configure.in by autoheader.  */

#define XML_USE_MSC_EXTENSIONS 1

/* 1234 = LIL_ENDIAN, 4321 = BIGENDIAN */
#ifdef CPU_LITTLE_ENDIAN
#define BYTEORDER 1234
#else
#define BYTEORDER 4321
#endif

/* Define to 1 if you have the `bcopy' function. */
// #define HAVE_BCOPY

/* Define to 1 if you have the <dlfcn.h> header file. */
// #define HAVE_DLFCN_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
// #define HAVE_FCNTL_H 1

/* Define to 1 if you have the `getpagesize' function. */
// #define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the <inttypes.h> header file. */
// #define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have a working `mmap' system call. */
#define HAVE_MMAP 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
// #define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
// #define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
// #define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
// #define HAVE_UNISTD_H 1

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "expat-bugs@libexpat.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "expat"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "expat 2.1.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "expat"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.1.0"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* whether byteorder is bigendian */

#ifdef CPU_BIG_ENDIAN
#define WORDS_BIGENDIAN
#endif

/* Define to specify how much context to retain around the current parse
   point. */
// #define XML_CONTEXT_BYTES @XML_CONTEXT_BYTES@

/* Define to make parameter entity parsing functionality available. */
#define XML_DTD

/* Define to make XML Namespaces functionality available. */
#define XML_NS

/* Define to __FUNCTION__ or "" if `__func__' does not conform to ANSI C. */
#ifdef _MSC_VER
# define __func__ __FUNCTION__
#endif

/* Define to `long' if <sys/types.h> does not define. */
// #define off_t @OFF_T@

/* Define to `unsigned' if <sys/types.h> does not define. */
// #define size_t @SIZE_T@
