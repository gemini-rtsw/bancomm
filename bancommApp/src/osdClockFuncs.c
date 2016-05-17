
#include <epicsInterrupt.h>
#include "osdClockFuncs.h"

#if defined (__rtems__)
extern rtems_configuration_table Configuration;
extern rtems_interval rtemsTicksPerSecond;
extern double rtemsTicksPerSecond_double;
extern double rtemsTicksPerTwoSeconds_double;
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
//   int key;
//   key = epicsInterruptLock();

#if defined (__rtems__)
   Configuration.microseconds_per_tick =  1000000 / rate;
   rtems_clock_get (RTEMS_CLOCK_GET_TICKS_PER_SECOND, &rtemsTicksPerSecond);
   rtemsTicksPerSecond_double = rtemsTicksPerSecond;
   rtemsTicksPerTwoSeconds_double = rtemsTicksPerSecond_double * 2.0;
#endif

   NTPTimeUpdateTickRate();            /* inform NTP time provider of updated tick rate */
//   epicsInterruptUnlock(key);
   return 0;
}
