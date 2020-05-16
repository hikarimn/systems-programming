all: fw fwd

fw:fw.c
	gcc fw.c -o fw

fwd:fwd.c
	gcc fwd.c csapp.c -pthread -o fwd

clean:
	rm fw; rm fwd; rm pid; rm fwd.log
