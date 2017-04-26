/*
 * buffer_sequence.h
 *
 *  Created on: 2010-12-16
 *      Author: jiffychen
 */

#ifndef BUFFER_SEQUENCE_H_
#define BUFFER_SEQUENCE_H_

#include <sys/uio.h>
#include <stdexcept>
#include "static_buffer.h"

namespace snslib
{
    // Stores M static buffers inside.
    // Just hate to use vector here (only 2/3 buffers expected).
    // After doing a benchmark(set M = 3):
    // 1. BufferSequence performs over shared_ptr<BufferSequence<> > in containers(using std deque)
    //    (10% faster, 185w/s vs 166w/s).
    //    can se say that the struct is memory tight enough?
    // 2. BufferSequence performs over std string (50% faster, 217w/s vs 100w/s).
    //    if buffer don't required to be copied(i.e.: hold in tbus in our most cases):
    //    BufferSequence performs really excellent(1111w/s).
    // 3. vsnprintf seems to be the bottleneck(111w/s for 6 int and 1 string).
    template <int M>
    struct BufferSequence
    {
    	BufferSequence() : m_count(0), m_total_size(0)
    	{
    	}

    	BufferSequence(const BufferSequence &other) :
    		m_count(other.m_count), m_total_size(other.m_total_size)
    	{
    		for (size_t i = 0; i < m_count; i++)
    			m_buffers[i] = other.m_buffers[i];
    	}

    	BufferSequence &operator+(StaticBuffer &other)
        {
    		if (m_count >= M)
    			throw std::runtime_error("buffer overflow");

        	m_buffers[m_count++] = other;

        	m_total_size += other.Remain();

        	return *this;
        }

    	BufferSequence &operator+=(StaticBuffer &other)
        {
    		if (m_count >= M)
    			throw std::runtime_error("buffer overflow");

    		//std::cout << "assign buffer " << m_count << std::endl;

        	m_buffers[m_count] = other;
        	// other became invalid

        	m_total_size += m_buffers[m_count].Remain();

        	m_count++;

        	return *this;
        }

    	BufferSequence &operator+=(DynamicBuffer &other)
        {
    		if (m_count >= M)
    			throw std::runtime_error("buffer overflow");

    		//std::cout << "assign buffer " << m_count << std::endl;

    		StaticBuffer sb(other);
        	m_buffers[m_count] = sb;
        	// other became invalid

        	m_total_size += m_buffers[m_count].Remain();

        	m_count++;

        	return *this;
        }

    	BufferSequence &operator=(BufferSequence &other)
        {
    		m_count = other.m_count;
    		m_total_size = other.m_total_size;
    		for (size_t i = 0; i < m_count; i++)
    			m_buffers[i] = other.m_buffers[i];
        	return *this;
        }

        template <int N>
        inline int InitIoVec(struct iovec (&vec) [N]) const
        {
        	int count = 0;
        	for (size_t i = 0; i < m_count && count < N; i++)
        	{
        		if (!m_buffers[i].Empty())
        		{
        			vec[count].iov_base = const_cast<char*>(m_buffers[i].Str());
        			vec[count].iov_len = m_buffers[i].Remain();
        			count++;
        		}
        	}

        	return count;
        }

        inline bool Advance(size_t len)
        {
        	int to_advance = 0;
        	for (size_t i = 0; i < m_count && len > 0; i++)
        	{
        		if (!m_buffers[i].Empty())
        		{
        			//std::cout << "before advance, remain=" << m_buffers[i].Remain() << std::endl;
        			to_advance = std::min(m_buffers[i].Remain(), len);

        			//std::cout << "to advance, to=" << to_advance << std::endl;

        			m_buffers[i].Advance(to_advance);
        			len -= to_advance;
        			m_total_size -= to_advance;

        			//std::cout << "after advance, remain=" << m_buffers[i].Remain() << std::endl;
        		}
        	}

        	return true;
        }

        inline void Solidify()
        {
        	if (m_total_size == 0)
        		return;

        	for (size_t i = 0; i < m_count; i++)
        	{
        		if (!m_buffers[i].Empty() && !m_buffers[i].Solided())
        		{
        			//std::cout << "soliding buffer " << ", base=" << (void*)m_buffers[i].m_base_p << std::endl;
        			// original buffer might be temporal, make a copy
        			DynamicBuffer new_buffer(const_cast<char*>(
        					m_buffers[i].Str()), m_buffers[i].Remain());
        			m_buffers[i] = new_buffer;
        		}
        	}
        }

        inline unsigned short CheckSum()
        {
        	if (m_total_size == 0) return 0;

            unsigned short sum = 0;
            const unsigned char *data;
            size_t k = 0;
            bool last_buffer_odd = false;
            short last_buffer_char = 0;
            size_t buffer_len = 0;
            for (size_t i = 0; i < m_count; i++)
            {
            	if (!m_buffers[i].Empty())
            	{
            		data = (unsigned char *)m_buffers[i].Str();
            		buffer_len = m_buffers[i].Remain();

            		if (last_buffer_odd)
            		{
            			sum ^= data[0] * 256 + last_buffer_char;
            			//std::cout << "step result " << CStrTool::Format("%02x, %02x ", data[0], last_buffer_char) << ((short)(last_buffer_char * 256) + data[0]) << ", " << sum << std::endl;
            			buffer_len--;
            			data++;
            		}

					for (k = 0; k < buffer_len / 2; ++k)
					{
						sum ^= *(short *)((char *)data + k * 2);
						//std::cout << "step result " << CStrTool::Format("%02x, %02x ", data[0 + k *2], data[1 + k*2]) << *(short *)((char *)data + k * 2) << ", " << sum << std::endl;
					}

					if (buffer_len % 2)
					{
						last_buffer_odd = true;
						last_buffer_char = data[buffer_len - 1];
					}
					else
					{
						last_buffer_odd = false;
					}
            	}
            }

            return sum;
        }

        inline bool Empty() const { return m_total_size == 0; }
        inline size_t Remain() const { return m_total_size; }

        ~BufferSequence()
        {
        	//std::cout << "~buffer sequence" << std::endl;
        }

        StaticBuffer m_buffers[M];
        size_t m_count;
        size_t m_total_size;
    };
}

#endif /* BUFFER_SEQUENCE_H_ */
