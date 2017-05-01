/*
 * dynamic_buffer.h
 *
 *  Created on: 2010-12-16
 *      Author: jiffychen
 */

#ifndef DYNAMIC_BUFFER_H_
#define DYNAMIC_BUFFER_H_

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <iostream>

namespace snslib
{
    /**
     * Dynamic buffers can grow.
     */
    struct DynamicBuffer
    {
        static size_t const SMALL_BUFF_SIZE = 4096;

    	DynamicBuffer() :
    		m_base_p(0), m_capacity(0), m_own(true),
    		m_ptr(m_base_p), m_offset(0)
    	{
    	}

    	explicit DynamicBuffer(size_t len) :
    		m_capacity(len > SMALL_BUFF_SIZE ? len : SMALL_BUFF_SIZE),
    		m_own(true),
                m_offset(0)
    	{
    		m_base_p = new char[m_capacity];
    		m_ptr = m_base_p;
    	}

    	// make a deep copy
    	DynamicBuffer(char *data, size_t len) :
                m_offset(0)
    	{
    		Construct(data, len);
    	}

    	// don't make a deep copy
    	DynamicBuffer(char *data, size_t len, bool nocopy) :
                m_offset(0)
    	{
		if (!nocopy)
	    		Construct(data, len);
		else
		{
			m_base_p = data;
			m_capacity = len;
			m_ptr = m_base_p;
			m_own = false;
		}
    	}

    	// don't make a deep copy
    	DynamicBuffer(char *data, size_t len, bool nocopy, size_t offset) :
                m_offset(offset)
    	{
		if (!nocopy)
	    		Construct(data, len);
		else
		{
			m_base_p = data;
			m_capacity = len;
			m_ptr = m_base_p;
			m_own = false;
		}
                assert(Advance(offset));
    	}

        ~DynamicBuffer()
        {
        	if (m_own && m_base_p)
        	{
        		delete [] m_base_p;
        	}
        }

        void Reset()
        {
        	if (m_own && m_base_p)
        	{
        		delete [] m_base_p;
        	}

    		m_base_p = 0;
    		m_capacity = 0;
    		m_own = true;
    		m_ptr = m_base_p;
                m_offset = 0;
        }

        char *m_base_p;
        size_t m_capacity; // maximum capacity
        bool m_own;
        char *m_ptr; // m_ptr - m_base_p is buffer size
        size_t m_offset; // offset reserved for this dynamic buffer

        inline size_t Size() const { return m_ptr - m_base_p; }
        inline size_t Remain() const { return m_capacity - (m_ptr - m_base_p); }
        inline size_t Capacity() const { return m_capacity; }
        inline char* Str() const { return m_base_p; }
        inline size_t Offset() const { return m_offset; }
        inline char* OffsetPtr() const { return m_base_p + m_offset; }
        

        inline void Free()
        {
        	if (m_own && m_base_p)
        		delete [] m_base_p;

        	m_base_p = 0;
        	m_ptr = 0;
        	m_capacity = 0;
        	m_own = true;
                m_offset = 0;
        }

        // Ensure there are more then len bytes available
        // make 1/2 extra space if necessary
        inline void Ensure(size_t len, bool nocopy = false)
        {
        	if (len > m_capacity)
        		Grow(len * 3 / 2, nocopy);
        }

        // Ensure there are more then len bytes available
        // make 1/2 extra space if necessary
        inline void EnsureRemain(size_t len, bool nocopy = false)
        {
        	if (len > Remain())
        		Grow((len + Size()) * 3 / 2, nocopy);
        }

        inline void Reserve(size_t len, bool nocopy = false)
        {
        	if (len > m_capacity)
        		Grow(len, nocopy);
        }

        inline void ReserveRemain(size_t len, bool nocopy = false)
        {
        	if (len > Remain())
        		Grow(len + Size(), nocopy);
        }

        // Borrow some space to store something outside
        inline char* Borrow(size_t len, bool nocopy = false)
        {
        	EnsureRemain(len, nocopy);

        	return m_ptr;
        }

        inline bool Advance(size_t len)
        {
        	if (len > Remain())
        		return false;
        	m_ptr += len;
        	return true;
        }

        inline size_t Append(const char* fmt, ...)
        {
        	EnsureRemain(1024);
            va_list ap_list;

            size_t n, size = Remain();

            do
            {
            	va_start(ap_list, fmt);
                n = vsnprintf(m_ptr, size, fmt, ap_list);
                va_end(ap_list);

                if ((n >= 0) && n < size)
                {
                    // Fit the buffer exactly
                    break;
                }

                if (n < 0)
                {
                    // Double the size of buffer
                	EnsureRemain(size * 2);
                }
                else
                {
                    // Need result+1 exactly
                    EnsureRemain(n + 1);
                }

                size = Remain();
            }
            while (true);

            m_ptr += n;
            return n;
        }

        inline size_t AppendBuf(const char *ptr, size_t len)
        {
        	EnsureRemain(len);
            memcpy(m_ptr, ptr, len);
            m_ptr += len;
            return len;
        }

    private:
        // noncoyable
        DynamicBuffer(DynamicBuffer &other);
        DynamicBuffer &operator=(DynamicBuffer &other);

        // Can only be used in constructor
        inline void Construct(char *data, size_t len)
    	{
    		m_capacity = len;
    		m_own = true;
    		m_base_p = new char[len];

    		memcpy(m_base_p, data, len);
    		m_ptr = m_base_p + len;
    	}

        inline void Grow(size_t new_capacity, bool nocopy = false)
        {
        	if (m_capacity >= new_capacity) return;
        	if (new_capacity < SMALL_BUFF_SIZE) new_capacity = SMALL_BUFF_SIZE;

        	char *new_data = new char[new_capacity];
        	if (!nocopy)
        		memcpy(new_data, m_base_p, m_ptr - m_base_p);

        	m_ptr = new_data + (m_ptr - m_base_p);

        	m_capacity = new_capacity;

			if (m_own && m_base_p)
				delete [] m_base_p;

        	m_base_p = new_data;
        	m_own = true;
        }
    };
}

#endif /* DYNAMIC_BUFFER_H_ */
