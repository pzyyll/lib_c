/*
 * auto_cleanup.h
 *
 *  Created on: 2010-9-6
 *      Author: jiffychen
 */

#ifndef AUTO_CLEANUP_H_
#define AUTO_CLEANUP_H_

#include <boost/function.hpp>

namespace snslib
{
class AutoCleanup
{
public:
	AutoCleanup(boost::function0<void> fCommitedCallback, boost::function0<void> fUncommitedCallback) :
		m_fCommitedCallback(fCommitedCallback), m_fUncommitedCallback(fUncommitedCallback), m_bCommited(false)
	{
	}

	AutoCleanup(boost::function0<void> fUncommitedCallback) :
		m_fUncommitedCallback(fUncommitedCallback), m_bCommited(false)
	{
	}

	inline void Commit()
	{
		m_bCommited = true;
	}

	~AutoCleanup()
	{
		if (!m_bCommited)
		{
			if (m_fUncommitedCallback)
				m_fUncommitedCallback();
		}
		else
		{
			if (m_fCommitedCallback)
				m_fCommitedCallback();
		}
	}

private:
	boost::function0<void> m_fCommitedCallback;
	boost::function0<void> m_fUncommitedCallback;
	bool m_bCommited;
};
}

#endif /* AUTO_CLEANUP_H_ */
