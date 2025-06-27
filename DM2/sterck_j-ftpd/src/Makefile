include ../Makefile.rules
SRC=	main.c\
	ftp.c\
	my_malloc.c\
	ufile.c\
	client.c\
	ftp_basics.c\
	ftp_basics2.c\
	ftp_basics3.c\
	ftp_cwd.c\
	ftp_list.c\
	ftp_retr.c\
	ftp_stor.c\
	reply.c\
	serv.c\
	serv_client.c\
	net.c\
	type.c
OBJ=$(SRC:.c=.o)

all: $(DST) ftp.h

$(DST): $(OBJ) ftp.h
	$(CC) $(OBJ) -o $(DST) $(LFLAGS)

.c.o: ftp.h
	$(CC) $(FLAGS) -o $@ -c $<

clean:
	rm -rf *.o *~ *\#*

distclean: clean
	rm -f $(DST)
