/* devBc635.c */

/* devBc635.c - Bancomm 635 Device Support Routines
*
*	Original Author: Philip Taylor
*	Current Author:  Bret Goodrich
*	Date:            1996feb08
*
* modification history
* --------------------
* 01, 08Feb96,bdg  merged ai and ai devsup files.  added si and so support.
* 02, 26Jan98,ajf  read_ai must return val if alarm is only MINOR.
* 03, 26Mar99,ajf  read_ai was not returning a value if status equalled OK.
*
* 2016/03/07 mdw   Removed vxWorks #includes. Code is now EPICS OSI compliant.
* 2016/05/25 mdw   read_si now returns val if alarm is only MINOR.
*/


#include <epicsStdio.h>
#include <epicsTime.h>

#include <dbDefs.h>
#include <dbAccess.h>
#include <dbScan.h>
#include <recGbl.h>
#include <recSup.h>
#include <devSup.h>
#include <link.h>
#include <alarm.h>
#include <aiRecord.h>
#include <aoRecord.h>
#include <stringinRecord.h>
#include <stringoutRecord.h>
#include <epicsExport.h>

#include "bc635.h"

static long	init_record_ai (struct aiRecord *);
static long	init_record_ao (struct aoRecord *);
static long	init_record_si (struct stringinRecord *);
static long	init_record_so (struct stringoutRecord *);
static long	get_ioint_info_ai (int, struct aiRecord *, IOSCANPVT *);
static long	get_ioint_info_ao (int, struct aoRecord *, IOSCANPVT *);
static long	get_ioint_info_si (int, struct stringinRecord *, IOSCANPVT *);
static long	get_ioint_info_so (int, struct stringoutRecord *, IOSCANPVT *);
static long	read_ai (struct aiRecord *);
static long	write_ao (struct aoRecord *);
static long	read_si (struct stringinRecord *);
static long	write_so (struct stringoutRecord *);

long bc635_ioint_info( const unsigned short, IOSCANPVT * );
int  bc635_event( double * );


typedef struct
{
    long number;
    DEVSUPFUN report;
    DEVSUPFUN init;
    DEVSUPFUN init_record;
    DEVSUPFUN get_ioint_info;
    DEVSUPFUN read_write;
    DEVSUPFUN special_linconv;
} DEV_BC635;

DEV_BC635 devAiBc635 =
    {6, NULL, NULL, init_record_ai, get_ioint_info_ai, read_ai, NULL};
epicsExportAddress(dset, devAiBc635); 

DEV_BC635 devAoBc635 =
    {6, NULL, NULL, init_record_ao, get_ioint_info_ao, write_ao, NULL};
epicsExportAddress(dset, devAoBc635); 

DEV_BC635 devSiBc635 =
    {6, NULL, NULL, init_record_si, get_ioint_info_si, read_si, NULL};
epicsExportAddress(dset, devSiBc635); 

DEV_BC635 devSoBc635 =
    {6, NULL, NULL, init_record_so, get_ioint_info_so, write_so, NULL};
epicsExportAddress(dset, devSoBc635); 


/*******************************************************************************
* init_record_ai
*/

static long init_record_ai (
    struct aiRecord *pai)
{
    struct vmeio *pvmeio;
					/* input link must be VME_IO */
    if (pai->inp.type != VME_IO)
    {
	recGblRecordError (S_db_badField, (void *) pai,
	   "devAiBc635 (init_record) Illegal INP field");
	return S_db_badField;
    }

    pvmeio = (struct vmeio *) &pai->inp.value;

#if FALSE
    if (pai->scan == SCAN_IO_EVENT &&
	bc635IntEnable (pvmeio->signal, pvmeio->parm) != OK)

    {
	recGblRecordError(S_dev_vxWorksIntEnFail, (void *) pai,
	    "devAiBc635 (init_record) BC635 interrupt enable error");
    }
#endif /* FALSE */
					/* configure Bancomm hardware */
    if (pai->scan == SCAN_IO_EVENT)
	(void) bc635IntEnable (pvmeio->signal, pvmeio->parm);

