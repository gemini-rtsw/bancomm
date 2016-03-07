

#ifndef BCCLOCK_H
#define BCCLOCK_H

#ifdef vxWorks
#include <sysLib.h>
#include <tickLib.h>
#endif

#ifdef RTEMS
#include <rtems.h>
/*#include <rtems/clockdrv.h>*/
#include <libcpu/c_clock.h>
#endif

extern void sysClockOn(void);
extern void sysClockOff(void);
extern void clock_tick(void);
extern int clock_rate_get(void);
extern void clock_rate_set(int);


#endif /* BCCLOCK_H */
