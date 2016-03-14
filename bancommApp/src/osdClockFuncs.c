#include "bc635.h"

void sysClockOn(void)
{
#if defined (__rtems__)
   clockOn(NULL);
#endif
}

void sysClockOff(void)
{
#if defined  (__rtems__)
   clockOff(NULL);
#endif
}


void clock_tick(void) 
{
#if defined (__rtems__)
   rtems_clock_tick();
#endif
}


int clock_rate_get(void) 
{
#if defined (__rtems__)
   rtems_interval ticksPerSecond;
   rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticksPerSecond);
   return (double)ticksPerSecond;
#else
return -1;
#endif
}

int clock_rate_set(int rate)
{
return OK;
}
