/* $Id: drvBc635.c,v 1.1.1.1 2011/01/20 13:10:45 ajf Exp $ */
/*
 * Copyright 1996 Association of Universities for Research In Astronomy, Inc.
 * See the file COPYRIGHT for more details.
 *
 * FILENAME
 * drvBc635.c
 *
 * DESCRIPTION
 * This driver contains support routines for the Bancomm 635 and 637
 * VME time cards, for use both from EPICS and from a C subroutine
 *
 *INDENT-OFF*
 * $Log: drvBc635.c,v $
 * Revision 1.1.1.1  2011/01/20 13:10:45  ajf
 * Initial import of R3.14.12
 *
 * Revision 1.4  pbt
 * Changes for EPICS R3.14.8+ and Tornado 2.2:
 * Added includes string.h, errlog.h, epicsTime.h. 
 * Removed include drvTS.h
 * Defined VXWORKS EPOCH conversion constants
 * Replaced deprecated devConnectInterrupt with devConnectInterruptVME
 * ISR routine declared with void pointer argument
 * TSgetUnixTime() replaced by calls to epicsTime library routines
 *
 * $Log: drvBc635.c,v $
 * Revision 1.1.1.1  2011/01/20 13:10:45  ajf
 * Initial import of R3.14.12
 *
 * Revision 1.3  2005/02/02 16:41:17  ajf
 * Added routine: "setBcConfiguredOK"
 *
 * Revision 1.2  2004/07/05 15:58:12  ajf
 * Add .cvsignore file.
 * Convert the Bancomm device and driver support to 3.14
 *
 * Revision 1.1.1.1  2004/07/02 14:55:30  ajf
 * Gemini specific code
 *
 * Revision 1.1  2003/12/04 15:41:04  ajf
 * Import of the Gemini specific directory into GEM8.5.
 *
 * Revision 1.1.1.1  2003/07/04 09:41:14  ajf
 * Initial creation of the OSL EPICS 3.13.8 Repository
 *
 * Revision 1.1  2001/07/10 10:29:44  ajf
 * ajf: Added for the GEM7 release
 *
 *
 * 22/06/01 Foster
 * Fixed bug in bc635IntEnable. This routine must be able to be called twice.
 * Use static init flag.
 *
 * 24/02/99 Foster
 * Fixed bug in bcClkRateSet. Increase arry size of tfpstring to 16.
 *
 * Revision 1.3  13/04/98  Foster
 * Fixed compilation warnings for the MIPS processor.
 *
 * Revision 1.2  1997/07/11 18:34:20  goodrich
 * Fixed timeout bug in bcGetGpsLeap().
 *
 * Revision 1.1.1.1  1997/03/11 23:14:42  goodrich
 * Gem4
 *
 * Revision 1.1  1996/12/31 21:13:31  goodrich
 * Gem4
 *
 *INDENT-ON*
 *
 * MODIFICATION HISTORY
 * 01a,12oct94,pbt P.B. Taylor
 * 01b,10jan96,bdg changed bcRegsToTime to return TAI rather than GPS time.
 *                 changed bc635_set_strobe to set TAI rather than GPS time.
 * 01c,24jan96,bdg replaced TS_UNIX_TO_EPICS_EPOCH to
 *                 TS_VXWORKS_TO_EPICS_EPOCH.
 *                 replaced TS_1900_TO_UNIX_EPOCH to TS_1900_TO_VXWORKS_EPOCH.
 *                 added functions ErForceSync(), ErSyncEvent(),
 *                 ErDriverInit(), ErGetTime(), and ErDirectTime().
 * 01d,30dec96,bdg ErDirectTime() returns 1 if Bancomm configured OK.
 *                 ANSI-fied some functions.
 *
 */


#include <epicsStdio.h>
#include <epicsThread.h>
#include <epicsInterrupt.h>
#include <epicsTimer.h>
#include <epicsTime.h>
#include <epicsEvent.h>
#include <epicsExit.h>
#include <epicsMutex.h>
#include <epicsExport.h>
#include <generalTimeSup.h>
#include <iocsh.h>
#include <taskwd.h>
#include <devLib.h>
#include <string.h>
#include <drvSup.h>
#include <dbDefs.h>
#include <dbScan.h>
#include <errMdef.h>
#include <errlog.h>
#include <envDefs.h>

#include "bc635.h"
#include "osdClockFuncs.h"

#if( CPU_FAMILY == MIPS )
#define TS_1900_TO_UNIX_EPOCH_MIPS ((TS_SEC_IN_YEAR*70.0)+(17.0*TS_SEC_IN_DAY))
#endif

#define TS_1900_TO_UNIX_EPOCH 2208988800UL
#define TS_UNIX_TO_EPICS_EPOCH 631152000UL
#define BC635_ADDR0    0x4000
#define BC635VEC    0x40
#define BC635LVL    1
#define SOH 0x01
#define ETB 0x17

#define GPS_TO_TAI    19        /* Seconds between GPS and TAI time */
#define YEAR_MONITOR_SLEEP 3600        /* Interval (secs) between year monitoring */

/* Bit patterns for CMD register */
#define BC635CMD_1MHZ       0x80
#define BC635CMD_5MHZ       0x40
#define BC635CMD_10MHZ      0x00
#define BC635CMD_STRMODE    0x20    /* Set=minor time only */
#define BC635CMD_STREN      0x10    /* Set=Enable */
#define BC635CMD_EVENTEN    0x08    /* Set=Enable */
#define BC635CMD_EVSENSE    0x04    /* Set=Falling edge */
#define BC635CMD_HBEN       0x02    /* Set=Enable periodic time capture */
#define BC635CMD_LOCKEN     0x01    /* Set=Enable event lockout */

/*bc635 memory structure*/
typedef struct {
    epicsUInt16 dev_id;      /* VXIbus ID Register */
    epicsUInt16 dev_type;    /* VXIbus Device type */
    epicsUInt16 dev_csr;     /* VXIbus  Control & Status Register */
    epicsUInt16 res1[2];     /* Reserved */
    epicsUInt16 time_req;    /* Time Latch */
    epicsUInt8  time[10];    /* Requested Time */
    epicsUInt16 event[5];    /* Event/Strobe Time */
    epicsUInt16 unlock;      /* Release Lockout/Capture time */
    epicsUInt16 ack;         /* Acknowledge */
    epicsUInt16 cmd;         /* Command */
    epicsUInt16 fifo;        /* FIFO In/out */
    epicsUInt16 mask;        /* Interrupt masks */
    epicsUInt16 intstat;     /* Interrupt status */
    epicsUInt16 vector;      /* Interrupt vector */
    epicsUInt16 level;       /* Interrupt level */
    epicsUInt16 res2[8];     /* Reserved */
} volatile bc635Regs_t;

