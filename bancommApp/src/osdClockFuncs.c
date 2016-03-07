#include "osdClockFuncs.h"


void sysClockOn(void)
{
#ifdef RTEMS
   clockOn(NULL);
#endif
}

void sysClockOff(void)
{
#ifdef RTEMS
   clockOff(NULL);
#endif
}


void clock_tick(void) 
{
#ifdef RTEMS
   rtems_clock_tick();
#endif
}


int clock_rate_get(void) 
{
#ifdef RTEMS
   rtems_interval ticksPerSecond;
   rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticksPerSecond);
   return (double)ticksPerSecond;
#else
return -1;
#endif
}

void clock_rate_set(int rate)
{
}
