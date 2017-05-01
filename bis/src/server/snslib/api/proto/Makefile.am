AUTOMAKE_OPTIONS=foreign

INCLUDES=-I..

noinst_LIBRARIES=libproto.a

libproto_a_SOURCES=lock.pb.cc  flag.pb.cc qqlist2.pb.cc qqinfo2.pb.cc snsapp.pb.cc router.pb.cc

PROTOC=protoc
PROTO_PATH=.
CPP_OUT=.

%.pb.cc: %.proto
	$(PROTOC) --proto_path=$(PROTO_PATH) --cpp_out=$(CPP_OUT) $^

clean:
	test -z $(noinst_LIBRARIES) || rm -f $(noinst_LIBRARIES)
	rm -f *.o *.pb.h *.pb.cc

AUTOMAKE_OPTIONS=foreign

INCLUDES=-I..

noinst_LIBRARIES=libproto.a

libproto_a_SOURCES=lock.pb.cc flag.pb.cc  qqlist2.pb.cc qqinfo2.pb.cc    snsapp.pb.cc router.pb.cc

PROTOC=protoc
PROTO_PATH=.
CPP_OUT=.

%.pb.cc: %.proto
	$(PROTOC) --proto_path=$(PROTO_PATH) --cpp_out=$(CPP_OUT) $^

clean:
	test -z $(noinst_LIBRARIES) || rm -f $(noinst_LIBRARIES)
	rm -f *.o *.pb.h *.pb.cc