//static long report();
//static long init();
void bc635Time_Init(int);
int bc635Time_Report(int);
static int bc635TimeGetCurrent(epicsTimeStamp *);
int bc635TimeSetTpPrio(int); 

static int bcStartYearMonitor();

struct {
        long    number;
        DRVSUPFUN       report;
        DRVSUPFUN       init;
} drvBc635={
        2,
        bc635_report,
        bc635_init};
epicsExportAddress(drvet, drvBc635);

static bc635Regs_t  *pbc635 = NULL;   /* Pointer to bc635 register structure */
static IOSCANPVT ioscanpvt[4];        /* I/O scan list pointers */
static int bcThisIsMaster = 0;        /* Flag, id master = TRUE; default is FALSE/slave */
static int bcNoLeapSecs = 0;          /* FLAG : TRUE => DON'T use GPS leap secs */
static int bcOffset = 0;              /* Offset relative to input reference in microsecs */
static int HadPerint;                 /* Flag = TRUE when first periodic interrupt received */
static int sysClkRateWas;             /* Saved value of system clock */
static int tickFrequency;             /* System clock tick frequency */
static int bcUseper;                  /* Whether to use BC periodics for sys clock */
static int bcConfiguredOK = 0;        /* Whether have a Bancomm at all */
static int bcYearNumber = 42;         /* The current year number */
static int bcYearEpoch = 0;           /* The epoch (wrt 1970) of Jan 1, Oh UTC of current year */
static int bcLastEpoch = 0;           /* Last year's epoch, saved when year changes */
static int bcIntPerTick = 1;          /* Number of interrupts per clock tick */
static int bcIntCounter = 0;          /* Count all interrupts */
static int bcTickCnt = 0;             /* Count all interrupts */
static int altIntCounter = 0;	      /* Count alternate interrupts */

int bcYearMonitorStarted = 0;         /* Flag indicating Year Monitor task running */

static int bc635TimeTpPrio = 20;    /* Time provider priority, default=20, negative to disable bc635 time provider */

/* Declare pointer to user definable function called on each interrupt */
void (*bcUsrClock)(const int icount) = NULL;

/* Following two routines are wrap-arounds for the local report and init functions 
*  to provide standard calls from EPICS device support 
*/
#if 0
static long report(int level)
{
    bc635_report(level);
    return(0);
}

static long init()
{
    bc635_init();
    return(0);
}
#endif

/**********************************************************************************************************
*
* bcSendTfp - Send a control data packet to the Bancomm TFP
*
* Send a string to the bc635 output FIFO by writing
* successive bytes to the fifo address. This routine
* delimits the supplied string with <SOH>...<ETB>.      
* The Bancomm ACK register is set to 0x95 to cause TFP to 
* take action, then wait for TFP acknowledge bit set (0x01) 
*
* RETURNS:
* OK, or ERROR on time-out
*/
int bcSendTfp (char *charptr)    /* Character string containing the TFP data packet */
{
    int count = 0;
    pbc635->fifo = SOH;
    while (*charptr) pbc635->fifo = *charptr++;     /* write body of packet */
    pbc635->fifo = ETB;
    pbc635->ack = 0x95;                             /* command TFP and clear bits */

    /* wait for TFP acknowledge */
    while (!(pbc635->ack & 0x01) && (count < 200))
    {
        epicsThreadSleep(0.001);
        count++;
    }

    if (count == 200) 
    {
        printf("Timed out waiting for ACK in bcSendTfp\n");
        return ERROR;
    }
    else return OK;
}


/**********************************************************************************************************
*
* bcGetGpsLeap - Return the current value of GPS leap seconds.
*
* The current value of GPS leap seconds is obtained from a Bancomm
* 637 card by sending an O2 packet request to the TFP.
*
* The value returned is zero on a bc635 (non-GPS) card
*
* RETURNS:
* Integer number of seconds 
*/
int bcGetGpsLeap(void)
{
    int numsecs, j, count, ret = -1;
    unsigned char temp, message[3];

    bcSendTfp("O2");                                /* Send TFP info request for GPS leap data */
    for (j=0; j <200; j++)
    {
        epicsThreadSleep(0.001);
        if (pbc635->ack & 0x04) break;    /* Wait for output FIFO data */
         
        
        count = 0;
        while ((temp = pbc635->fifo) != 'o' && count < 200 ) /* Wait for 'o', start of reply */
        {
            epicsThreadSleep(0.001);
            count++;
        }

        if( count < 200 )
        {
            temp = pbc635->fifo;                            /* Skip next char */
            for (j=0; j < 2; j++)
                message[j] = pbc635->fifo;              /* load body of packet : 2 chars  */
            sscanf( (char *)message, "%2d", &numsecs);   /* Read leap second value into integer */
            ret = numsecs;
        }
   }
   return ret;                                 /* And return value */
}

/**********************************************************************************************************
*
* bcTestCard - test whether the Bancomm card is present.
*
* Probe the card address for a bus error
*
* RETURNS:
* OK if Bancomm present, ERROR if not (bus error)
*
* NOMANUAL
*/

int bcTestCard (void)
{
    short value;

/*
 * allow for reconfiguration of the addr map
 */
    if (pbc635 == NULL)    /* Not previously initialised? */
    {
        /* Convert bus address to local address */
        if( devRegisterAddress("bc635", atVMEA16, BC635_ADDR0, sizeof(bc635Regs_t), (void*)&pbc635) != OK)
        {
            pbc635 = NULL;
            return ERROR;
        }
    }

    /* Probe the card address for a bus error  */
    if (devReadProbe(sizeof(short), pbc635, &value) != OK)
    {
        pbc635 = NULL;
        return ERROR;
    }

    return OK; 
}



/**********************************************************************************************************
*
* bcHardwareSetup - Initialise Bancomm hardware
*
* Bancomm hardware setup, can be called prior to the standard Init routine
*
* NOMANUAL
*/
 
