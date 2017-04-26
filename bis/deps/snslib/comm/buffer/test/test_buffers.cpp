/*
 * test_buffers.cpp
 *
 *  Created on: 2010-12-16
 *      Author: jiffychen
 */

#include <stdio.h>
#include <iostream>
#include <deque>
#include <tr1/memory>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "comm/buffer/dynamic_buffer.h"
#include "comm/buffer/static_buffer.h"
#include "comm/util/pet_util.h"
#include "comm/buffer/buffer_sequence.h"

using namespace std;
using namespace snslib;

void test_static_buffer1()
{
	char* data = new char[10240000];
	strcpy(data, "wahaha");
	StaticBuffer sb(data, strlen(data));
	std::cout << sb.Str() << " " << sb.Remain() << std::endl;

	StaticBuffer new_sb(sb);
	std::cout << new_sb.Str() << " " << new_sb.Remain() << std::endl;
	std::cout << (void*)sb.Str() << " " << sb.Remain() << std::endl;
	std::cout << (void*)sb.m_ptr << std::endl;
	std::cout << (void*)sb.m_base_p << std::endl;

	StaticBuffer sb2(data, strlen(data), true);
	std::cout << sb2.Str() << " " << sb2.Remain() << std::endl;

	StaticBuffer new_sb2(sb2);
	std::cout << new_sb2.Str() << " " << new_sb2.Remain() << std::endl;
	std::cout << (void*)sb2.Str() << " " << sb2.Remain() << std::endl;
	std::cout << (void*)sb2.m_ptr << std::endl;
	std::cout << (void*)sb2.m_base_p << std::endl;
}

void test_static_buffer2()
{
	DynamicBuffer db("wahaha", strlen("wahaha"));
	std::cout << db.Str() << " " << db.Size() << " " << db.Remain() << std::endl;

	db.Ensure(1024, true);

	std::cout << db.Str() << " " << db.Size() << " " << db.Remain() << std::endl;

	strcpy(db.Str(), "hahawa");
	db.Ensure(2048, true);
	std::cout << db.Str() << " " << db.Size() << " " << db.Remain() << std::endl;

	char* t = db.Borrow(8192);
	strcpy(t, "haha");
	db.Advance(strlen("haha"));

	std::cout << db.Str() << " " << db.Size() << " " << db.Remain() << std::endl;


	StaticBuffer sb(db);

	std::cout << db.Str() << " " << db.Size() << " " << db.Remain() << std::endl;

	std::cout << sb.Str() << " " << sb.Remain() << std::endl;
}

void test_dynamic_buffer1()
{
        snslib::DynamicBuffer db1;
        db1.Append("{\"header\":{\"uin\":%u,\"version\":%hu,\"app\":%hu,\"zone\":%hu,\"cmd\":%hu},\"data\":",
                149511433, 2, 103, 4, 0x1007);
}

void test_dynamic_buffer2()
{

}

void test_buffer_sequence0()
{
	BufferSequence<3> bs;
	char * s1 = "abcdefghij";
	char * s2 = "abcdefghi";
	char * s3 = "abcdefgh";
	StaticBuffer sb1(s1, strlen(s1));
	StaticBuffer sb2(s2, strlen(s2));
	StaticBuffer sb3(s3, strlen(s3));

	bs += sb1;
	bs += sb2;
	bs += sb3;

	unsigned short checksum = bs.CheckSum();

	std::cout << "checksum=" << checksum << std::endl;

	char * s = "abcdefghijabcdefghiabcdefgh";
	unsigned short newcheck = CStrTool::CheckSum(s, strlen(s));

	std::cout << "should be " << newcheck << std::endl;
}

