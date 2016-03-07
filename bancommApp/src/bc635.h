/*
 * Copyright 1996 Association of Universities for Research In Astronomy, Inc.
 * See the file COPYRIGHT for more details.
 *
 *   FILENAME:
 *   bc635.h
 *
 *   PURPOSE:
 *   Bancomm BC635/637 device driver C interface
 *
 *INDENT-OFF*
 * Rev 2.0 2016/03/07 mdw
 * Added OK/ERROR #defines, readying for import into CCB ADE
 *
 *
 * $Log: bc635.h,v $
 * Revision 1.1.1.1  2011/01/20 13:10:45  ajf
 * Initial import of R3.14.12
 *
 * Revision 1.1.1.1  2004/07/02 14:55:30  ajf
 * Gemini specific code
 *
 * Revision 1.1  2003/12/04 15:41:03  ajf
 * Import of the Gemini specific directory into GEM8.5.
 *
 * Revision 1.1.1.1  2003/07/04 09:41:14  ajf
 * Initial creation of the OSL EPICS 3.13.8 Repository
 *
 * Revision 1.1  2001/07/10 10:29:44  ajf
 * ajf: Added for the GEM7 release
 *
 * Revision 1.1.1.1  1997/03/11 23:13:52  goodrich
 * Gem4
 *
 * Revision 1.3  1996/12/31 21:12:35  goodrich
 * Gem4
 *
 * Revision 1.2  1996/12/31 00:26:05  goodrich
 * ANSI-fied
 *
 * 01a,20mar95,pbt	Philip Taylor
 * 01b,30dec96,bdg	ANSI-fied
 *
 *INDENT-ON*
 */
 
#ifndef INCbc635h
#define INCbc635h


#define OK 0
#define ERROR (-1)

 
extern void	BCconfigure(const int, const int, const int, const int,
		    const int);
extern int	bc635IntEnable(const unsigned short, const char *);
extern int	bc635_read(double *);
extern int	bc635_write(const unsigned short, const double);
extern void	bcIntConnect(void (*isrproc)(const int n));
extern void	bcIntDisconnect(void);
extern int	bcSendTfp(char *);
extern void	bcSetRTC(void);
extern int	bcSetEpoch (const int);
extern int	bcRegsToTime (double *, unsigned char *);
extern int	bc635_report (int);
extern int	bc635_init (void);
extern int      bcTestCard( void );
extern int      bcGetGpsLeap( void );

#endif /* !INCbc635h */

