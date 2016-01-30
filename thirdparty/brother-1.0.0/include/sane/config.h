/* include/sane/config.h.  Generated automatically by configure.  */
#ifndef SANE_CONFIG_H
#define SANE_CONFIG_H

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#define HAVE_ALLOCA_H 1

/* Define if you have a working `mmap' system call.  */
#define HAVE_MMAP 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Do we need <sys/bitypes.h>? */
/* #undef NEED_SYS_BITYPES_H */

#ifdef NEED_SYS_BITYPES_H
# include <sys/bitypes.h>
#else /* not NEED_SYS_BITYPES_H */

/* Define to `unsigned char' if <sys/types.h> doesn't define.  */
/* #undef u_char */

/* Define to `unsigned int' if <sys/types.h> doesn't define.  */
/* #undef u_int */

/* Define to `unsigned long' if <sys/types.h> doesn't define.  */
/* #undef u_long */

/* Define to `unsigned char' if <sys/types.h> doesn't define. */
/* #undef u_int8_t */

/* Define to `unsigned short' if <sys/types.h> doesn't define. */
/* #undef u_int16_t */

/* Define to `unsigned int' if <sys/types.h> doesn't define. */
/* #undef u_int32_t */

#endif /* not NEED_SYS_BITYPES_H */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef ssize_t */

/* Define socklen_t as `int' if necessary.  */
/* #undef socklen_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define scsireq_t as `struct scsireq' if necessary.  */
/* #undef scsireq_t */

/* Define to the return type of signal handlers. */
#define RETSIGTYPE void

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if struct flock is available */
#define HAVE_STRUCT_FLOCK 1

/* Define if union semun is available */
/* #undef HAVE_UNION_SEMUN */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define to 1 if NLS is requested.  */
/* #undef ENABLE_NLS */

/* Define as 1 if you have catgets and don't want to use GNU gettext.  */
/* #undef HAVE_CATGETS */

/* Define as 1 if you have gettext and don't want to use GNU gettext.  */
/* #undef HAVE_GETTEXT */

/* Define if your locale.h file contains LC_MESSAGES.  */
/* #undef HAVE_LC_MESSAGES */

/* Define to 1 if you have the stpcpy function.  */
/* #undef HAVE_STPCPY */

/* Define to the name of the distribution.  */
#define PACKAGE "sane-backends"

/* The concatenation of the strings PACKAGE, "-", and VERSION.  */
#define PACKAGE_VERSION "sane-backends-1.0.7"

/* Define to the version of the distribution.  */
#define VERSION "1.0.7"

/* Define if you have the __argz_count function.  */
/* #undef HAVE___ARGZ_COUNT */

/* Define if you have the __argz_next function.  */
/* #undef HAVE___ARGZ_NEXT */

/* Define if you have the __argz_stringify function.  */
/* #undef HAVE___ARGZ_STRINGIFY */

/* Define if you have the dcgettext function.  */
/* #undef HAVE_DCGETTEXT */

/* Define if you have the getcwd function.  */
/* #undef HAVE_GETCWD */

/* Define if you have the getenv function.  */
#define HAVE_GETENV 1

/* Define if you have the getpagesize function.  */
#define HAVE_GETPAGESIZE 1

/* Define if you have the atexit function. */
#define HAVE_ATEXIT 1

/* Define if you have the ioperm function. */
#define HAVE_IOPERM 1

/* Define if you have the isfdtype function.  */
#define HAVE_ISFDTYPE 1

/* Define if you have the mkdir function.  */
#define HAVE_MKDIR 1

/* Define if you have the munmap function.  */
/* #undef HAVE_MUNMAP */

/* Define if you have the putenv function.  */
/* #undef HAVE_PUTENV */

/* Define if you have the scsireq_enter function.  */
/* #undef HAVE_SCSIREQ_ENTER */

/* Define if you have the sigprocmask function.  */
#define HAVE_SIGPROCMASK 1

/* Define if you have the setenv function.  */
/* #undef HAVE_SETENV */

/* Define if you have the setlocale function.  */
/* #undef HAVE_SETLOCALE */

/* Define if you have the stpcpy function.  */
/* #undef HAVE_STPCPY */

/* Define if you have the strcasecmp function.  */
#define HAVE_STRCASECMP 1

/* Define if you have the strchr function.  */
/* #undef HAVE_STRCHR */

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Is /dev/urandom available? */
#define HAVE_DEV_URANDOM 1

/* SCSI command buffer size */
#define SCSIBUFFERSIZE 131072

/* Define if you have the strncasecmp function.  */
#define HAVE_STRNCASECMP 1

/* Define if you have the strndup function.  */
#define HAVE_STRNDUP 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the strsep function.  */
#define HAVE_STRSEP 1

/* Define if you have the strtod function.  */
#define HAVE_STRTOD 1

/* Define if you have the valloc function.  */
/* #undef HAVE_VALLOC */

/* Define if you have the vsyslog function.  */
#define HAVE_VSYSLOG 1

/* Define if you have the inet_ntop function.  */
#define HAVE_INET_NTOP 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Ignore HAVE_USLEEP under Apollo Domain because the usleep()
   implementation in the Sys5.3 environment is broken.  */
#ifndef apollo
  /* Define if you have the usleep function.  */
# define HAVE_USLEEP 1
#endif

/* Define if you have the <argz.h> header file.  */
/* #undef HAVE_ARGZ_H */

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <libintl.h> header file.  */
/* #undef HAVE_LIBINTL_H */

/* Define if you have the <libc.h> header file.  */
/* #undef HAVE_LIBC_H */

/* Define if you have the <limits.h> header file.  */
/* #undef HAVE_LIMITS_H */

