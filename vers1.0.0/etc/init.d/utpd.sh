#! /bin/sh
#
# Start and stop utpd server

NAME=utpd
DAEMON=/usr/bin/$NAME
DAEMON_ARGS="-d -v"

# It's not safe to start if the daemon is not installed
test -x $DAEMON || exit 0

# It's needed start-stop-daemon
if [ ! -x /sbin/start-stop-daemon ]; then
    echo "Binary 'start-stop-daemon' is not found in your system"
    echo "If you want to install it on Ubuntu, type:"
    echo ""
    echo "\t$ sudo apt-get install dpkg"
    echo ""
    exit 0
fi

start() {
    echo "Starting $NAME server..."
    /sbin/start-stop-daemon --quiet -S -x $DAEMON -- $DAEMON_ARGS
    RETVAL=$?
}

stop() {
    echo "Stopping $NAME server..."
    /sbin/start-stop-daemon --quiet -K -n $NAME
    RETVAL=$?
}

case "$1" in
    start)
        start
        ;;

    stop)
        stop
        ;;

    restart)
        stop
        start
        ;;

    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
        ;;
esac

exit $RETVAL

