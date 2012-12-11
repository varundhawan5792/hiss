ACLOCAL_AMFLAGS= -I m4

SUBDIRS = src tests

bin_PROGRAMS   = server
server_SOURCES = server.cpp
server_LDADD   = src/libneweraHPC.la -ldl 

library_includedir      = $(includedir)/neweraHPC
library_include_HEADERS = include/*.h

dist_doc_DATA = README

src/libneweraHPC.la:
	cd src && make
   
