#!/bin/sh

# csp1.sh

# gcc -DU_KEEP_ALIVES -static bench1.c	-o bench1_keepalive
# gcc						 -static bench1.c	-o bench1_NO_keepalive

# const char* IP  = (argv[1]?	    argv[1] :"localhost");
# const char* URL = (argv[2]?	    argv[2] :"/index.html");
# int PORT			= (argv[3]?atoi(argv[3]):80);
# int FROM			= (argv[4]?atoi(argv[4]):0);
# int TO          = (argv[5]?atoi(argv[5]):1000);

# I had to increase the local port range (because of the TIME_WAIT status of the TCP ports)

# net.ipv4.ip_local_port_range = 1024 65535
/sbin/sysctl -w net/ipv4/ip_local_port_range="1024 65535"

 HOST=$1
#HOST=stefano
#HOST=giallo

mkdir -p		$HOST && chmod 777	 $HOST
mkdir -p	AB/$HOST && chmod 777 AB/$HOST

# 100.html
./bench1_keepalive    $HOST "/100.html" 8080 0 1000
mv $1/test.txt			AB/$1/gwan_100_keepalive.csv
sleep 60
./bench1_NO_keepalive $HOST "/100.html" 8080 0 1000
mv $1/test.txt			AB/$1/gwan_100_NO_keepalive.csv

sleep 60

# 1000.html
./bench1_NO_keepalive $HOST "/1000.html"	8080 0 1000
mv $1/test.txt			AB/$1/gwan_1000_NO_keepalive.csv
sleep 60
./bench1_keepalive    $HOST "/1000.html"	8080 0 1000
mv $1/test.txt			AB/$1/gwan_1000_keepalive.csv

sleep 60

# csp?hellox&name=stefano
./bench1_NO_keepalive $HOST "/csp?hellox&name=stefano" 8080 0 1000
mv $1/test.txt			AB/$1/csp_NO_keepalive.csv
sleep 60
./bench1_keepalive    $HOST "/csp?hellox&name=stefano" 8080 0 1000
mv $1/test.txt			AB/$1/csp_keepalive.csv

sleep 60

# ws/flash-bridge/WebSocketMain.swf
./bench1_keepalive    $HOST "/ws/flash-bridge/WebSocketMain.swf" 8080 0 1000
mv $1/test.txt			AB/$1/gwan_big_keepalive.csv
sleep 60
./bench1_NO_keepalive $HOST "/ws/flash-bridge/WebSocketMain.swf" 8080 0 1000
mv $1/test.txt			AB/$1/gwan_big_NO_keepalive.csv
