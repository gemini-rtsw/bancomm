[1] `Bancomm635VME User's Guide <http://www.microsemi.com/document-portal/doc_view/133468-bc635vme-user-s-guide>`__

2.5 Bancomm 
------------

The bancomm support library provides both driver and device support for
EPICS. The driver layer gives read and write access to the 635/637 board
registers whereas the device layer implements epics record specific
implementation. Since this driver was porting for OSI compatibility, a
thorough set of functional testing will be carried out.

2.5.1 Database Definition File (bancomm.dbd)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Bancomm database definition file defines the EPICS device and driver
requirements.

    variable( altIntCounter, int )

    variable( bcIntCounter, int )

    device(ai, VME_IO, devAiBc635, "Bancomm 635")

    device(ao, VME_IO, devAoBc635, "Bancomm 635")

    device(stringin, VME_IO, devSiBc635, "Bancomm 635")

    device(stringout, VME_IO, devSoBc635, "Bancomm 635")

    driver(drvBc635)

2.5.2 Device Support Requirements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following device support routines are required for the Bancomm
635/637 board.

1. devAiBc635 -- Provide Analog Input support for VME_IO including Scan
       I/O Interrupt support. Calls
       `bcIntEnable() <#kix.rrnoni7xhdye>`__.

2. devAoBc635 -- Provide Analog Output support for VME_IO Passive
       Scanning only.

3. devSiBc635 -- Provide String Input support for VME_IO including Scan
       I/O Interrupt support. Calls
       `bcIntEnable() <#kix.rrnoni7xhdye>`__.

4. devSoBc635 -- Provide String Output support for VME_IO Passive
       scanning only.

2.5.3 Driver Support Requirements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The bancomm 635/637 driver provides low-level access to the boards time
registers to acquire the current time as provided by either GPS or NTP.
The driver shall provide the following routines for access to external
support libraries such as timelib.:

    extern void BCconfigure(const int, const int, const int, const int,
    const int);

    extern int `bc635IntEnable <#kix.rrnoni7xhdye>`__\ (const unsigned
    short, const char \*);

    extern int bc635_read(double \*);

    extern int bc635_write(const unsigned short, const double);

    extern void bcIntConnect(void (*isrproc)(const int n));

    extern void bcIntDisconnect(void);

    extern int bcSendTfp(char \*);

    extern void bcSetRTC(void);

    extern int bcSetEpoch (const int);

    extern int bcRegsToTime (double \*, unsigned char \*);

    extern int bc635_report (int);

    extern int bc635_init (void);

    extern int bcTestCard( void );

    extern int bcGetGpsLeap( void );

