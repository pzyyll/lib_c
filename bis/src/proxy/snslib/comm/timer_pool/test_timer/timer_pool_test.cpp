#include <iostream>
#include <map>
#include <deque>
#include "comm/timer_pool/timer_pool.h"

typedef unsigned long long uint64;


enum TimerStatus
{
  ADDED,
  REMOVED,
  EXPIRED,
  WAIT,
};

struct TimerData
{
  uint64 timer_id;
  TimerStatus status;
  time_t expire;
  uint64 signature;
};

std::map<uint64, TimerData> timer_list;

std::deque<uint64> removable_list;

snslib::CTimerPool<uint64> timer_pool;

int init(int size)
{
  srand(time(NULL));
  char *p_mem = new char[size];

  int ret = timer_pool.Init(p_mem, size, 1);
  if (ret)
  {
    PetLog(0, 0, PETLOG_ERR, "failed to init timer pool");
    return -1;
  }

  OpenPetLog("timer_pool_test", 5, "/tmp");
}

int loop(int runs_per_sec, int run_seconds)
{
  int runs_in_sec = 0;
  struct timeval last_time;
  struct timeval now;
  gettimeofday(&now, NULL);
  gettimeofday(&last_time, NULL);
  struct timeval start = now;
  while (now.tv_sec - start.tv_sec < run_seconds)
  {
    gettimeofday(&now, NULL);

    if (now.tv_sec != last_time.tv_sec)
    {
      runs_in_sec = 0;
      last_time = now;
    }

    if (runs_in_sec < runs_per_sec)
    {
      if (now.tv_sec - start.tv_sec < run_seconds - 150)
      {
        TimerData timer_data;
        timer_data.status = WAIT;
        timer_data.expire = now.tv_sec + rand() % 100 + 4;
        timer_data.signature = rand();
        int ret = timer_pool.AddTimer((timer_data.expire - now.tv_sec) * 1000, timer_data.signature, &timer_data.timer_id);
        if (ret)
        {
          PetLog(0, 0, PETLOG_ERR, "failed to add timer, ret=%d, err=%s", ret, timer_pool.GetErrMsg());
        }
        else
        {
          if (rand() % 10 >= 5)
          {
              removable_list.push_back(timer_data.timer_id);
          }
          PetLog(0, 0, PETLOG_INFO, "AddTimer|id=%llu, signature=%llu, status=%d, expire=%lu", \
              timer_data.timer_id, timer_data.signature, timer_data.status, timer_data.expire);
          if (timer_list.find(timer_data.timer_id) != timer_list.end())
          {
             PetLog(0, 0, PETLOG_WARN, "ConflitTimer|Add|status=%d, signature=%llu, expire=%lu, now=%lu, id=%llu", \
                 timer_list[timer_data.timer_id].status, timer_list[timer_data.timer_id].signature, \
                 timer_list[timer_data.timer_id].expire, now.tv_sec, timer_data.timer_id);
          }
          timer_list[timer_data.timer_id] = timer_data;
        
        }

        if (rand() % 10 >= 8 && removable_list.size() > 0)
        {
            uint64 del_timer = removable_list.front();
            removable_list.pop_front();

            if (timer_list[del_timer].status != WAIT)
            {
              PetLog(0, 0, PETLOG_WARN, "del a timer with status %d, timer_id=%llu", timer_list[del_timer].status, del_timer);
            }

            ret = timer_pool.DelTimer(del_timer);
            if (ret)
            {
               PetLog(0, 0, PETLOG_ERR, "failed to del timer, ret=%d, err=%s", ret, timer_pool.GetErrMsg());
            }
            else
            {
              timer_list[del_timer].status = REMOVED;
            }
        }
      }
      runs_in_sec ++;
    }

      std::vector<uint64> v_timer_param;
      std::vector<uint64> v_timer_id;

      int ret = timer_pool.GetTimer(v_timer_id, v_timer_param);
      if (ret)
      {
        PetLog(0, 0, PETLOG_ERR, "failed to get timer, ret=%d, err=%s", ret, timer_pool.GetErrMsg());
      }
      else
      {
        for (size_t i = 0; i < v_timer_id.size(); i++)
        {
          uint64 expired_timer = v_timer_id[i];

          PetLog(0, 0, PETLOG_INFO, "ExpireTimer|id=%llu, signature=%llu", \
              expired_timer, v_timer_param[i]);


          if (timer_list.find(expired_timer) == timer_list.end())
          {
            PetLog(0, 0, PETLOG_WARN, "ConfilitTimer|Get|expired timer_id=%llu, but not found", expired_timer);
            continue;
          }
          TimerData& expired_timer_data = timer_list[expired_timer];
          if (expired_timer_data.status != WAIT || expired_timer_data.signature != v_timer_param[i])
          {
            PetLog(0, 0, PETLOG_WARN, "ConflictTimer|Get|expired timer_id=%llu, signature=%llu, \
                but expected signature=%llu, status=%d", expired_timer,
                v_timer_param[i], expired_timer_data.signature, expired_timer_data.status);
          }
          else
          {
            expired_timer_data.status = EXPIRED;
          }
        }
      }
  }

  for (std::map<uint64, TimerData>::iterator it = timer_list.begin(); it != timer_list.end(); it++)
  {
    if (it->second.status == WAIT)
    {
      PetLog(0, 0, PETLOG_WARN, "TimerNotExpired|timer_id=%llu, signature=%llu",
          it->second.timer_id, it->second.signature);
    }
  }

  return 0;
}

int main()
{
  if (init(204800000))
  {
    return -1;
  }

  return loop(2000, 1000);
}
