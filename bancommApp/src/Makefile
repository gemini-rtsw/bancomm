TOP=../..
include $(TOP)/configure/CONFIG

# Reference the parent DBD for this support
DBD += bancommDev.dbd
# bancomm.dbd will be built from these files:
bancomm_DBD  += base.dbd
bancomm_DBD  += bancommDev.dbd

bancommSupp_DBD += stdRecords.dbd
bancommSupp_DBD += bancommDev.dbd

# Don't build for Linux
#LIBRARY_IOC_Linux = -nil-
#LIBRARY_IOC_DEFAULT += bancomm
LIBRARY_IOC_RTEMS += bancomm

# The -fvolatile switch doesn't work on modern versions of GCC
# Need to declare hardware registers as 'volatile' in the code.
#USR_CFLAGS   += -O0
#USR_CFLAGS   += -std=c99 --pedantic
USR_CPPFLAGS += -DUSE_TYPED_RSET

#DBD          += bancommSupp.dbd
DBD          += bancomm.dbd

# bancommSupp.dbd will be built from these files:
#bancommSupp_DBD  += bancommDev.dbd

# bancomm.dbd will be built from these files:
#bancomm_DBD  += $(bancommSupp_DBD)
#bancomm_DBD  += base.dbd
#bancomm_DBD  += bancommDev.dbd

#bancommSupp_DBD  += base.dbd
#bancommSupp_DBD  += bancommDev.dbd

bancomm_SRCS += osdClockFuncs.c
bancomm_SRCS += devBc635.c
bancomm_SRCS += drvBc635.c
bancomm_LIB += $(EPICS_BASE_IOC_LIBS)

INC          += bc635.h
INC          += osdClockFuncs.h

include $(TOP)/configure/RULES