int bcHardwareSetup (void)
{
    if (pbc635 == NULL)    /* Not previously initialised? */
    {

        /* Convert and probe the card address for a bus error  */
        if (bcTestCard() != OK) 
            return ERROR;
        else
            bcConfiguredOK = 1;
     
        /* register initialization */
        pbc635->dev_csr = 0x01;        /* Control bit 0 clears registers at offset 20 thru 2E */
        pbc635->cmd     = BC635CMD_1MHZ;    /* 1MHz output */

        HadPerint = FALSE;        /* Flag to be set when a BC635 periodic interrupt occurs */
        bcUseper  = FALSE;        /* Flag  = whether to use BC periodics for sys clock */
        sysClkRateWas =  clock_rate_get();
        //sysClkRateWas = epicsThreadSleepQuantum();
        tickFrequency = sysClkRateWas;    /* Initial value for the clock tick frequency */
        printf("BC635 setup: initial clock tick frequency: %d\n", tickFrequency);

    }
    return OK;
}

/**********************************************************************************************************
*
* bc635_init - EPICS DRIVER INIT
*
* Initialize Bancomm bc635 Time & Frequency Card.
* called from iocInit
* 
* RETURNS:
* OK or ERROR if no Bancomm card present
*
* NOMANUAL
*/

long bc635_init()
{
    int status;
    char str_offset[9];            /* String for offset value */

    if (pbc635 == NULL)            /* Not previously setup? */
       if ((status = bcHardwareSetup()) != OK)
           return status;    

    /* Check whether this is master (i.e. BC637 GPS board.
     * If so, set to GPS mode. The value of the GPS leap seconds
     * should be zero for a bc635 (slave) and positive for a bc637 (master)
     */
    if (bcThisIsMaster)
    {
        bcSendTfp("A6");        /* Master - set mode to GPS */
        if ((bcGetGpsLeap()) == 0)    /* Strange if zero returned? */
            printf("Error in Master IOC Bancomm 637 : GPS failed or absent?\n");
        if ((bcGetGpsLeap()) < 0)
            printf("bcGetGpsLeap Timed out - no Leap Seconds returned\n");
    }
    else
    {
        bcSendTfp("A0");        /* Slave - set mode to IRIG-B input */
        if ((bcGetGpsLeap()) > 0)    /* Strange - may be this a bc637? */
            printf("Warning - slave IOC Bancomm card may be a bc637 ?\n");
        if ((bcGetGpsLeap()) < 0)
            printf("bcGetGpsLeap Timed out - no Leap Seconds returned\n");
    }
    if (!bcNoLeapSecs)
        bcSendTfp("P00");        /* Set mode for TFP processor */
                    /*  use GPS leap secs */
    else
        bcSendTfp("P20");        /* Set mode for TFP processor */
                    /* don't use GPS leap secs */

                    /* Set string for offset control */
    sprintf(str_offset,"G%+08d",bcOffset*10);
    bcSendTfp(str_offset);        /* Send packet G for offset control */
    bcSendTfp("M+00");            /* Time offset zero - */
                    /* display UTC unmodified */

     bcStartYearMonitor();  /* Start the year monitoring task */

    return OK;
} 

/*******************************************************************************
*
* itobcd - Int to BCD
*
* Return BCD character from input integer value
*
* RETURNS:
* unsigned char BCD character
*
* NOMANUAL
*/    
/* number = Integer value to be converted to BCD */
static unsigned char itobcd(int number)   
{
    return ( ((number/10) << 4) + (number%10 & 0xf) );
}

/**********************************************************************************************************
*
* bcdtoi - BCD to Int
*
* Return integer value from input BCD character
*
* RETURNS:
* integer equivalent of BCD character
*
* NOMANUAL
*/
static int bcdtoi (unsigned char digits)    /* BCD digits to be converted to integer */
{
   return ( (int) (digits & 0xf) + 10*((digits >> 4) & 0xf) );
}


/**********************************************************************************************************
*
* bc635_ioint_info - Look up I/O interrupt data structure
*
* Return the I/O scan list pointer corresponding to the
* signal (interrupt)  that has occurred 
*
* RETURNS: OK
*
* NOMANUAL
*/
long bc635_ioint_info (const epicsUInt16 signal, IOSCANPVT *ppvt)
{
    *ppvt = ioscanpvt[signal-1];
    return OK;
};

/**********************************************************************************************************
*
* isr_bc635 - Interrupt Service Routine for Bancomm 635.
*
* Check for each possible interrupt source indicated in
* the Bancomm interrupt status register (INTSTAT).
*
* For periodic interrupts set the clock rate for first interrupt
* at this rate, otherwise register a clock tick.
* For other types of interrupts, trigger an EPICS I/O scan
* request for the corresponding io_scan_list pointed to
* by ioscanpvt[]. Finally clear the interrupt bit.
*
* NOMANUAL
*/

void isr_bc635 (void *p)
{
    register short intnum;
    register epicsUInt16 tmask, bcistatus, bcimask;

    bcistatus = pbc635->intstat;            /* Save copy of status register */
    bcimask   = pbc635->mask;            /* Save copy of interrupt mask */

    /* First check for periodic interrupt */
    if ( ((bcistatus & 0x02) & bcimask) != 0)
    {
        if (bcUseper)                /* Using BC periodics for system clock? */
        {
            if (!HadPerint)            /* First Periodic Interrupt? */
            {
                HadPerint = TRUE;    

               //sysClockOff(); 

               //if(! (clock_rate_set(tickFrequency) == OK)) {

	       //   /*Trap something here on error, but this is an ISR... so caution.*/	       
	       //}
            }

            /* Increment interrupt count i.e., we actually got this interrupt*/
            bcIntCounter++; 

            /* Call user routine if it has been defined */
            if (bcUsrClock != NULL) (*bcUsrClock)(bcIntCounter);    

            if (bcIntCounter % bcIntPerTick == 0) {    /* If right number of ticks */
                bcTickCnt++;                             /* Increment tick announce count */
                clock_tick();                          /* Announce system clock tick */ 
            }
        }
        pbc635->intstat = pbc635->intstat | 0x02;    /* Clear interrupt status bit */
    }

    /* Other interrupts - not periodic pulse */
    for (intnum = 1; intnum < 5; intnum++)
    {
        if (intnum != 2)
        {

            tmask = 0x01 << (intnum - 1);        /* Test value for interrupt */
 
            /* Check for interrupt bit set - request I/O scan, but only if ioscanpvt is valid */
            if ( ((bcistatus & tmask) & bcimask) != 0)
            {
	    	altIntCounter++;
                if (ioscanpvt[intnum-1] != NULL)
                    scanIoRequest(ioscanpvt[intnum-1]);
                pbc635->intstat = pbc635->intstat | tmask;   /* Clear interrupt status bit */
            }
        }
    }
}



