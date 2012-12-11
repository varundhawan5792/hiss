bin_PROGRAMS 	    = rbtree_test thread_test tree_test cfg_test strings_test rb_queue file_test

rbtree_test_SOURCES = rbtree_test.cpp
rbtree_test_LDADD   = ../../src/.libs/libneweraHPC.a $(INITLIBS)

thread_test_SOURCES = thread_test.cpp
thread_test_LDADD   = ../../src/.libs/libneweraHPC.a $(INITLIBS)

tree_test_SOURCES = thread_test.cpp
tree_test_LDADD   = ../../src/.libs/libneweraHPC.a $(INITLIBS)

cfg_test_SOURCES = cfg_test.cpp
cfg_test_LDADD   = ../../src/.libs/libneweraHPC.a $(INITLIBS)

strings_test_SOURCES = strings_test.cpp
strings_test_LDADD   = ../../src/.libs/libneweraHPC.a $(INITLIBS)

rb_queue_SOURCES = rb_queue.cpp
rb_queue_LDADD   = ../../src/.libs/libneweraHPC.a $(INITLIBS)

file_test_SOURCES = file_test.cpp
file_test_LDADD   = ../../src/.libs/libneweraHPC.a $(INITLIBS)

../../src/.libs/libneweraHPC.a:
	make -C "../../src/"
