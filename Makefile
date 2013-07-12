CFLAGS+=-Wall -g

all:  statsd-gearmand-monitor

statsd-gearmand-monitor:
	gcc -o statsd-gearmand-monitor statsd-gearmand-monitor.c

clean:
	rm -f statsd-gearmand-monitor
