Reading :
=========
Example with alias definition file:
./readtag -f ./readtag.conf -a flex -d myint

Example with the default alias definition file (/etc/tuxeip.conf):
./readtag -a flex -d myint

Example without a configuration file
./readtag -p 10.140.200.45,1,0 -c LGX -r CNET -n 0 -d myfloat

Example without a configuration file on a MicroLogix 1100 ref: 1763-L16BWA
./readtag -p 10.140.200.58,1,0 -c SLC -r CNET -n 0 -d -l3 N7: 0


Writing:
==========
Example with default alias definition file (/etc/tuxeip.conf):
./readtag -a mybool count -w 1
