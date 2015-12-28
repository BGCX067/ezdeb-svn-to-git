#! /bin/sh
#
### BEGIN INIT INFO
# Provides:          dhis-server
# Required-Start:    $network
# Required-Stop:     $network
# Should-Start:      $syslog $named
# Should-Stop:       $syslog $named
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start the Dynamic Host Information System server
# Description:       DHIS is a client-server architecture meant to update
#                    databases for systems which are assigned a dynamic IP[v4]
#                    address. By the means of a DHIS client a host which is
#                    assigned a dynamic IP address (either from its ISP or from
#                    DHCP) is able to communicate with a DHIS server in order
#                    to advertise its newly acquired IP address.
### END INIT INFO

PATH=/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=/usr/sbin/dhisd
NAME=dhisd
DESC=dhis-server

test -f $DAEMON || exit 0

set -e

case "$1" in
  start)
	echo -n "Starting $DESC: $NAME"
	start-stop-daemon --start --quiet --pidfile /var/run/$NAME.pid \
		--exec $DAEMON
	echo "."
	;;
  stop)
	echo -n "Stopping $DESC: $NAME"
	start-stop-daemon --oknodo --stop --quiet --pidfile /var/run/$NAME.pid \
		--exec $DAEMON
	echo "."
	;;
  reload|force-reload)
	echo "Reloading $DESC configuration files."
	start-stop-daemon --stop --signal 1 --quiet --pidfile \
		/var/run/$NAME.pid --exec $DAEMON
  	;;
  restart)
	echo -n "Restarting $DESC: $NAME"
	start-stop-daemon --oknodo --stop --quiet --pidfile \
		/var/run/$NAME.pid --exec $DAEMON
	sleep 1
	start-stop-daemon --start --quiet --pidfile \
		/var/run/$NAME.pid --exec $DAEMON
	echo "."
	;;
  *)
	N=/etc/init.d/$NAME
	echo "Usage: $N {start|stop|restart|reload|force-reload}" >&2
	exit 1
	;;
esac

exit 0