/**********************************************************************************************************
* 
* bc635_set_strobe - Set the time conincidence strobe to the input time
*
* Time is measured in seconds from midnight : times > 86400
* are taken modulo 86400.
*
* Method : Convert input time into hours, mins, secs and hundredths
* and ten-thousandths of a second. Set the BC635 STROBE1-3 registers 
*
* RETURNS: N/A
*
* NOMANUAL
*/
void bc635_set_strobe (const double stime)
{
    int itime, hours, mins, secs, hsec, tthsec;
    double fracsec;

    /* Break down the time into component fields for setting in the STROBE registers */
    itime = (int)stime;
    fracsec = stime - itime;    /* Extract the fractional part */
    itime = itime + 86400 - GPS_TO_TAI;    /* go from TAI to GPS - bdg */
    itime = itime%86400;        /* Reduce to time of day from midnight = zero secs */
    hours = itime/3600;        /* Extract each component of the time for the regs */
    mins = (itime - (hours * 3600))/60;
    secs = itime - ((hours * 3600) + (mins * 60));
    hsec = fracsec * 100;
    tthsec = (fracsec * 10000) - (hsec * 100); 
    printf ("Time interrupt set for %02d:%02d:%02d.%02d%02d\n",hours,mins,secs, hsec, tthsec);


    /* Set the STROBE registers with 16 bit writes (same offsets as corresponding EVENT registers) */
    pbc635->event[1] = (epicsUInt16) itobcd(hours);
    pbc635->event[2] = (epicsUInt16) (itobcd(mins)<<8) + itobcd(secs);
    pbc635->event[3] = (epicsUInt16) (itobcd(hsec)<<8) + itobcd(tthsec);
    pbc635->cmd |= (BC635CMD_STREN );   /* Enable 'major time mode' coincidence strobe */

}

/**********************************************************************************************************
* 
* bc635SetPeriod - Set the Bancomm periodic frequency rate
*
* The string to send to the TFP to set the periodic frequency is constructed
* here. The frequency in Hz is input, then the parameter N2 is computed 
* which is written out as a Hex string in tfpstring along with the 
* parameter N1, here fixed as 10000. The frequency is computed by :
*
*    Hz = 10,000,000/(N1 * N2) where   2 < {N1,N2} < 65536
*
* RETURNS: N/A
*
* NOMANUAL
*/
void bc635SetPeriod (const int hzval, const int intpertick, char *tfpstring)
{
    int n1, n2;
    double hzint, hztick;

    if (hzval == 0) return;                /* Non-event if zero frequency */
    /*    Need to compute parameter N2 required for setting periodic frequency */
    n1 = 1 + (int)(10000000.0/(hzval * 65535.0));
    if (n1 < 2 ) n1 = 2;
    n2 = (int)(10000000/(hzval * n1));    /* Calculate value of N2 for frequency */
    /* Mode F5 is for synchronous operation. In this mode the numbers supplied are one
       less than the N1/N2 factors used in computing the frequency. (See bc635 doc) */
    sprintf(tfpstring,"F5%04X%04X",(n1-1),(n2-1));    /* Write out the string to set frequency */
    bcSendTfp(tfpstring);                /* Write out to Bancomm TFP */
    /* Announce the actual frequency that was set - may not be precisely as requested */
    hzint = 10000000.0/(n1 * n2);
    printf("Bancomm interrupt frequency set to %7.3f Hz\n",hzint);
    /* If system clock rate is different, also announce */
    if (intpertick  < 2) hztick = hzint;
        else hztick = hzint/intpertick;
    printf("System clock frequency = %7.3f Hz\n",hztick);
}

/**********************************************************************************************************
* 
*  bc635IntEnable - Enable interrupts on the Bancomm 635 card.
*
* RETURNS :
* OK or Error (no Bancomm card or failed to connect)
*
* NOMANUAL
*/
/*  signal = Interrupt source to be enabled */
/*  parm = additional interrupt data */
int bc635IntEnable (const epicsUInt16 signal, const char *parm )
{
    static int init = FALSE;
    int status;
    epicsUInt16 intnum, imask, dummy;    /* Interrupt number and mask */

    if( !init )
    {
        /* Probe the card address for a bus error  */
        if (bcTestCard() != OK) return ERROR;

        /* Connect bc635 interrupts */
        if( (status = devConnectInterruptVME(BC635VEC, isr_bc635, NULL) ) != OK )
        {
            errMessage(status, "BC635 error - unable to connect\n");
            return status;
        }
        pbc635->vector = BC635VEC;      /* Interrupt vector */
        pbc635->level = BC635LVL;       /* Interrupt level */

        /* Enable interrupts on this level */
        status = devEnableInterruptLevel( intVME, BC635LVL );
        if( status )
            printf("Enabling Bancomm interrupts failed\n");
        init = TRUE;
    }

    /* Check type of signal (interrupt source) required */
    if (signal > 0)
    {
        intnum = (signal > 4) ? 4 : signal;    /* Limit signal to 4 */
        switch (intnum)
        {
            /* Signal 1 = External event input interrupt */
            case 1 :
                pbc635->cmd |= BC635CMD_EVENTEN | BC635CMD_LOCKEN;
                if (*parm == '-')
                    pbc635->cmd |= BC635CMD_EVSENSE;
                dummy = pbc635->unlock; /* Clear event capture lockout */
                printf("BC635 driver : external event interrupts enabled\n");
            break;

            /* Signal 2 = Periodic pulse interrupt. Used only for system clock ticks, so no action */
            case 2 :    
            break;

            /* Signal 3 = Time coincidence strobe interrupt. Get time from parm */
            case 3 : 
                printf("BC635 driver : time of day interrupts enabled\n");
            break;

            /* Signal 4 = 1PPS interrupt */
            case 4 :
                printf("BC635 driver : 1PPS interrupts enabled\n");
            break;

            default :
            break;
        }
        imask = 0x01 << (intnum - 1);        /* Corresponding interrupt mask */
        pbc635->mask    = pbc635->mask | imask;        /* Set interrupt mask */
        pbc635->intstat = pbc635->intstat | imask;    /* Clear interrupt status */

        /* Initialise EPICS I/O scan, but not for periodic pulse interrupts (signal 2) */
        if (signal != 2) scanIoInit(&ioscanpvt[intnum - 1]);
    }
    return OK;
}

