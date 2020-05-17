all: adder tinyd

adder: cgi-bin/adder.c
	gcc cgi-bin/adder.c csapp.c -pthread -I. -o cgi-bin/adder

tinyd: tiny.c
	gcc tiny.c csapp.c -pthread -o tinyd

install:
	cp tiny.conf /etc/tiny.conf
	rm -rf /var/tiny
	mkdir /var/tiny
	cp www/home.html /var/tiny/home.html
	mkdir /var/tiny/cgi-bin
	cp cgi-bin/adder /var/tiny/cgi-bin/adder
	cp tiny.service /lib/systemd/system/tiny.service
	cp tinyd /usr/sbin/tinyd
