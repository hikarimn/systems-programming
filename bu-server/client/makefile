all: 
	fwdd

fwdd:	fwd.c
	gcc fwd.c csapp.c -pthread -o fwdd

install:
	cp fwd.conf /etc/fwd.conf
	rm -rf /var/fwd
	mkdir /var/fwd
	cp fwd.service lib/systemd/system/fwd.service
	cp fwdd /usr/sbin/fwdd