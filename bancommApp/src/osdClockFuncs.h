

#ifndef BCCLOCK_H
#define BCCLOCK_H

#ifdef RTEMS
#include <rtems.h>
#include <libcpu/c_clock.h>
#endif

extern void sysClockOn(void);
extern void sysClockOff(void);
extern void clock_tick(void);
extern int clock_rate_get(void);
extern void clock_rate_set(int);


#endif /* BCCLOCK_H */
