cmake_minimum_required(VERSION 3.7)
project(server)

set(INCLUDE ./include ./snslib)

##include/.h
include_directories(../../deps/zeromq-4.2.1/include)
include_directories(../../deps/protobuf-master/src/)
include_directories(E:/LIB/linux_std_include/include)
include_directories(C:\\cygwin64\\usr\\include)
include_directories(
        ${INCLUDE}
)
include_directories(../../deps/glog/include)

##lib dir
link_directories(../../deps/glog/lib)
##

##sub dir
add_subdirectory(${INCLUDE})
##

aux_source_directory(./ SOURCES)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES ${SOURCES}
        snslib/comm/buffer/recv_buffer.h
        proto/msg_head.h
        include/singleton.h
        include/log_def.h timer_heap.h db.cpp db.h mq_mgr.cpp mq_mgr.h
        include/application_base.h include/application_base.cpp include/ansi_color.h include/ring_queue.h
        sort_set.h sort_set.cpp skiplist.h sldb.cpp sldb.h)

#set(TEMPH proto/src/bs_cmd.pb.h proto/src/bs_errcode.pb.h proto/src/bs_msg.pb.h)
add_executable(server ${SOURCE_FILES})
#add_definitions(-DLOG_DEF)
##.a/.so
target_link_libraries(server -lglog)
target_link_libraries(server -lpthread)
target_link_libraries(server sinc)