/**********************************************************************************************************
* 
*  bc635_read - Read time
*
* Read the current time registers and return the time 
* as the number of seconds (type double value).
*
* RETURNS:
* OK or errors (no Bancomm or Bancomm status bit field)
*
*/ 
/* prval = Current time as a double precision real number */
int bc635_read (double *prval)
{
    epicsUInt16       dummy;
    static unsigned char stime[10];
    register int         lockKey;
    int                  i;

    *prval = 0.0;
    if (!bcConfiguredOK || bcYearEpoch == 0) return -1;        /* Forget it - no Bancomm */
    lockKey = epicsInterruptLock();            /* Disable interrupts */
    dummy = pbc635->time_req;        /* Latch time registers */
    for(i=0; i<10; i++) {
        stime[i] = pbc635->time[i];
    }
    epicsInterruptUnlock(lockKey);            /* Re-enable interrupts */
    
    return bcRegsToTime(prval, stime);    /* Finish off */
}


/**********************************************************************************************************
* 
*  bc635_event - Read event time
*
* Read the event time registers and return the time 
* as the number of seconds (type double value).
*
* RETURNS:
* OK or errors (no Bancomm or Bancomm status bit field)
*
*/ 
/* prval = Event time as a double precision real number */
int bc635_event (double *prval)
{

    epicsUInt16 dummy;
    static unsigned char etime[10];
    int i;

    if (!bcConfiguredOK || bcYearEpoch == 0) return -1;        /* Forget it - no Bancomm */

    /* Copy event time into etime array */
    /* memcpy(etime, &(pbc635->event[0]), 10);  */ /* memcpy() doesn't operate on  'volatile' data */ 
    for (i = 0; i<10; ++i) {
        etime[i] = pbc635->event[i];
    }

    dummy = pbc635->unlock;            /* Release lockout */
    return bcRegsToTime(prval, etime);    /* Finish off */
}

/**********************************************************************************************************
* 
*  bc635RegsToTime - Convert bc Time or Event regs to time
*
* Takes a mem copy of the EVENT or TIME regs and returns the time 
* as the number of seconds (type double value).
*
* RETURNS:
* OK or errors (no Bancomm or Bancomm status bit field)
*
* NOMANUAL
*/ 
/* prval = Current time as a double precision real number */
/* stime = register contents */
int bcRegsToTime (double *prval, unsigned char *stime)
{
    register epicsUInt16 days, hours, minutes, seconds;
    register unsigned long useconds;
    register int status = OK;
    static epicsUInt16 prevdaynum;


    days = (stime[1] & 0xf)*100 + bcdtoi(stime[2]);
    if (days == 0) days++;

    if (days == 1 && prevdaynum > 364)    /* Check for end of year */
    {
        bcYearNumber++;               /* Happy New Year! */
        bcLastEpoch = bcYearEpoch;    /* Save last year's Epoch */
        bcSetEpoch(bcYearNumber);     /* Set start of year epoch */
    }
    hours = bcdtoi(stime[3]);
    minutes = bcdtoi(stime[4]);
    useconds = (bcdtoi(stime[6]) * 100 + bcdtoi(stime[7])) * 100 +
    bcdtoi(stime[8]);
    seconds = bcdtoi(stime[5]);

    *prval = (days-1)*86400.0 
           + hours*3600.0 
           + minutes*60.0 
           + seconds*1.0 
           + ((double)useconds)/1000000;
    if (prevdaynum == 1 && days > 364)
        *prval += bcLastEpoch;        /* Event occurred last year! */
    else
        *prval += bcYearEpoch;

    prevdaynum = days;
    *prval += GPS_TO_TAI;        /* convert to GPS - bdg */

    /* Sanity check the time field values. Note 2 leap seconds permitted  */
    if (days > 366 || days == 0 || hours > 23 || minutes > 59 ||
            seconds > 61 || useconds > 999999)
    {
        status = -1;
        printf("days=%d, hours=%d, minutes=%d, seconds=%d, usecs=%ld\n",
         days, hours, minutes, seconds, useconds);
    }

    /* and check time status for errors :
     *  (Bit 4) the time is not locked to the synchronisation source 
     *  (Bit 5) time offset > 2 or 5 microsecs
     *  (Bit 6) Frequency offset is too large
     */
    else
        status = (stime[1] & 0x70) >> 4;

    return status;
}

/*******************************************************************************
* 
*  bc635TSread - Read time to struct timespec
*
* Return the time from the Bancomm as a timespec structure,
* EPICS epoch 1990 Jan 1, for use in EPICS Timestamp software 
*
* RETURNS:
* OK or error status from bc635_read()
*
* NOMANUAL
*/
int bc635TSread(struct timespec *sp)
{
    double btime;    /* Bancomm time as a double value */
    int bstatus;    /* Bancomm status return */

    bstatus = bc635_read(&btime);                /* Read current time from Bancomm */
    sp->tv_sec = (int)btime - TS_UNIX_TO_EPICS_EPOCH;    /* Adjusted seconds value is integer part */
    sp->tv_nsec = (btime - (int)btime) * 1000000000;    /* Nanoseconds from fractional part */
    return bstatus;
}


/**********************************************************************************************************
* 
*  bcClkRateSet - Set up periodic pulse frequency
*
* Value is Bancomm frequency in Hz.
* If input value is zero, re-enable the system clock
*
* RETURNS: ??
*
* NOMANUAL
*/
/* intPerTick = No. of Bancomm int's per system tick */
int bcClkRateSet (int intPerSecond, int intPerTick)
{
    char tfpstring[16];        /* String for setting periodic frequency */

    /* Set up periodic pulse frequency */
    if (intPerSecond > 0)
    {
        if (intPerTick < 2)
            bcIntPerTick = 1;
        else
            bcIntPerTick = intPerTick;

        bc635SetPeriod(intPerSecond, intPerTick, tfpstring);
                    /* Ensure interrupt source enabled */
        pbc635->mask = pbc635->mask | 0x02;
                     /* Clear interrupt status */
        pbc635->intstat = pbc635->intstat | 0x02;
                    /* Save tick frequency value */
        tickFrequency = intPerSecond/bcIntPerTick;
        HadPerint = FALSE;               /* Wait for interrupt flag */
        bcUseper = TRUE;                 /* Set flag to use BC periodics */
        sysClockOff();                   /* disable system clock */
        clock_rate_set(tickFrequency);   /* set system tick rate */
    }
    else
    {                    /* Input value was zero - */
                    /* re-enable system clock */

        sysClockOn(); 

                    /* Ensure interrupt source disabled */
        pbc635->mask = pbc635->mask & 0xFD;
                    /* Clear any interrupt status bit */
        pbc635->intstat = pbc635->intstat | 0x02;
        bcUseper = FALSE;        /* Clear flag to use BC periodics */
                    /* for sys clock */
        bcIntPerTick = 1;
        clock_rate_set(sysClkRateWas);    /* restore clock rate */
        printf("System clock re-enabled. Rate = %d Hz\n",clock_rate_get()); 
    }

    return OK;
}

