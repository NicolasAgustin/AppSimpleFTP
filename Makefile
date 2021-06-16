CC=gcc
FLAGS=-g -Wall

ftp: myftpsrv_skel.o myftp_skel.o srvfunctions.o clientfunctions.o
	$(CC) $(FLAGS) -o ftpServer myftpsrv_skel.o srvfunctions.o -I .
	$(CC) $(FLAGS) -o ftpClient myftp_skel.o clientfunctions.o -I .
	rm -f *.o

myftp_skel.o:
	$(CC) -c myftp_skel.c
myftpsrv_skel.o:
	$(CC) -c myftpsrv_skel.c

srvfunctions.o:
	$(CC) -c srvfunctions.c

clientfunctions.o:
	$(CC) -c clientfunctions.c

clean:
	$(RM) *.o
