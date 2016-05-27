[schematic2]
uniq 12
[tools]
[detail]
w 2044 443 100 0 n#1 hwin.hwin#3.in 1992 440 2096 440 eais.getEventTime.INP
w 1028 307 100 0 n#2 eaos.setTodIntTime.OUT 976 304 1080 304 hwout.hwout#9.outp
w 416 1403 100 0 n#3 hwin.hwin#16.in 352 1400 480 1400 estringinval.timestamp.INP
w 372 955 100 0 n#4 hwin.hwin#23.in 320 952 424 952 eais.todOnInterrupt.INP
w 1172 1347 100 0 n#5 hwin.hwin#26.in 1144 1344 1200 1344 eais.getCurrentTime.INP
w 1476 1331 100 0 n#6 eais.getCurrentTime.FLNK 1456 1328 1496 1328 1496 880 1728 880 ecalcs.ecalcs#28.SLNK
w 1520 1299 100 0 n#7 eais.getCurrentTime.VAL 1456 1296 1584 1296 1584 1264 1728 1264 ecalcs.ecalcs#28.INPA
w 2112 1115 100 0 n#8 ecalcs.ecalcs#28.VAL 2016 1072 2200 1072 2200 760 552 760 552 368 720 368 eaos.setTodIntTime.DOL
w 2140 1147 100 0 n#9 ecalcs.ecalcs#28.FLNK 2016 1104 2256 1104 2256 720 576 720 576 336 720 336 eaos.setTodIntTime.SLNK
w 728 939 100 0 n#10 eais.todOnInterrupt.FLNK 680 936 776 936 776 1080 352 1080 352 1368 480 1368 estringinval.timestamp.SLNK
w 792 1387 100 0 n#11 estringinval.timestamp.FLNK 736 1384 848 1384 848 1312 1200 1312 eais.getCurrentTime.SLNK
s 312 165 100 0 This specified time must be in the range 0.0 (midnight) to 86399.999 (23h59m59.999s) inclusive.
n 1166 1158 1453 1182 100
Time output here is local time
_
n 1877 1234 2463 1426 100
This assumes EPICS timezone is set to UTC
[See $(EPICS)/base/configure/CONFIG_SITE_ENV ]
Otherwise you will have subtract the number of seconds your
time zone is lagging behind UTC.
Add 5 seconds to current time,
Divide by the number of seconds in a day
and take remainder as number of seconds since
midnight to set the TOD interrupt registers.
_
n 1957 507 2342 627 100
External Event interrupts are triggered
by raisng a signal on the VME backplane
at Connector P2 pin C6.

This still needs to be tested.
_
[cell use]
use bb200tr 0 0 100 0 bb200tr#1
xform 0 1280 800
use eais 2175 315 100 0 getEventTime
xform 0 2224 408
p 2120 475 100 0 1 DTYP:Bancomm 635
p 2290 361 100 0 1 SCAN:I/O Intr
p 2091 315 100 0 -1 PV:$(top)
p 2169 340 100 0 1 PREC:6
use hwin 1800 424 100 0 hwin#3
xform 0 1896 440
p 1803 432 100 0 -1 val(in):#C0 S1
use eaos 787 234 100 0 setTodIntTime
xform 0 848 336
p 874 464 100 0 1 DTYP:Bancomm 635
p 872 430 100 0 1 SCAN:Passive
p 703 234 100 0 -1 PV:$(top)
p 789 254 100 0 1 PREC:12
p 716 206 100 0 1 OMSL:closed_loop
use hwout 1080 288 100 0 hwout#9
xform 0 1176 304
p 1176 295 100 0 -1 val(outp):#C0 S3
use estringinval 543 1298 100 0 timestamp
xform 0 608 1368
p 569 1444 100 0 1 DTYP:Bancomm 635
p 480 1430 100 0 1 SCAN:Passive
p 459 1298 100 0 -1 PV:$(top)
use hwin 160 1384 100 0 hwin#16
xform 0 256 1400
p 163 1392 100 0 -1 val(in):#C0 S0
use eais 500 832 100 0 todOnInterrupt
xform 0 552 920
p 540 1034 100 0 1 DTYP:Bancomm 635
p 538 998 100 0 1 SCAN:I/O Intr
p 416 832 100 0 -1 PV:$(top)
p 497 852 100 0 1 PREC:6
use hwin 128 936 100 0 hwin#23
xform 0 224 952
p 131 944 100 0 -1 val(in):#C0 S3
use eais 1265 1235 100 0 getCurrentTime
xform 0 1328 1312
p 1303 1214 100 0 1 DTYP:Bancomm 635
p 1301 1196 100 0 1 SCAN:Passive
p 1181 1235 100 0 -1 PV:$(top)
p 1267 1384 100 0 1 PINI:YES
use hwin 952 1328 100 0 hwin#26
xform 0 1048 1344
p 955 1336 100 0 -1 val(in):#C0 S0
use ecalcs 1728 816 100 0 ecalcs#28
xform 0 1872 1056
p 1846 835 100 0 1 CALC:(A+5) % 86400
[comments]