/*******************************************************************************
* 
* bc635_write - Set time coincidence strobe
*
* For signal number 3 (set TOD interrupt) write out to the Bancomm as follows :-
*
* Set Time Of Day for time coincidence interrupt. Input value is time 
* for the interrupt (seconds since midnight).
*
* RETURNS:
*
* NOMANUAL
*/
int bc635_write (const epicsUInt16 signal, const double value)
{
    if (signal == 3)
    {
        /* Check value is in range between midnight (0) and 23h59m59.999s */
        if (value >= 86400.0 || value < 0.0) return -1;
        else
        /* Value is OK : set Time Of Day for time coincidence interrupt */
        {
            bc635_set_strobe(value);
            pbc635->mask = pbc635->mask | 0x04;    /* Ensure interrupt source enabled */
            pbc635->intstat = pbc635->intstat | 0x04; /* Clear interrupt status */
            return OK;
        }
    }
    else return -1;
}


/**********************************************************************************************************
* 
* bc635_report - Report Bancomm 635/637 status
*
* RETURNS: OK
*
* NOMANUAL
*/
long bc635_report (int level) 
{
    if (bcTestCard() != OK)
    {
        printf("Bancomm 635/637 card not found\n");
        return OK;
    }
    else
    {
        printf("Bancomm 635 board at address 0x%04X\n",BC635_ADDR0);
        if ( (pbc635->time[1] & 0x10) != 0)
            printf("TFP is flywheeling (not locked)\n");
        else
            printf("TFP is locked to selected reference\n");
        return OK;
    }
}

/**********************************************************************************************************
* 
* NTPgetTime - get time from NTP server
*
* Returns the current time in a *(struct tm) result,  by interrogating
* the NTP server. This is done using the TSdrv function TSgetUnixTime 
*
*  RETURNS: Error if fail to get NTP time
*
*  NOMANUAL
*/
int NTPgetTime (struct tm **gtime) 
{
    epicsTimeStamp ets;
    struct timespec sp;
    time_t utime;

    /* note: this doesn't necessarily get the time from an NTP server.... */
    if (epicsTimeGetCurrent(&ets) != epicsTimeOK)
        return -1;            /* If error, return year as -1 */
                    /* NTP epoch is 1900 */
    epicsTimeToTimespec(&sp, &ets); /* Convert EPICS timestamp to POSIX struct timespec */

    utime = sp.tv_sec;

    *gtime = gmtime(&utime);        /* Convert to broken down time */
                    /* in struct tm */
    return 0;
}

/**********************************************************************************************************
* 
* bcSetEpoch - set epoch year
*
* calculate the start of year epoch (wrt 1970) using mktime, given year. 
* Set the value in the global var bcYearEpoch
*
* RETURNS: OK
*
* NOMANUAL
*/
int bcSetEpoch(const int year)
{
    struct tm gtime;

    gtime.tm_sec = 0;
    gtime.tm_min = 0;
    gtime.tm_hour = 0;
    gtime.tm_mday = 1;
    gtime.tm_mon = 0;
    gtime.tm_year = year - 1900;    /* Date for this year, Jan 1st, 00:00:00 UTC */
    printf("bcSetEpoch year is %d\n", year);
    bcYearEpoch = mktime(&gtime);    /* Calculate current start of year epoch */
    return 0;
}


/**********************************************************************************************************
* 
* bcYearMonitor - Year update monitor task
*
* This is run as a task, regularly setting the year number fetched via NTP 
* and also setting the Bancomm real time clock to the current time.
* The task runs every hour on the half-hour to avoid problems with it being
* called as the year changes, or during leap seconds, both of which could
* result in the date and epoch going haywire.
* If an IOC is started up at midnight on Dec 31st, it is conceivable that 
* it could have the wrong year, but only for the first half hour.
*
* RETURNS: N/A
*
* NOMANUAL
*/
int bcYearMonitor(void)
{
    int status,year;
    int first_try_count = 0;
    struct tm *gtime;
    
    
    do {
        first_try_count++;
        if (first_try_count >25 ) return ERROR;        /* Give up eventually */
        status = NTPgetTime(&gtime);
        if (status == 0) {
            year = 1900 + gtime->tm_year;
            if (year > 1993 && year < 2050) {    /* If value OK, only during epoch 1994-2049 */
                bcYearNumber = year;        /* Set year number */
                bcSetEpoch(year);        /* Calculate and set epoch */
            }
        }   
        else {
            year = 0;
            epicsThreadSleep( 5); /* Problem? - wait a few seconds */
        }   
    } while ((status != 0) || year < 1994 || year > 2049);

    /* Wait for xx:30 */
    /*
      epicsThreadSleep(clock_rate_get() * 60 * (30 - gtime->tm_min));
     */

    while (1) {                /* Now loop forever */
        status = NTPgetTime(&gtime);
        if (status == 0) {
            year = 1900 + gtime->tm_year;    /* add 1900 to get year number */
            if (year > 1993 && year < 2050) {    /* If value OK, only during epoch 1994-2049 */
               if(bcYearNumber != year) {
                  bcYearNumber = year;        /* Set year number */
                  bcSetEpoch(year);           /* Calculate and set epoch */
               }
            }
        }
         epicsThreadSleep(YEAR_MONITOR_SLEEP);   /* Now wait specified number of seconds (1 hour) */
         /* epicsThreadSleep(60.0);  */
    }
}


/**********************************************************************************************************
* 
* bcStartYearMonitor - Start year monitor task
*
* Spawn the task to run the year monitor function (bcYearMonitor).
*
* RETURNS: N/A
*
* NOMANUAL
*/
static int bcStartYearMonitor(void)
{

    if (!bcYearMonitorStarted)
    {
        bcYearMonitorStarted = 1;

        /* Don't rush it - wait a few seconds */
        epicsThreadSleep(1);

        if( (epicsThreadCreate("bcYearMonitor", 
                                100, 
                                epicsThreadGetStackSize(epicsThreadStackMedium), 
                                (EPICSTHREADFUNC)bcYearMonitor, NULL) ) )
        {
            bcYearMonitorStarted = 0;
            return ERROR;
        }
    }
    return OK;
}

