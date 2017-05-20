/*
 * static_buffer.h
 *
 *  Created on: 2010-12-16
 *      Author: jiffychen
 */

#ifndef STATIC_BUFFER_H_
#define STATIC_BUFFER_H_

#include "dynamic_buffer.h"

namespace snslib
{
    /**
     * Static buffers are immutable, can not grow after initialized.
     */
    struct StaticBuffer
    {
    	StaticBuffer() :
    		m_base_p(0), m_size(0), m_own(true), m_solid(false), m_ptr(0)
    	{
    	}

    	explicit StaticBuffer(size_t len) :
    		m_base_p(new char[len]),
    		m_size(len),
    		m_own(true),
    		m_solid(false),
    		m_ptr(m_base_p)
    	{
    	}

    	StaticBuffer(const char *data, size_t len, bool ownership = false) :
    		m_base_p(const_cast<char*>(data)),
    		m_size(len),
    		m_own(ownership),
    		m_solid(false),
    		m_ptr(m_base_p)
    	{
    	}

        StaticBuffer(const StaticBuffer &other_)
        {
        	StaticBuffer &other = const_cast<StaticBuffer &>(other_);
        	m_base_p = other.m_base_p;
        	m_size = other.m_size;
        	m_own = other.m_own;
        	m_solid = other.m_solid;
        	m_ptr = other.m_ptr;
        	if (m_own)
        	{
        		other.m_own = true;
        		other.m_solid = false;
        		other.m_size = 0;
        		other.m_base_p = 0;
        		other.m_ptr = 0;
        	}

        	//std::cout << "static buffer " << "own=" << m_own << ", solid=" << m_solid << std::endl;
        }

    	StaticBuffer(const DynamicBuffer &other_)
    	{
    		DynamicBuffer &other = const_cast<DynamicBuffer &>(other_);
    		m_base_p = other.m_base_p;
    		m_size = other.Size();
    		m_own = other.m_own;
    		m_solid = false;
    		m_ptr = m_base_p;
    		if (m_own)
    		{
    			other.m_own = false;
    			other.Free();
    		}
    	}

        StaticBuffer &operator=(const StaticBuffer &other_)
        {
			if (m_own && m_base_p) delete [] m_base_p;
 
        	StaticBuffer &other = const_cast<StaticBuffer &>(other_);
        	m_base_p = other.m_base_p;
        	m_size = other.m_size;
        	m_own = other.m_own;
        	m_solid = other.m_solid;
        	m_ptr = other.m_ptr;
        	if (m_own)
        	{
        		other.m_own = false;
        		other.m_solid = false;
        		other.m_size = 0;
        		other.m_base_p = 0;
        		other.m_ptr = 0;
        	}

        	//std::cout << "dst static buffer " << "own=" << m_own << ", solid=" << m_solid << ", base=" << (void*)m_base_p << std::endl;
        	//std::cout << "src static buffer " << "own=" << other.m_own << ", solid=" << other.m_solid << ", base=" << (void*)other.m_base_p << std::endl;

        	return *this;
        }

        StaticBuffer &operator=(const DynamicBuffer &other_)
        {
        	DynamicBuffer &other = const_cast<DynamicBuffer &>(other_);
        	m_base_p = other.m_base_p;
        	m_size = other.Size();
        	m_own = other.m_own;
        	m_solid = false;
        	m_ptr = m_base_p;
        	if (m_own)
        	{
    			other.m_own = false;
    			other.Free();
           	}

        	//std::cout << "dst static buffer " << "own=" << m_own << ", solid=" << m_solid << ", base=" << (void*)m_base_p << std::endl;
        	//std::cout << "src dynamic buffer " << "own=" << other.m_own << ", base=" << (void*)other.m_base_p << std::endl;
        	return *this;
        }

		StaticBuffer &Assign(const char *data, size_t len, bool ownership = false)
		{
			if (m_own && m_base_p) delete [] m_base_p;
 
			m_base_p = const_cast<char*>(data);
			m_size = len;
			m_own = ownership;
			m_solid = false;
			m_ptr = m_base_p;

			return *this;
		}


        inline bool Empty() const { return m_ptr == m_base_p + m_size; }
        inline size_t Remain() const { return m_size - (m_ptr - m_base_p); }

        inline void Solidit() { m_solid = true; }

        inline bool Solided() { return m_own == true || m_solid == true; };

        inline bool Advance(size_t len)
        {
        	if (len > Remain())
        	{
        		//std::cout << "advance fail, to_advance=" << len << ", remain=" << Remain() << std::endl;
        		return false;
        	}
        	m_ptr += len;
        	return true;
        }

        const char* Str() const { return m_ptr; }

        ~StaticBuffer()
        {
        	//std::cout << "~static buffer " << "own=" << m_own << ", solid=" << m_solid << ", base=" << (void*) m_base_p << std::endl;
        	if (m_own && m_base_p) delete [] m_base_p;
        }

        char *m_base_p;
        size_t m_size;
        bool m_own;
        bool m_solid; // solid means when own is false, we still don't need to
				      // create a dynamic buffer to store the data
        char *m_ptr; // read ptr
    };
}

#endif /* STATIC_BUFFER_H_ */
