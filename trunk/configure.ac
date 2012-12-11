AC_INIT(neweraHPC, 2.90, varunmittal91@gmail.com)

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])
AM_CONFIG_HEADER(config.h)

AC_CONFIG_SRCDIR([server.cpp])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([pthread],[pthread_create],[],[AC_MSG_ERROR([libpthread not found])])

# Checks for header files.
AC_CHECK_HEADERS([pthread.h])
AC_CHECK_HEADERS([dlfcn.h])

# Check for optional arguments
AC_ARG_ENABLE([garbage_collector], [Enable garbage collector], [AC_DEFINE([ENABLE_GARBAGE_COLLECTOR], [1], [Enable garbage collector])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
LT_INIT()
LT_SUPPORTED_TAG(CXX)

AC_OUTPUT([Makefile]
          [src/Makefile]
          [src/general/Makefile]
	  [src/network/Makefile]
          [src/network/web_server/Makefile]
	  [src/poll/Makefile]
          [src/ai/Makefile]
          [src/grid/Makefile]
          [src/system/Makefile]
          [tests/Makefile]
	  [tests/general/Makefile]
          [tests/network/Makefile]
          [tests/grid/Makefile])