/**********************************************************************************************************
* 
* bcSetRTC - Set Battery-backed clock
*
* Load the Bancomm real-time clock (RTC) using the current NTP time
*
* RETURNS: N/A
*
*/
void bcSetRTC(void)
{
    struct tm *gtime;
    char rtctime[14];
    epicsTimeStamp ets;
    struct timespec sp;
    double fracsec;
    time_t utime;

    //epicsTimeGetCurrent(&ets);
    generalTimeGetExceptPriority(&ets, &bc635TimeTpPrio, 1); /* don't get current time from Bancomm board! */
    epicsTimeToTimespec(&sp, &ets); /* Convert EPICS timestamp to POSIX struct timespec */
    fracsec = 1.0 - (sp.tv_nsec/1000000000.0);
                    /* NTP epoch is 1900 */
    utime = sp.tv_sec - TS_1900_TO_UNIX_EPOCH;
//printf("ntp secs: %d; unix secs: %d\n", sp.tv_sec,utime);
    gtime = gmtime(&utime);        /* Convert to broken down time */
                    /* in struct tm */
    sprintf(rtctime,"L%02d%02d%02d%02d%02d%02d",
    (gtime->tm_year)%100,gtime->tm_mon+1, gtime->tm_mday,
    gtime->tm_hour, gtime->tm_min, gtime->tm_sec + 1); 
    printf("TFP packet for RTC = %s\n",rtctime);
                    /* wait for approx. next second tick */
    /* (void) taskDelay((int)(fracsec * clock_rate_get()) - 1); */ 
    //epicsThreadSleep((int)(fracsec * epicsThreadSleepQuantum()) - 1);
    epicsThreadSleep(fracsec);
    bcSendTfp(rtctime);
}



/**********************************************************************************************************
* 
* BCconfigure - Configure the Bancomm time interface
*
* Define startup configuration options for the Bancomm time card
* To be called direct from the VxWorks startup file before iocInit.
*
* RETURNS: N/A
*
*/
void BCconfigure
(
    const int MasterIOC,        /* TRUE for Master IOC with bc637 GPS receiver */
    const int NoLeapSecs,       /* FALSE = use UTC; TRUE = GPS time, no leap secs */
    const int intPerSecond,     /* Bancomm Periodic Frequency in Hz */
    const int intPerTick,       /* Number of periodic interrupts per system clock tick */
    const int Offset            /* Offset in microseconds relative to input reference, +ve = correction for delay */
)
{
    int status;

    bcThisIsMaster = MasterIOC;            /* Master IOC? */
    bcNoLeapSecs = NoLeapSecs;        /* Don't use leap secs? */
    bcOffset = Offset;                /* Offset in microsecs */
    if ((status = bcHardwareSetup()) != OK)        /* Initialise the hardware, abandon on failure */
    {
        printf("BCconfigure failed - Bancomm card not found\n");
        return;
    }

    /* Start up the BC635 Time Provider */
    if(bc635TimeTpPrio >= 0)
       bc635Time_Init(bc635TimeTpPrio);
    bc635Time_Report(1);

    bc635IntEnable(2,"");                /* Enable Bancomm periodic interrupts */
    bcClkRateSet(intPerSecond, intPerTick);        /* Set up the interrupt and tick rate */

}

/**********************************************************************************************************
* 
* bcIntConnect - Connect user interrupt routine
*
* Register a user defined interupt service routine to be called on
* every Bancomm periodic interrupt.
*
* RETURNS: N/A
*
* SEE ALSO: bcIntDisconnect()
*/ 
/* isrproc = ISR routine name */
void bcIntConnect (void (*isrproc)(const int n))
{
    bcUsrClock = isrproc;
}

/*******************************************************************************
* 
* bcIntDisconnect - Disconnect user interrupt routine
*
* Disconnects a previously defined user interrupt routine for handling
* Bancomm periodic interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: bcIntConnect()
*/  
void bcIntDisconnect (void)
{
    bcUsrClock = NULL;
}

/*******************************************************************************
* ErForceSync - routine called by time stamp driver to sync time
*/

long ErForceSync (int card)
{
    if (card != 0)
        return ERROR;
    return OK;
}


/*******************************************************************************
* ErSyncEvent - routine called by time stamp driver to synchronize
*/

long ErSyncEvent(void)
{
    return OK;
}


/*******************************************************************************
* ErDriverInit - routine called by time stamp driver to initialize driver
*/

long ErDriverInit(void)
{
    bc635_init ();
    return OK;
}


/*******************************************************************************
* ErGetTime - routine called by time stamp driver to get the real time
*/

long ErGetTime (struct timespec *sp)
{
    bc635TSread (sp);
    return OK;
}


/*******************************************************************************
* ErDirectTime - routine called by time stamp driver to determine time master
*/

long ErDirectTime (void)
{
    return bcConfiguredOK;
}

void setBcConfiguredOK()
{
      bcConfiguredOK = 1;
}

void bcSendOcode(char *charptr )
{
    int j, count;
    unsigned char temp;

    bcSendTfp(charptr);

    for (j=0; j <200; j++)
    {
        epicsThreadSleep(0.001);
        printf("pbc635->ack & 0x04 = %d\n", (pbc635->ack & 0x04) );
        if (pbc635->ack & 0x04) break;    /* Wait for output FIFO data */
    }

    count = 0;

    while ((temp = pbc635->fifo) != ETB && count < 200)
    {
        epicsThreadSleep(0.001);
        printf("FIFO = 0x%x\n", temp);
        count++;
    }

    if( count == 200 )
        printf("Error: Timed out waiting for response to %s\n", charptr);
}

/* Register these symbols for use by IOC code */
/* Information needed by iocsh */
static const iocshArg     bc635_reportArg0 = {"interest_level", iocshArgInt};
static const iocshArg    *bc635_reportArgs[] = { &bc635_reportArg0 };
static const iocshFuncDef bc635_reportFuncDef = {"bc635_report", 1, bc635_reportArgs};

/* Wrapper called by iocsh, selects the argument types that bc635_report needs */
static void bc635_reportCallFunc(const iocshArgBuf *args) {
    bc635_report(args[0].ival);
}

/* Registration routine, runs at startup */
static void bc635_reportRegister(void) {
    iocshRegister(&bc635_reportFuncDef, bc635_reportCallFunc);
}

/* Register these symbols for use by IOC code */
/* Information needed by iocsh */
static const iocshArg     BCconfigureArg0 = {"master", iocshArgInt};  /* TRUE for Master IOC with bc637 GPS receiver */
static const iocshArg     BCconfigureArg1 = {"useleap", iocshArgInt};  /* FALSE = use UTC; TRUE = GPS time, no leap secs */
static const iocshArg     BCconfigureArg2 = {"intPerSecond", iocshArgInt};  /* Bancomm Periodic Frequency in Hz */
static const iocshArg     BCconfigureArg3 = {"intPerTick", iocshArgInt};  /* Number of periodic interrupts per VxWorks system clock tick */
static const iocshArg     BCconfigureArg4 = {"Offset", iocshArgInt};  /* Offset in microseconds relative to input reference, +ve = correction for delay */

