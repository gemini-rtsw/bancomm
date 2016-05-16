[schematic2]
uniq 3
[tools]
[detail]
w 1212 875 100 0 n#1 hwin.hwin#3.in 1160 872 1264 872 eais.bcCurrTime.INP
w 1290 378 -100 0 n#2 eaos.TODinterrupt.OUT 1288 368 1392 368 hwout.hwout#9.outp
[cell use]
use bb200tr 0 0 100 0 bb200tr#1
xform 0 1280 800
use eais 1340 752 100 0 bcCurrTime
xform 0 1392 840
p 1380 954 100 0 1 DTYP:Bancomm 635
p 1378 918 100 0 1 SCAN:I/O Intr
p 1256 752 100 0 -1 PV:$(top)
p 1337 772 100 0 1 PREC:6
use hwin 968 856 100 0 hwin#3
xform 0 1064 872
p 971 864 100 0 -1 val(in):#C0 S0
use eaos 1099 298 100 0 TODinterrupt
xform 0 1160 400
p 1186 528 100 0 1 DTYP:Bancomm 635
p 1184 494 100 0 1 SCAN:Passive
p 1015 298 100 0 -1 PV:$(top)
p 1101 318 100 0 1 PREC:12
p 1417 405 100 0 1 OMSL:supervisory
use hwout 1392 352 100 0 hwout#9
xform 0 1488 368
p 1488 359 100 0 -1 val(outp):#C0 S3
[comments]