/* Define if you have the <locale.h> header file.  */
/* #undef HAVE_LOCALE_H */

/* Define if you have the <malloc.h> header file.  */
/* #undef HAVE_MALLOC_H */

/* Define if you have the <nl_types.h> header file.  */
/* #undef HAVE_NL_TYPES_H */

/* Define if you have the <string.h> header file.  */
/* #undef HAVE_STRING_H */

/* Define if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/shm.h> header file. */
#define HAVE_SYS_SHM_H 1

/* Define if you have the <sys/io.h> header file. */
#define HAVE_SYS_IO_H 1

/* Define if you have the <asm/io.h> header file. */
#define HAVE_ASM_IO_H 1

/* Define if you have the <scsi.h> header file. */
/* #undef HAVE_SCSI_H */

/* Define if you have the <scsi/sg.h> header file. */
#define HAVE_SCSI_SG_H 1

/* Define if you have the "/usr/src/linux/include/scsi/sg.h" header file. */
/* #undef HAVE__USR_SRC_LINUX_INCLUDE_SCSI_SG_H */

/* Define if you have the <sys/dsreq.h> header file. */
/* #undef HAVE_SYS_DSREQ_H */

/* Define if you have the <sys/scsi.h> header file. */
/* #undef HAVE_SYS_SCSI_H */

/* Define if you have the <sys/sdi_comm.h> header file. */ 
/* #undef HAVE_SYS_SDI_COMM_H */ 

/* Define if you have the <sys/passthrudef.h.h> header file. */ 
/* #undef HAVE_SYS_PASSTHRUDEF_H */ 

/* Define if you have the <sys/scsi/targets/scgio.h> header file. */
/* #undef HAVE_SYS_SCSI_TARGETS_SCGIO_H */

/* Define if you have the <sys/scsi/sgdefs.h> header file. */
/* #undef HAVE_SYS_SCSI_SGDEFS_H */

/* Define if you have the <sys/scsicmd.h> header file. */
/* #undef HAVE_SYS_SCSICMD_H */

/* Define if you have the <sys/scsiio.h> header file. */
/* #undef HAVE_SYS_SCSIIO_H */

/* Define if you have the <sys/scanio.h> header file. */
/* #undef HAVE_SYS_SCANIO_H */

/* Define if you have the <apollo/scsi.h> header file. */
/* #undef HAVE_APOLLO_SCSI_H */

/* Define if you have the <bsd/dev/scsireg.h> header file. */
/* #undef HAVE_BSD_DEV_SCSIREG_H */

/* Define if you have the <io/cam/cam.h> header file. */
/* #undef HAVE_IO_CAM_CAM_H */

/* Define if you have the <camlib.h> header file. */
/* #undef HAVE_CAMLIB_H */

/* Define if you have the <gscdds.h> header file. */
/* #undef HAVE_GSCDDS_H */

/* Define if you have the <os2.h> header file.  */
/* #undef HAVE_OS2_H */

/* Define if you have the <linux/ppdev.h> header file.  */
#define HAVE_LINUX_PPDEV_H 1

/* Define if you have the <linux/usb.h> header file.  */
#define HAVE_USB_H 1

/* Define if you have EMX's sys/hw.h headers. */
/* #undef HAVE_SYS_HW_H */

/* Define if you have sys/types.h. OS/2 wants them before select.h, etc. */
#define HAVE_SYS_TYPES_H 1

/* Define if you have sys/scsi/scsi.h. */
/* #undef HAVE_SYS_SCSI_SCSI_H */

/* Define if you have sys/bitypes.h. */
#define HAVE_SYS_BITYPES_H 1

/* Define if you have sys/sem.h. */
#define HAVE_SYS_SEM_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <values.h> header file.  */
/* #undef HAVE_VALUES_H */

/* Define if you have the i library (-li).  */
/* #undef HAVE_LIBI */

/* Define if you have the intl library (-lintl).  */
/* #undef HAVE_LIBINTL */

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Define if you have the <dlfcn.h> header file.  */
#define HAVE_DLFCN_H 1

/* Define if you have the dlopen function.  */
#define HAVE_DLOPEN 1

/* Define if you have the <dl.h> header file.  */
/* #undef HAVE_DL_H */

/* Define if you have the shl_load function.  */
/* #undef HAVE_SHL_LOAD */

/* Define if you have the POSIX routine tcsendbreak().  */
#define HAVE_TCSENDBREAK 1

/* Define if you have cfmakeraw() */
#define HAVE_CFMAKERAW 1

/* Define if you have PTAL. */
/* #undef HAVE_PTAL */

/* Version of the dll backend (=version of the sane-backends package) */
#define SANE_DLL_V_MAJOR 1
#define SANE_DLL_V_MINOR 0
#define SANE_DLL_V_BUILD 7

#ifndef HAVE_STRNCASECMP
  /* OS/2 needs this */
# define strncasecmp(a, b, c) strnicmp(a, b, c)
#endif

#ifndef HAVE_STRCASECMP
  /* OS/2 needs this */
# define strcasecmp(a, b) stricmp(a, b)
#endif

#ifndef HAVE_STRDUP
  /* declare return type to avoid compile errors on 64-bit platforms */
  extern char *strdup ();
#endif

#ifndef HAVE_STRNDUP
  /* declare return type to avoid compile errors on 64-bit platforms */
  extern char *strndup ();
#endif

#ifndef HAVE_STRSEP
  /* declare return type to avoid compile errors on 64-bit platforms */
  extern char *strsep ();
#endif

#if defined (__sun) && defined (__GNUC__)
# define _POSIX_SOURCE
# define __EXTENSIONS__
#endif



#endif /* SANE_CONFIG_H */