static const iocshArg    *BCconfigureArgs[] = {
	&BCconfigureArg0,
	&BCconfigureArg1,
	&BCconfigureArg2,
	&BCconfigureArg3,
	&BCconfigureArg4,
};

static const iocshFuncDef BCconfigureFuncDef = {"BCconfigure", 6, BCconfigureArgs};

/* Wrapper called by iocsh, selects the argument types that bc635_report needs */
static void BCconfigureCallFunc(const iocshArgBuf *args) {
    BCconfigure(args[0].ival, args[1].ival, args[2].ival, args[3].ival, args[4].ival );
}

/* Registration routine, runs at startup */
static void BCconfigureRegister(void) {
    iocshRegister(&BCconfigureFuncDef, BCconfigureCallFunc);
}

epicsExportRegistrar(bc635_reportRegister);
epicsExportRegistrar(BCconfigureRegister);
epicsExportAddress(int, altIntCounter);
epicsExportAddress(int, bcIntCounter);
epicsExportAddress(int, bcTickCnt);



/*********************************************************************/
/*   EPICS Time Provider section: maybe should be in a separate file */

#define NSEC_PER_SEC 1000000000

static epicsThreadOnceId onceId = EPICS_THREAD_ONCE_INIT;


static struct {
   epicsMutexId     lock;
   epicsUInt32      priority;
   epicsUInt32      flywheeling;
   epicsTimeStamp   syncTime;
} bc635TimePvt;



/* Initialization */
static void bc635Time_InitOnce(void *priority)
{
   bc635TimePvt.lock        = epicsMutexCreate();
   bc635TimePvt.priority    = *(int*)priority;
   bc635TimePvt.flywheeling = pbc635->time[1] & 0x10;
   bc635TimePvt.syncTime.secPastEpoch = 1;
   bc635TimePvt.syncTime.nsec = 0;

   /* Finally register as a time provider */
   generalTimeRegisterCurrentProvider("bc635", 
                                       bc635TimePvt.priority,
                                       bc635TimeGetCurrent);
}  
  
void bc635Time_Init(int priority)
{
   epicsThreadOnce(&onceId, bc635Time_InitOnce, &priority);
}


/**********************************************************************************************************
* 
*  bc635TimeGetCurrent - Function to register with EPICS for reading the current time
*
* Read the current time registers and return the time 
* as an epicsTimeStamp
*
* RETURNS:
* OK or errors (no Bancomm or Bancomm status bit field)
*
*/ 
/* pDest = Current time as an EPICS time stamp */
static int bc635TimeGetCurrent(epicsTimeStamp *pDest)
{
    double cTime = 0.0;
    int status = 0;

    epicsMutexMustLock(bc635TimePvt.lock);


    status = bc635_read(&cTime);
    if (!(status & ~0x07))
    {
       pDest->secPastEpoch = (epicsUInt32)cTime - (epicsUInt32)(POSIX_TIME_AT_EPICS_EPOCH); 
       pDest->nsec = (cTime - (epicsUInt32)cTime)*NSEC_PER_SEC;
       if(!(bc635TimePvt.flywheeling = status & 0x01))
          bc635TimePvt.syncTime = *pDest;
    }
#if 0
{
static int count;
if(count%500 == 0) {
   printf("cTime=%f, secPastEpoch=%d, nsec=%d\n", cTime, pDest->secPastEpoch, pDest->nsec);
}
count++;
}
#endif
    epicsMutexUnlock(bc635TimePvt.lock);
    if ((status & ~0x07) || bc635TimePvt.flywheeling) 
       return epicsTimeERROR;
    return epicsTimeOK;
}

/************************************************************************************************
 *
 * BCsetTpPrio -- Set the priority of the BC635 Time Provider
 * 
 * call this from the startup script BEFORE calling BCconfigure() if you want to set the BC635 
 * Time Provider priority to something other than the default.
 * Set the priority to a negative value to disable the BC635 Time Provider.
 *
 * RETURNS: N/A
 */
int bc635TimeSetTpPrio(int prio) 
{
   bc635TimeTpPrio = prio;
   return 0;
}



/* Status Report */

int bc635Time_Report(int level)
{
    if (onceId == EPICS_THREAD_ONCE_INIT) {
        printf("BC635 Time Provider not initialized\n");
    } 
    else {
       printf("BC635 Time Provider registered\n");
       if (level) {
          char lastSync[32];
          epicsTimeToStrftime(lastSync, sizeof(lastSync),
                    "%Y-%m-%d %H:%M:%S.%06f", &bc635TimePvt.syncTime);
          printf("\tpriority = %u\n", bc635TimePvt.priority);
          printf("\t%s\n", bc635TimePvt.flywheeling?"Flywheeling (not locked to reference)":"Locked to reference");
          printf("\tLast successful sync was at %s\n", lastSync);
       }
    }
    return 0;
}

/* Set up to  export the report function to the IOC shell                         */
static const iocshArg ReportArg0 = { "interest_level", iocshArgInt};

static const iocshArg * const ReportArgs[1] = { &ReportArg0 };
static const iocshFuncDef bc635TimeReportFuncDef = {"bc635Time_Report", 1, ReportArgs};
static void bc635TimeReportCallFunc(const iocshArgBuf *args)
{
    bc635Time_Report(args[0].ival);
}


/* Set up to  export the time provider priority set function to the IOC shell                         */
static const iocshArg bc635TimeSetTpPrioArg0 = {"BC635 Time Provider Priority", iocshArgInt}; 
static const iocshArg * const bc635TimeSetTpPrioArgs[1] = { &bc635TimeSetTpPrioArg0 };
static const iocshFuncDef bc635TimeSetTpPrioFuncDef = {"bc635TimeSetTpPrio", 1, bc635TimeSetTpPrioArgs};
static void bc635TimeSetTpPrioCallFunc(const iocshArgBuf *args)
{
    bc635TimeSetTpPrio(args[0].ival);
}

/* now register and export the shell functions */
static void bc635TimeRegister(void) {
   iocshRegister(&bc635TimeReportFuncDef, bc635TimeReportCallFunc);
   iocshRegister(&bc635TimeSetTpPrioFuncDef, bc635TimeSetTpPrioCallFunc);
}
epicsExportRegistrar(bc635TimeRegister);