void test_buffer_sequence1()
{
	BufferSequence<3> bs;
	char * fmt = "{header:{uin:%d,cmd:%d,key:%s},data:";
	char * s1 = "{header:{uin:1,cmd=2,key=3199812121321kudafdfwqwq123121},data:";
	char * s2 = "}\0";
	DynamicBuffer sb1(s1, strlen(s1));
	sb1.Append(fmt, 1, 2, "3199812121321kudafdfwqwq123121");
	StaticBuffer sb2(s1, strlen(s1));
	StaticBuffer sb3(s2, strlen(s2) + 1);

	std::cout << "sb1=" << sb1.Str() << std::endl;

	sb3.Solidit();
	//sb2.Solidit();

	bs += sb1;
	bs += sb2;
	bs += sb3;

	bs.Solidify();

	struct iovec vec[2];
	int count = bs.InitIoVec(vec);
	std::cout << "remain=" << bs.Remain() << std::endl;
	std::cout << "iobuff=" << count << std::endl;

	bs.Advance(100);

	count = bs.InitIoVec(vec);
	std::cout << "remain=" << bs.Remain() << std::endl;
	std::cout << "iobuff=" << count << std::endl;

	bs.Advance(100);

	count = bs.InitIoVec(vec);
	std::cout << "remain=" << bs.Remain() << std::endl;
	std::cout << "iobuff=" << count << std::endl;

}

void test_buffer_sequence2()
{
	char * fmt = "{header:{uin:%d,cmd:%d,key:%s,request:%d,zone:%d,app:%d,version:%d},data:";
	char * s2 = new char[3800];
	char * s3 = "}\0";

	boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();

	for (int i = 0; i < 50000000; i++)
	{
		//BufferSequence<3> bs;
		//DynamicBuffer sb1(fmt, 40);
		DynamicBuffer sb1;
		sb1.Append(fmt, 1, 2, "3199812121321kudafdfwqwq123121", 120, 103, 103, 1);
		//StaticBuffer sb2(s2, 3800);
		//StaticBuffer sb3(s3, 2);

		//sb3.Solidit();

		//bs += sb1;
		//bs += sb2;
		//bs += sb3;

		//bs.Solidify();

/*
		std::string s;
		s.append(fmt, 40);
		s.append(s2, 3800);
		s.append(s3, 2);
		*/
	}

    boost::posix_time::ptime end = boost::posix_time::microsec_clock::universal_time();
    boost::posix_time::time_duration diff = end - start;
    printf("time elapsed seconds %d.%lld \n", diff.total_seconds(), diff.fractional_seconds());
}

void test_buffer_sequence3()
{
	typedef std::tr1::shared_ptr<BufferSequence<3> > BufferSequencePtr;
	char * fmt = "{header:{uin:%d,cmd:%d,key:%s,request:%d,zone:%d,app:%d,version:%d},data:";
	char * s2 = new char[3800];
	char * s3 = "}\0";

	std::deque<BufferSequence<3> > queue;
	//std::deque<BufferSequencePtr> queue;
	//std::deque<std::string> queue;

	boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();

	for (int i = 0; i < 50000000; i++)
	{
		BufferSequence<3> bs;
		//BufferSequencePtr bs_p(new BufferSequence<3>());
		DynamicBuffer sb1(fmt, 40);
		//sb1.Append(fmt, 1, 2, "3199812121321kudafdfwqwq123121", 120, 103, 103, 1);
		StaticBuffer sb2(s2, 3800);
		StaticBuffer sb3(s3, 2);

		sb3.Solidit();
		sb2.Solidit();

		//bs_p->operator+=(sb1);
		//bs_p->operator+=(sb2);
		//bs_p->operator+=(sb3);
		bs += sb1;
		bs += sb2;
		bs += sb3;

		//bs_p->Solidify();
		bs.Solidify();

		//queue.push_back(bs_p);
		queue.push_back(bs);
		queue.pop_front();

/*
		std::string s;
		s.append(fmt, 40);
		s.append(s2, 3800);
		s.append(s3, 2);
		*/
	}

    boost::posix_time::ptime end = boost::posix_time::microsec_clock::universal_time();
    boost::posix_time::time_duration diff = end - start;
    printf("time elapsed seconds %d.%lld \n", diff.total_seconds(), diff.fractional_seconds());
}

int main()
{
	//close(stdout);
	//test_static_buffer1();
	//test_static_buffer2();
	//test_dynamic_buffer1();
	//test_dynamic_buffer2();
	test_buffer_sequence2();
	//test_buffer_sequence1();
	//test_buffer_sequence2();
	//test_buffer_sequence0();
	//test_buffer_sequence3();

	return 0;
}
