all: fwd

fw:	fwd.c
	gcc fwd.c csapp.c -pthread -o fwdd

install:
	cp fwd.conf /etc/fwd.conf
	rm -rf /var/fwd # You do not need this step
	mkdir /var/fwd # You do not need this step
	cp fwd.service lib/systemd/system/fwd.service
	cp fwd /usr/sbin/fwdd