#!/bin/sh

# usp1.sh

# gcc -DKEEP_ALIVES -DIBM_APACHEBENCH -static ab.c	-lpthread -o bench1_keepalive
# gcc					  -DIBM_APACHEBENCH -static ab.c	-lpthread -o bench1_NO_keepalive

# const char* IP  = (argv[2]?	    argv[2] :"localhost");
# const char* URL = (argv[3]?	    argv[3] :"/index.html");
# int PORT			= (argv[4]?atoi(argv[4]):80);
# int FROM			= (argv[5]?atoi(argv[5]):0);
# int TO          = (argv[6]?atoi(argv[6]):1000);

# I had to increase the local port range (because of the TIME_WAIT status of the TCP ports)

# net.ipv4.ip_local_port_range = 1024 65535
/sbin/sysctl -w net/ipv4/ip_local_port_range="1024 65535"

 HOST=$1
#HOST=stefano
#HOST=giallo

mkdir -p		$HOST && chmod 777	 $HOST
mkdir -p	AB/$HOST && chmod 777 AB/$HOST

# ab -n 1000000 -c 10 -S -d -t 1    -H 'Accept-Encoding: gzip,deflate' "http://$HOST:8080/servlet/benchmarking?name=stefano" // NO Keep-Alives
# ab -n 1000000 -c 10 -S -d -t 1 -k -H 'Accept-Encoding: gzip,deflate' "http://$HOST:8080/servlet/benchmarking?name=stefano" // KEEP-ALIVES

date > AB/$1/start.txt

# 100.html
./bench1_keepalive	userver_tcp $HOST "/100.html" 8080 0 1000
mv $1/test.txt			AB/$1/userver_tcp_100_keepalive.csv
sleep 60
./bench1_NO_keepalive userver_tcp $HOST "/100.html" 8080 0 1000
mv $1/test.txt			AB/$1/userver_tcp_100_NO_keepalive.csv

sleep 60

# 1000.html
./bench1_NO_keepalive userver_tcp $HOST "/1000.html" 8080 0 1000
mv $1/test.txt			AB/$1/userver_tcp_1000_NO_keepalive.csv
sleep 60
./bench1_keepalive	userver_tcp $HOST "/1000.html" 8080 0 1000
mv $1/test.txt			AB/$1/userver_tcp_1000_keepalive.csv

sleep 60

# servlet/benchmarking?name=stefano
./bench1_NO_keepalive userver_tcp $HOST "/servlet/benchmarking?name=stefano"   8080 0 1000
mv $1/test.txt			AB/$1/usp_NO_keepalive.csv
sleep 60
./bench1_keepalive   userver_tcp  $HOST "/servlet/benchmarking?name=stefano"   8080 0 1000
mv $1/test.txt			AB/$1/usp_keepalive.csv

sleep 60

# ws/flash-bridge/WebSocketMain.swf
./bench1_keepalive   userver_tcp  $HOST "/ws/flash-bridge/WebSocketMain.swf"   8080 0 1000
mv $1/test.txt			AB/$1/userver_tcp_big_keepalive.csv
sleep 60
./bench1_NO_keepalive userver_tcp $HOST "/ws/flash-bridge/WebSocketMain.swf"   8080 0 1000
mv $1/test.txt			AB/$1/userver_tcp_big_NO_keepalive.csv

date > AB/$1/end.txt
