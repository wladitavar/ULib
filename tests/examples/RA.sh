#!/bin/sh

. ../.function

DOC_ROOT=/srv/RA/www

rm -f RA.log /tmp/processCGIRequest.err \
		$DOC_ROOT/client/*/*/ \
		trace.*userver_tcp*.[0-9]* object.*userver_tcp*.[0-9]* stack.*userver_tcp*.[0-9]* \
 		$DOC_ROOT/trace.*userver_tcp*.[0-9]* $DOC_ROOT/object.*userver_tcp*.[0-9]* $DOC_ROOT/stack.*userver_tcp*.[0-9]*

 UTRACE="0 50M 0"
#UOBJDUMP="0 100k 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

DIR_CMD="../../examples/userver"

mkdir -p /var/run /var/log /tmp/client/scan # /tmp/scan /tmp/mail
chown -R ra: /srv/RA /tmp/client

#STRACE=$TRUSS
 start_prg_background userver_tcp -c RA.cfg

#$SLEEP
#kill_prg userver_tcp TERM

mv err/userver_tcp.err err/RA.err
