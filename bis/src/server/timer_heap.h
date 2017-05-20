//
// @Created by czllo
//

#ifndef SERVER_TIMER_HEAP_H_H
#define SERVER_TIMER_HEAP_H_H

#include <queue>
#include <vector>

#include <unistd.h>
#include <ctime>
#include <sys/time.h>

struct ExpireTimer
{
	struct timeval expire;
    int type;
	unsigned int task_pos;
};

inline int CompareTime(const struct timeval *tv1, const struct timeval *tv2){
	if (tv1->tv_sec > tv2->tv_sec) return 1;
	if (tv1->tv_sec < tv2->tv_sec) return -1;
	return tv1->tv_usec - tv2->tv_usec;
}

struct LtTimer {
	bool operator()(const ExpireTimer &t1, const ExpireTimer &t2) const {
		return CompareTime(&t1.expire, &t2.expire) > 0;
	}
};


typedef std::priority_queue<ExpireTimer, std::vector<ExpireTimer>, LtTimer>
	TimerHeap;

template <typename T, typename S, typename C>
S& Container(std::priority_queue<T, S, C> &q) {
	struct HackedQueue : private std::priority_queue<T, S, C> {
		static S& Container(std::priority_queue<T, S, C> &q) {
			return q.*&HackedQueue::c;
		}
	};
	return HackedQueue::Container(q);
}

#endif //SERVER_TIMER_HEAP_H_H
