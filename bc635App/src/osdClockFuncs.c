#include "osdClockFuncs.h"


void sysClockOn(void)
{
#ifdef RTEMS
   clockOn(NULL);
#endif

#ifdef vxWorks
   sysClkEnable();
#endif 
}

void sysClockOff(void)
{
#ifdef RTEMS
   clockOff(NULL);
#endif

#ifdef vxWorks
   sysClkDisable();
#endif
}


void clock_tick(void) 
{
#ifdef RTEMS
   rtems_clock_tick();
#endif

#ifdef vxWorks
   tickAnnounce();
#endif
}


int clock_rate_get(void) 
{
#ifdef RTEMS
   rtems_interval ticksPerSecond;
   rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticksPerSecond);
   return (double)ticksPerSecond;
#else
#ifdef vxWorks
   return sysClkRateGet();
#else
return -1;
#endif
#endif
}

void clock_rate_set(int rate)
{
#ifdef vxWorks
   sysClkRateSet(rate);
#endif
}
