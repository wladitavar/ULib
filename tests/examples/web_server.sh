#!/bin/sh

. ../.function

start_msg web_server

rm -f web_server.log \
      out/userver_tcp.out err/userver_tcp.err \
		trace.*userver_tcp*.[0-9]* object.*userver_tcp*.[0-9]*

 UTRACE="0 10M 0"
 UOBJDUMP="0 100k 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

DIR_CMD="../../examples/userver"

( cd ../../src/ulib/plugin/.libs; ln -sf ../mod_shib/.libs/mod_shib.so )

#STRACE=$TRUSS
start_prg_background userver_tcp -c web_server.cfg

#$SLEEP
#kill_prg userver_tcp TERM

mv err/userver_tcp.err err/web_server.err

#openssl s_client -debug -cert ../ulib/CA/username.crt -key ../ulib/CA/username.key -pass pass:caciucco -CApath ../ulib/CA/CApath -verify 0 -connect localhost:80