    pai->udf = FALSE;			/* record is now defined */
    pai->linr = 0;			/* no linearization */

    return OK;
}


/*******************************************************************************
* init_record_ao
*/

static long init_record_ao (
    struct aoRecord *pao)
{
    struct vmeio *pvmeio;
					/* output link must be VME_IO */
    if (pao->out.type != VME_IO)
    {
	recGblRecordError (S_db_badField, (void *) pao,
	    "devAoBc635 (init_record) Illegal OUT field");
	return S_db_badField;
    }
					/* only passive scanning allowed */
    pvmeio = (struct vmeio *) &pao->out.value;
    if (pao->scan != SCAN_PASSIVE)
    {
	recGblRecordError (S_db_badField, (void *) pao,
	    "devAoBc635 (init_record) Illegal SCAN type");
    }

    return 2;				/* do not convert raw value */
}


/*******************************************************************************
* init_record_si
*/

static long init_record_si (
    struct stringinRecord *psi)
{
    struct vmeio *pvmeio;
					/* input link must be VME_IO */
    if (psi->inp.type != VME_IO)
    {
	recGblRecordError (S_db_badField, (void *) psi,
	    "devSiBc635 (init_record) Illegal INP field");
	return S_db_badField;
    }

    pvmeio = (struct vmeio *) &psi->inp.value;

#if FALSE
    if (psi->scan == SCAN_IO_EVENT &&
	bc635IntEnable (pvmeio->signal, pvmeio->parm) != OK)
    {
	recGblRecordError (S_dev_vxWorksIntEnFail, (void *) psi,
	    "devSiBc635 (init_record) BC635 interrupt enable error");
    }
#endif /* FALSE */
					/* configure Bancomm hardware */
    if (psi->scan == SCAN_IO_EVENT)
	(void) bc635IntEnable (pvmeio->signal, pvmeio->parm);

    psi->udf = FALSE;
    return OK;
}


/*******************************************************************************
* long init_record_so
*/

static long init_record_so (
    struct stringoutRecord *pso)
{
    struct vmeio *pvmeio;
					/* output link must be VME_IO */
    if (pso->out.type != VME_IO)
    {
	recGblRecordError (S_db_badField, (void *) pso,
	    "devSoBc635 (init_record) Illegal OUT field");
	return S_db_badField;
    }
					/* only passive scanning allowed */
    pvmeio = (struct vmeio *) &pso->out.value;
    if (pso->scan != SCAN_PASSIVE)
    {
	recGblRecordError (S_db_badField, (void *) pso,
	    "devSoBc635 (init_record) Illegal SCAN type");
    }

    return OK;
}


/*******************************************************************************
* get_ioint_info_ai
*/
static long get_ioint_info_ai (
    int cmd,
    struct aiRecord *pai,
    IOSCANPVT *ppvt)
{
    struct vmeio *pvmeio;

    pvmeio = (struct vmeio *) &pai->inp.value;
    bc635_ioint_info (pvmeio->signal, ppvt);

    return OK;
}


/*******************************************************************************
* get_ioint_info_ao
*/
static long get_ioint_info_ao (
    int cmd,
    struct aoRecord *pao,
    IOSCANPVT *ppvt)
{
    struct vmeio *pvmeio;

    pvmeio = (struct vmeio *) &pao->out.value;
    bc635_ioint_info (pvmeio->signal, ppvt);

    return OK;
}


/*******************************************************************************
* get_ioint_info_si
*/
static long get_ioint_info_si (
    int cmd,
    struct stringinRecord *psi,
    IOSCANPVT *ppvt)
{
    struct vmeio *pvmeio;

    pvmeio = (struct vmeio *) &psi->inp.value;
    bc635_ioint_info (pvmeio->signal, ppvt);

    return OK;
}


