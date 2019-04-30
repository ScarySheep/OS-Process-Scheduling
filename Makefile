CC = gcc
.PHONY:	clean
objs:= main.o scheduler.o process.o 
project1: $(objs)
	$(CC) -o project1 $(objs)
main.o: main.c process.h scheduler.h
	$(CC) main.c -c
scheduler.o: scheduler.c scheduler.h
	$(CC) scheduler.c -c
process.o: process.c process.h
	$(CC) process.c -c
clean:
	rm -f $(objs)
run:
	sudo ./project1
