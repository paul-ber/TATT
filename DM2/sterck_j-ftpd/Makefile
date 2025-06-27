include Makefile.rules

all:
	cd src && make && cp -f $(DST) ..

clean:
	rm -rf *.o *~ *\#* *.core
	cd src && make clean

distclean: clean
	cd src && make distclean
	rm -rf *.so *.a Makefile.rules $(DST)

dist: distclean
	find . -type f -exec chmod 600 {} \;
	find . -type d -exec chmod 700 {} \;
	chmod 700 configure
	cd .. && tar cjvvf sterck_j-ftpd.tar.bz2 sterck_j-ftpd
