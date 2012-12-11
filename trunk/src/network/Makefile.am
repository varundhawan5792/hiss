SUBDIRS = web_server

SOURCES = network.cpp sockets.cpp sendrecv.cpp headers.cpp waitio.cpp sockopts.cpp communication.cpp

noinst_LTLIBRARIES    = libnetwork.la 
libnetwork_la_SOURCES = $(SOURCES)
libnetwork_la_LIBADD = web_server/libweb_server.la

