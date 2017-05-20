/*
 * =====================================================================================
 *
 *       Filename:  shm_queue_ex.h
 *
 *    Description:  保证单进程唯一性的队列
 *
 *        Version:  1.0
 *        Created:  2010年03月05日 15时04分31秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Michaelzhao (赵广宇), michaelzhao@tencent.com
 *        Company:  Tencent
 *
 * Warning: 只能保证单进程唯一！！！！！！
 * 需要DataType提供UniqueKey方法用来得到唯一标识UniqueKey
 * =====================================================================================
 */

#pragma once

#include "shm_queue_ex.h"
#include <set>

namespace snslib
{

template<typename KeyType, typename DataType>
class CShmQueueUnique : public CShmQueueEx<DataType>
{
	typedef CShmQueueEx<DataType> Base;

public:
	const static int SUCCESS = 0;
    const static int ERROR = -1;

	const static int E_SHM_QUEUE_DUP_KEY = -611;

	class InitCallback : public CShmQueueEx<DataType>::Callback{
		public:
			std::set<KeyType> &key_set;
			int result;

			InitCallback(std::set<KeyType> &ks)	: key_set(ks), result(SUCCESS) {
				key_set.clear();
			}

			bool Exec(DataType &data){
				if(!key_set.insert(data.UniqueKey()).second){
					result = E_SHM_QUEUE_DUP_KEY;
					return false;
				}
				return true;
			}
	};

	int Init(int shm_key, unsigned int capacity){
		int rv = Base::Init(shm_key, capacity);
		if(rv == SUCCESS){
			InitCallback callback(m_key_set);
			Base::ForEach(&callback);
			rv = callback.result;
		}
		return rv;
	}

	int Push(const DataType &data){
		int rv = SUCCESS;
		Base::SemLock();

		if(m_key_set.insert(data.UniqueKey()).second){
			rv = Base::Push_Nolock(data);
			if(rv != SUCCESS){
				m_key_set.erase(data.UniqueKey());
			}
		}
		else{
			rv = E_SHM_QUEUE_DUP_KEY;
		}

		Base::SemRelease();
		return rv;
	}

	int Pop(DataType *data = NULL){
		DataType tmp;
		if(data == NULL)
			data = &tmp;

		Base::SemLock();

		int rv = Pop_Nolock(data);
		m_key_set.erase(data->UniqueKey());

		Base::SemRelease();

		return rv;
	}

private:
	std::set<KeyType> m_key_set;
};

}
