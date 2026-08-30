5 poke 53280,0:poke 53281,0:print chr$(147)
6 restore:p=0:read cx:read cy:dx=16:dy=26
10 xr=cx-19.5*dx:yi=cy-12*dy
20 sa=1024:ca=55296
30 ci=yi:for j=0 to 24
40 cr=xr:for i=0 to 39
50 zr=0:zi=0:k=0
60 rr=zr*zr/256:ii=zi*zi/256:if rr+ii>1024 then 100
70 nz=zr*zi*2/256
80 zi=nz+ci:zr=rr-ii+cr:k=k+1:if k<256 then 60
90 poke sa,160:poke ca,0:goto 120
100 c=k and 15:if c=0 then c=6
110 poke sa,160:poke ca,c
120 cr=cr+dx:sa=sa+1:ca=ca+1:next
130 ci=ci+dy:next
135 dx=dx*0.95:dy=dy*0.95:if dx>0.007 then 10
136 p=p+1:if p>4 then p=0:restore
137 read cx:read cy:dx=16:dy=26:goto 10
138 data -190.797,28.851,67.84,-0.896,-454.963,-0.512,-320,5.12,-22.733,166.528