/*******************************************************************************
* get_ioint_info_so
*/
static long get_ioint_info_so (
    int cmd,
    struct stringoutRecord *pso,
    IOSCANPVT *ppvt)
{
    struct vmeio *pvmeio;

    pvmeio = (struct vmeio *) &pso->out.value;
    bc635_ioint_info (pvmeio->signal, ppvt);

    return OK;
}


/*******************************************************************************
* read_ai
*/

static long read_ai (
    struct aiRecord *pai)
{
    double value;
    struct vmeio *pvmeio;
    long status;

    pvmeio = (struct vmeio *) &pai->inp.value;
    if (pvmeio->signal == 1)
        status = bc635_event (&value);
    else
        status = bc635_read (&value);

    /*
     * Check for failures. 
     * status = -1: Nonsense time - invalid alarm.
     * status = 1:  Lost frequency source - major alarm.
     * status > 1:  Not synced, frequency offset, time offset - minor alarm.
     */

    pai->val = value;
    if(status == ERROR)
    {
      recGblSetSevr (pai, READ_ALARM, INVALID_ALARM);
      pai->val = 0;
    }
    else if (status & 0x01)
      recGblSetSevr (pai, READ_ALARM, MAJOR_ALARM);
    else if (status != OK)
      recGblSetSevr (pai, READ_ALARM, MINOR_ALARM);

    pai->udf = FALSE;
    return 2;				/* don't convert from raw value */
}


/*******************************************************************************
* write_ao
*/

static long write_ao (
    struct aoRecord *pao)
{
    struct vmeio *pvmeio;
 
    pvmeio = (struct vmeio *)&(pao->out.value);
    if (bc635_write (pvmeio->signal, pao->val) != OK)
	recGblSetSevr (pao, WRITE_ALARM, INVALID_ALARM);

    return OK;
}


/*******************************************************************************
* read_si
*/

static long read_si (
    struct stringinRecord *psi)
{
    time_t ival;
    double value;
    struct tm *ptm;
    struct vmeio *pvmeio;
    long status;
 
    pvmeio = (struct vmeio *) &psi->inp.value;
    if (pvmeio->signal == 1)
        status = bc635_event (&value);
    else
        status = bc635_read (&value);

    /*  Check for failures */
    if (status == ERROR) /* Bancomm registers have invalid time values */
	recGblSetSevr (psi, READ_ALARM, INVALID_ALARM);
    else if (status & 0x01) /* Bancomm not synched to time reference */
	recGblSetSevr (psi, READ_ALARM, MAJOR_ALARM);
    else
    {
        if (status & 0x06 )  /* Bancomm time or frequency offset too large */
	   recGblSetSevr (psi, READ_ALARM, MINOR_ALARM);

	ival = (time_t) value;
	ptm = gmtime (&ival);
	ptm->tm_year += (ptm->tm_year >= 70) ? 1900 : 2000;
	sprintf (psi->val, "%04d/%02d/%02d %02d:%02d:%02d.%06d",
	    ptm->tm_year, ptm->tm_mon+1, ptm->tm_mday,
	    ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
	    ((int) ((value - ival) * 1000000.0)));
    }
 
    psi->udf = FALSE;
    return OK;
}


/*******************************************************************************
* write_so
*/

static long write_so (
    struct stringoutRecord *pso)
{
    time_t ival;
    int n, h, m, s;
    struct vmeio *pvmeio;

    pvmeio = (struct vmeio *) &pso->out.value;
    n = sscanf (pso->val, "%d:%d:%d", &h, &m, &s);
    ival = h * 3600 + m * 60 + s;
    if (n != 3 || h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 50)
	recGblSetSevr (pso, WRITE_ALARM, INVALID_ALARM);

    if (bc635_write (pvmeio->signal, (double) ival) != OK)
    {
	recGblSetSevr (pso, WRITE_ALARM, INVALID_ALARM);
    }

    return OK;
}
