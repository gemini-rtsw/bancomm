
#include "osdClockFuncs.h"

#if defined (__rtems__)
extern rtems_configuration_table Configuration;
#endif


void sysClockOn(void)
{
#if defined (__rtems__)
   BSP_connect_clock_handler();
#endif
}

void sysClockOff(void)
{
#if defined  (__rtems__)
   BSP_disconnect_clock_handler();
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
   return (int)ticksPerSecond;
#else
return -1;
#endif
}

int clock_rate_set(int  rate)
{
#if defined (__rtems__)
   Configuration.microseconds_per_tick =  1000000 / rate;
#endif
   return 0;
}
