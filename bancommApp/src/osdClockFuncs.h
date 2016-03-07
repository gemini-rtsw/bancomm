/* osdClocksFuncs.h
 * Header file for wrapper functions to operating system dependant clock functions
 * Created 4 Mar 2016, mdw
 */

#ifndef OSDCLOCKFUNCS_H
#define OSDCLOCKFUNCS_H

#ifdef RTEMS
#include <rtems.h>
#include <libcpu/c_clock.h>
#endif

extern void sysClockOn(void);
extern void sysClockOff(void);
extern void clock_tick(void);
extern int clock_rate_get(void);
extern void clock_rate_set(int);


#endif /* OSDCLOCKFUNCS_H */