+-----------------+-----------------+-----------------+-----------------+
| **Function**    | **Used By**     | **Meaning**     | **Return        |
|                 |                 |                 | value**         |
+=================+=================+=================+=================+
| BCconfigure     | Timelib:        | Configure the   | N/A             |
|                 |                 | Bancomm time    |                 |
|                 | timeClockInit() | interface.      |                 |
|                 |                 | Define startup  |                 |
|                 |                 | configuration   |                 |
|                 |                 | options for the |                 |
|                 |                 | Bancomm time    |                 |
|                 |                 | card to be      |                 |
|                 |                 | called direct   |                 |
|                 |                 | from the RTEMS  |                 |
|                 |                 | shell or        |                 |
|                 |                 | startup file    |                 |
|                 |                 | before iocInit. |                 |
|                 |                 | (In practice,   |                 |
|                 |                 | this is called  |                 |
|                 |                 | by              |                 |
|                 |                 | timeClockInit() |                 |
|                 |                 | ,               |                 |
|                 |                 | which is called |                 |
|                 |                 | in the startup  |                 |
|                 |                 | file before     |                 |
|                 |                 | iocInit).       |                 |
+-----------------+-----------------+-----------------+-----------------+
| bc635IntEnable  | Drv635:         | Enable          | OK,ERROR        |
|                 | BCconfigure()   | interrupts on   |                 |
|                 |                 | the Bancomm 635 |                 |
|                 | Dev635:         | card.           |                 |
|                 |                 |                 |                 |
|                 | initRecordAi()  |                 |                 |
|                 |                 |                 |                 |
|                 | initRecordSi()  |                 |                 |
+-----------------+-----------------+-----------------+-----------------+
| bc635_read      | Timelib         | Read time --    | OK,ERROR        |
|                 |                 | Read the        |                 |
|                 | timeInit()      | current time    |                 |
|                 |                 | registers and   |                 |
|                 |                 | return the time |                 |
|                 |                 | as the number   |                 |
|                 |                 | of seconds      |                 |
|                 |                 | (type double    |                 |
|                 |                 | value).         |                 |
+-----------------+-----------------+-----------------+-----------------+
| bc635_write     | Dev635:         | - Set time      | OK,ERROR if not |
|                 |                 | coincidence     | signal 3        |
|                 | write_ao(),     | strobe -- For   |                 |
|                 | write_so()      | signal number 3 |                 |
|                 |                 | (set TOD        |                 |
|                 |                 | interrupt)      |                 |
|                 |                 | write out to    |                 |
|                 |                 | the Bancomm as  |                 |
|                 |                 | follows :       |                 |
|                 |                 |                 |                 |
|                 |                 | Set Time Of Day |                 |
|                 |                 | for time        |                 |
|                 |                 | coincidence     |                 |
|                 |                 | interrupt.      |                 |
|                 |                 | Input value is  |                 |
|                 |                 | time for the    |                 |
|                 |                 | interrupt       |                 |
|                 |                 | (seconds since  |                 |
|                 |                 | midnight).      |                 |
+-----------------+-----------------+-----------------+-----------------+
| bcIntConnect    | Not used        | Register a user | N/A             |
|                 |                 | defined         |                 |
|                 |                 | interrupt       |                 |
|                 |                 | service routine |                 |
|                 |                 | to be called on |                 |
|                 |                 | every Bancomm   |                 |
|                 |                 | periodic        |                 |
|                 |                 | interrupt.      |                 |
+-----------------+-----------------+-----------------+-----------------+
| bcIntDisconnect | Not used        | Disconnects a   | N/A             |
|                 |                 | previously      |                 |
|                 |                 | defined user    |                 |
|                 |                 | interrupt       |                 |
|                 |                 | routine for     |                 |
|                 |                 | handling        |                 |
|                 |                 | Bancomm         |                 |
|                 |                 | periodic        |                 |
|                 |                 | interrupts.     |                 |
+-----------------+-----------------+-----------------+-----------------+
| bcSendTfp       | Drv635:         | Configure       | OK, or ERROR on |
|                 |                 | 635/637 for     | timeout.        |
|                 | EPICS Device    | specific modes  |                 |
|                 | Support         | (i.e., Master,  |                 |
|                 | “init()” call.  | Slave,          |                 |
|                 |                 | LeapSeconds     |                 |
|                 |                 | (on|off), or    |                 |
|                 |                 | bcoffset        |                 |
|                 |                 | control)        |                 |
|                 |                 |                 |                 |
|                 |                 | Send a string   |                 |
|                 |                 | to the bc635    |                 |
|                 |                 | output FIFO by  |                 |
|                 |                 | writing         |                 |
|                 |                 | successive      |                 |
|                 |                 | bytes to the    |                 |
|                 |                 | fifo address.   |                 |
|                 |                 | This routine    |                 |
|                 |                 | delimits the    |                 |
|                 |                 | supplied string |                 |
|                 |                 | with            |                 |
|                 |                 | <SOH>...<ETB>.  |                 |
|                 |                 | The Bancomm ACK |                 |
|                 |                 | register is set |                 |
|                 |                 | to 0x95 to      |                 |
|                 |                 | cause TFP to    |                 |
|                 |                 | take action,    |                 |
|                 |                 | then wait for   |                 |
|                 |                 | TFP acknowledge |                 |
|                 |                 | bit set (0x01). |                 |
+-----------------+-----------------+-----------------+-----------------+
| bcSetRTC        | Not used        | Set             | N/A             |
|                 |                 | Battery-backed  |                 |
|                 |                 | clock --Load    |                 |
|                 |                 | the Bancomm     |                 |
|                 |                 | real-time clock |                 |
|                 |                 | (RTC) using the |                 |
|                 |                 | current NTP     |                 |
|                 |                 | time            |                 |
+-----------------+-----------------+-----------------+-----------------+
| bcSetEpoch      | Drv635:         | Set epoch year  | OK              |
|                 |                 | -- calculate    |                 |
|                 | Only used       | the start of    | Needs           |
|                 | internally      | year epoch (wrt | Protection from |
|                 |                 | 1970) using     | ERROR           |
|                 |                 | mktime, given   |                 |
|                 |                 | year. Set the   |                 |
|                 |                 | value in the    |                 |
|                 |                 | global var      |                 |
|                 |                 | bcYearEpoch.    |                 |
+-----------------+-----------------+-----------------+-----------------+
| bcRegsToTime    | Drv635:         | - Convert bc    |                 |
|                 |                 | Time or Event   |                 |
|                 | Only used       | regs to time -- |                 |
|                 | internally      | Takes a mem     |                 |
|                 |                 | copy of the     |                 |
|                 |                 | EVENT or TIME   |                 |
|                 |                 | regs and        |                 |
|                 |                 | returns the     |                 |
|                 |                 | time as the     |                 |
|                 |                 | number of       |                 |
|                 |                 | seconds (type   |                 |
|                 |                 | double value).  |                 |
|                 |                 |                 |                 |
|                 |                 | \* RETURNS:     |                 |
|                 |                 |                 |                 |
|                 |                 | OK or errors    |                 |
|                 |                 | (no Bancomm or  |                 |
|                 |                 | Bancomm status  |                 |
|                 |                 | bit field)      |                 |
+-----------------+-----------------+-----------------+-----------------+
| bc635_report    | Drv635:         | - Report        | OK              |
|                 |                 | Bancomm 635/637 |                 |
|                 | EPICS Device    | status. This is |                 |
|                 | Support         | called from the |                 |
|                 | “report()”      | use of dbior    |                 |
|                 | call.           | drvBc635        |                 |
|                 |                 | (interest_level |                 |
|                 |                 | )               |                 |
|                 |                 | .               |                 |
+-----------------+-----------------+-----------------+-----------------+
| bc635_init      | EPICS iocInit   | - EPICS DRIVER  | OK, ERROR       |
|                 |                 | INIT            |                 |
|                 |                 | --Initialize    |                 |
|                 |                 | bc635/637 --    |                 |
|                 |                 | called from     |                 |
|                 |                 | iocInit         |                 |
+-----------------+-----------------+-----------------+-----------------+
| bcTestCard      | Drv635          | EPICS iocInit - | OK if card      |
|                 |                 | EPICS DRIVER    | present, ERROR  |
|                 |                 | INIT            | if absent       |
|                 |                 | --Initialize    |                 |
|                 |                 | bc635/637 --    |                 |
|                 |                 | called from     |                 |
|                 |                 | iocInit         |                 |
+-----------------+-----------------+-----------------+-----------------+
| bcGetGpsLeap    | Drv635:         | - Return the    | Integer number  |
|                 | bc635_init()    | current value   | of seconds.     |
|                 |                 | of GPS leap     |                 |
|                 |                 | seconds.        |                 |
|                 |                 |                 |                 |
|                 |                 | The current     |                 |
|                 |                 | value of GPS    |                 |
|                 |                 | leap seconds is |                 |
|                 |                 | obtained from a |                 |
|                 |                 | Bancomm 637     |                 |
|                 |                 | card by sending |                 |
|                 |                 | an O2 packet    |                 |
|                 |                 | request to the  |                 |
|                 |                 | TFP. The value  |                 |
|                 |                 | returned is     |                 |
|                 |                 | zero on a bc635 |                 |
|                 |                 | (non-GPS) card. |                 |
+-----------------+-----------------+-----------------+-----------------+

