bin_PROGRAMS 	    = simple_client

simple_client_SOURCES = simple_client.cpp
simple_client_LDADD   = ../../src/.libs/libneweraHPC.a $(INITLIBS)

../../src/.libs/libneweraHPC.a:
	make -C "../../src/"
