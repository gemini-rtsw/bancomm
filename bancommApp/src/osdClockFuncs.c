
#include "osdClockFuncs.h"

#if defined (__rtems__)
#include <rtems.h>
#include <bsp.h>

extern rtems_configuration_table Configuration;
extern rtems_interval rtemsTicksPerSecond;
extern double rtemsTicksPerSecond_double;
extern double rtemsTicksPerTwoSeconds_double;
#endif

#if defined (vxWorks)
#include <sysLib.h>
#include <tickLib.h>
#endif


void sysClockOn(void)
{
#if defined (__rtems__)
   BSP_connect_clock_handler();
#endif

#if defined (vxWorks)
   sysClkEnable();
#endif
}

void sysClockOff(void)
{
#if defined  (__rtems__)
   BSP_disconnect_clock_handler();
#endif

#if defined (vxWorks)
   sysClkDisable();
#endif
}


void clock_tick(void) 
{
#if defined (__rtems__)
   rtems_clock_tick();
#endif

#if defined (vxWorks)
   tickAnnounce();
#endif
}

int clock_rate_get(void) 
{
#if defined (__rtems__)
   rtems_interval ticksPerSecond;
   rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticksPerSecond);
   return (int)ticksPerSecond;
#endif
#if defined (vxWorks)
   return sysClkRateGet();
#endif
return -1;
}

int clock_rate_set(int  rate)
{

#if defined (__rtems__)
   Configuration.microseconds_per_tick =  1000000 / rate;
   rtems_clock_get (RTEMS_CLOCK_GET_TICKS_PER_SECOND, &rtemsTicksPerSecond);
   rtemsTicksPerSecond_double = rtemsTicksPerSecond;
   rtemsTicksPerTwoSeconds_double = rtemsTicksPerSecond_double * 2.0;
#endif

#if defined (vxWorks)
   sysClkRateSet(rate);
#endif

   return 0;
}