Further, the driver shall register itself with the epics generalTime
libraries adding the Bancomm driver to the list of available Time
Providers.

2.5.4 OSI Specific Challenges 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Bancomm board is required to stop and start the operating system
clock and provide it’s own tick pulse. See
`timeClockInit() <#fts0i87foetw>`__ for the description of changing the
system clock tick during startup scripts for different systems. Eg., the
MCS sets the tick resolution to 80 ticks per second, and the WFS’s and
SCS set the tick resolution to 200 ticks per second. The OSI layer of
epics base does not implement all VxWorks calls. We found that
controlling the system clock (as required by the Bancomm driver) needed
to use operating system dependent calls.

2.5.5 epicsGeneralTime
~~~~~~~~~~~~~~~~~~~~~~

The generalTime framework provides a mechanism for several time
providers to be present within the system. There

are two types of provider, one type for the current time and one type
for providing Time Event times. Each time

provider has a priority, and installed providers are queried in priority
order whenever a time is requested, until one

returns successfully. Thus there is a fallback from higher priority
providers (smaller value of priority) to lower priority

providers (larger value of priority) if the higher priority ones fail.
Each architecture has a “last resort” provider,

installed at priority 999, usually based on the system clock, which is
used in the absence of any other provider.

Targets running vxWorks and RTEMS have an NTP provider installed at
priority 100.

The Bancomm driver will register itself as the highest priority time
provider with priority 20. See specific requirements below.

