[schematic2]
uniq 8
[tools]
[detail]
w 388 1099 100 0 n#1 hwin.hwin#3.in 336 1096 440 1096 eais.bcExtEvent.INP
w 780 563 100 0 n#2 eaos.TODinterrupt.OUT 728 560 832 560 hwout.hwout#9.outp
w 416 1403 100 0 n#3 hwin.hwin#16.in 352 1400 480 1400 estringinval.timestamp.INP
w 432 627 100 0 n#4 eaos.TODinterrupt.DOL 472 624 392 624 392 704 536 704 536 768 496 768 eais.timesecs.VAL
w 876 275 100 0 n#5 hwin.hwin#23.in 824 272 928 272 eais.read_tod.INP
w 524 803 100 0 n#6 eais.timesecs.FLNK 496 800 552 800 552 712 376 712 376 592 472 592 eaos.TODinterrupt.SLNK
w 988 1019 100 0 n#7 hwin.hwin#26.in 960 1016 1016 1016 eais.ttt.INP
s 112 861 100 0 This specified time must be in the range 0.0 (midnight) to 86399.999 (23h59m59.999s) inclusive.
[cell use]
use bb200tr 0 0 100 0 bb200tr#1
xform 0 1280 800
use eais 516 976 100 0 bcExtEvent
xform 0 568 1064
p 556 1178 100 0 1 DTYP:Bancomm 635
p 554 1142 100 0 1 SCAN:I/O Intr
p 432 976 100 0 -1 PV:$(top)
p 513 996 100 0 1 PREC:6
use hwin 144 1080 100 0 hwin#3
xform 0 240 1096
p 147 1088 100 0 -1 val(in):#C0 S1
use eaos 539 490 100 0 TODinterrupt
xform 0 600 592
p 626 720 100 0 1 DTYP:Bancomm 635
p 624 686 100 0 1 SCAN:Passive
p 455 490 100 0 -1 PV:$(top)
p 541 510 100 0 1 PREC:12
p 468 462 100 0 1 OMSL:closed_loop
use hwout 832 544 100 0 hwout#9
xform 0 928 560
p 928 551 100 0 -1 val(outp):#C0 S3
use estringinval 543 1298 100 0 timestamp
xform 0 608 1368
p 569 1444 100 0 1 DTYP:Bancomm 635
p 480 1430 100 0 1 SCAN:1 second
use hwin 160 1384 100 0 hwin#16
xform 0 256 1400
p 163 1392 100 0 -1 val(in):#C0 S0
use eais 283 716 100 0 timesecs
xform 0 368 784
use estringouts 1040 1280 100 0 testWrite
xform 0 1168 1328
use eais 1004 152 100 0 read_tod
xform 0 1056 240
p 1044 354 100 0 1 DTYP:Bancomm 635
p 1042 318 100 0 1 SCAN:I/O Intr
p 920 152 100 0 -1 PV:$(top)
p 1001 172 100 0 1 PREC:6
use hwin 632 256 100 0 hwin#23
xform 0 728 272
p 635 264 100 0 -1 val(in):#C0 S3
use eais 1081 907 100 0 ttt
xform 0 1144 984
p 1119 886 100 0 1 DTYP:Bancomm 635
p 1117 868 100 0 1 SCAN:Passive
use hwin 768 1000 100 0 hwin#26
xform 0 864 1016
p 771 1008 100 0 -1 val(in):#C0 S1
[comments]
