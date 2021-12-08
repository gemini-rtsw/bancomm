TOP=../..
include $(TOP)/configure/CONFIG

# Reference the parent DBD for this support
DBD	+= bancommDev.dbd

LIBRARY_IOC_RTEMS += bancomm

# The -fvolatile switch doesn't work on modern versions of GCC
# Need to declare hardware registers as 'volatile' in the code.
#USR_CFLAGS   += -O0
#USR_CFLAGS   += -std=c99 --pedantic
USR_CPPFLAGS += -DUSE_TYPED_RSET

bancomm_SRCS += osdClockFuncs.c
bancomm_SRCS += devBc635.c
bancomm_SRCS += drvBc635.c

bancomm_LIB += $(EPICS_BASE_IOC_LIBS)

INC          += bc635.h
INC          += osdClockFuncs.h

include $(TOP)/configure/RULES