2.5.6 Bancomm Board Jumper and Mode Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The jumper locations for the Rev. A through Rev. F TFP versions are
shown in Figure 2-2. The Rev. G and up along with the P100004 version
jumpers are shown in Figure 2-3. The jumper blocks are not drawn to
scale in order to make the numbers more visible. It may be helpful to
refer to the schematic diagrams to obtain a clearer idea of the function
of each jumper option.

-  JP1

..

    With the jumper in the 1-2 position the TFP is configured to use DC
    level shift input timecode. In the 3-4 or open position the TFP is
    configured to use modulated timecode.

-  JP2 (GPS Option)

..

    In the 1-2 position the TFP is configured to use a single ended 1pps
    GPS input. In the 3-4 position the TFP is configured to use a
    differential 1pps GPS input.

-  JP3 (GPS Option)

..

    In the 1-2 position the TFP is configured to use the ACUTIME Smart
    Antenna or SV-6 as the GPS sensor. In the 3-4 position the TFP is
    configured to use the TANS as the GPS sensor. The ACUTIME, SV-6, and
    TANS are GPS sensor options that are available from Datum, Inc. This
    jumper is not present on the P100004 model boards.

-  JP4

..

    The jumpers in the JP4 group are designed to be moved as a pair.
    Positions 3-4 and 5-6 define one configuration, and positions 1-2
    and 7-8 define a second configuration. In the default configuration
    the TFP is configured with an auxiliary RS-422 output. In the second
    configuration the TFP is configured ina daisy-chain mode (the RS-422
    input is jumpered to the RS-422 output). This jumper set is intended
    to be used in a digital synchronization mode. At the present time
    this mode has not been implemented. This jumper is not present on
    the P100004 model boards.

-  JP5

..

    In the 1-2 position this jumper places a “100Ω” load between the
    RS-422 input lines. In the 3-4 position the “100Ω” load is bypassed.
    When the TFP is the terminal device on an RS-422 daisy chain the
    load should be used. When the TFP is not at the end of the chain the
    load should be omitted.

-  JP6

..

    In the 1-2 position this jumper places GROUND on P2 pin C12. In the
    2-3 position the 1, 5, 10MHz clock is driven out of P2 pin C12. On
    the model P100004 boards, this jumper is implemented as a 2x2 pin
    block. A shunt on pins 2 and 4 enables the 10MHz output on P2 pin
    C12. A shunt on pins 1 and 2 disables the output by grounding P2 pin
    C12.

Operational Jumper Settings for GPS Locking on 637

+--+--+--+--+
|  |  |  |  |
+--+--+--+--+
|  |  |  |  |
+--+--+--+--+

Operational Jumper Settings for GPS Locking on 635

+--+--+--+--+
|  |  |  |  |
+--+--+--+--+
|  |  |  |  |
+--+--+--+--+

2.5.7 EEPROM Version Requirements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For successful use of the 637 boards Post May 3rd, 2015, the EEPROM must
be upgrade to version XXXX.

2.5.8 Specific Requirements
~~~~~~~~~~~~~~~~~~~~~~~~~~~

2.5.8.1 REQ-105-01 Initialize
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bancomm support library shall be capable of executing the
bc635_init() routine without error by calling iocInit().

2.5.8.2 REQ-105-02 EPICS Report
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The EPICS routine dbior drvBc635( <interest_level>) shall display the
state of all data channels respective for that card. See
`bc635_report <#8mo1uwbcs4cn>`__.

2.5.8.3 REQ-105-03 Analog Input Record Support (Scan I/O Interrupt)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bancomm support library shall be capable of initializing and
processing an EPICS Analog Input record with its DTYP set to drvBc635
and Scan I/O Interrupt support.

2.5.8.4 REQ-105-04 Analog Output Record Support
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bancomm support library shall be capable of initializing and
processing an EPICS Analog Output record with its DTYP set to drvBc635.

2.5.8.5 REQ-105-05 String Input Record Support (Scan I/O Interrupt)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bancomm support library shall be capable of initializing and
processing an EPICS String Input record with its DTYP set to drvBc635
and Scan I/O Interrupt support.

2.5.8.6 REQ-105-06 String Output Record Support
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bancomm support library shall be capable of initializing and
processing an EPICS String Output support with its DTYP set to drvBc635.

2.5.8.7 REQ-105-06 epicsGeneralTime Time Provider
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bancomm driver shall register itself with the epicsGeneralTime
framework as the highest time provider. This shall be evident at any
time by calling `generalTimeReport() <#9s27o4jsqqhh>`__ on the IOC.



