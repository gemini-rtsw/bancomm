TOP=../..
include $(TOP)/configure/CONFIG

LIBRARY_IOC_RTEMS += bancomm
bancomm_LIBS        += $(EPICS_BASE_IOC_LIBS)

# The -fvolatile switch doesn't work on modern versions of GCC
# Need to declare hardware registers as 'volatile' in the code.
#USR_CFLAGS   += -fvolatile

DBD          += bancomm.dbd
bancomm_DBD  += base.dbd
bancomm_DBD  += bancommDev.dbd

bancomm_SRCS += osdClockFuncs.c
bancomm_SRCS += devBc635.c
bancomm_SRCS += drvBc635.c

INC          += bc635.h
INC          += osdClockFuncs.h

include $(TOP)/configure/RULES
