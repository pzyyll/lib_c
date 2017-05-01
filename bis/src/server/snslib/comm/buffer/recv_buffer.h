//
// Created by czllo on 2017/4/29.
//

#ifndef SERVER_RECV_BUFFER_H
#define SERVER_RECV_BUFFER_H

#include <string>
#include <cstring>
#include "dynamic_buffer.h"

namespace snslib {

    struct RecvBuffer {
        RecvBuffer() :
                m_base_p(0), m_size(0), m_own(true), m_solid(false), m_nptr(0) {
        }

        explicit RecvBuffer(size_t len) :
                m_base_p(new char[len]),
                m_size(len),
                m_own(true),
                m_solid(false),
                m_nptr(m_base_p) {
        }

        RecvBuffer(const char *data, size_t len, bool ownership = false) :
                m_base_p(const_cast<char *>(data)),
                m_size(len),
                m_own(ownership),
                m_solid(false),
                m_nptr(m_base_p) {
        }

        RecvBuffer(const RecvBuffer &other_) {
            RecvBuffer &other = const_cast<RecvBuffer &>(other_);
            m_base_p = other.m_base_p;
            m_size = other.m_size;
            m_own = other.m_own;
            m_solid = other.m_solid;
            m_nptr = other.m_nptr;
            if (m_own) {
                other.m_own = true;
                other.m_solid = false;
                other.m_size = 0;
                other.m_base_p = 0;
                other.m_nptr = 0;
            }

            //std::cout << "static buffer " << "own=" << m_own << ", solid=" << m_solid << std::endl;
        }

        RecvBuffer(const DynamicBuffer &other_) {
            DynamicBuffer &other = const_cast<DynamicBuffer &>(other_);
            m_base_p = other.m_base_p;
            m_size = other.Size();
            m_own = other.m_own;
            m_solid = false;
            m_nptr = m_base_p;
            if (m_own) {
                other.m_own = false;
                other.Free();
            }
        }

        RecvBuffer &operator=(const RecvBuffer &other_) {
            if (m_own && m_base_p) delete[] m_base_p;

            RecvBuffer &other = const_cast<RecvBuffer &>(other_);
            m_base_p = other.m_base_p;
            m_size = other.m_size;
            m_own = other.m_own;
            m_solid = other.m_solid;
            m_nptr = other.m_nptr;
            if (m_own) {
                other.m_own = false;
                other.m_solid = false;
                other.m_size = 0;
                other.m_base_p = 0;
                other.m_nptr = 0;
            }

            //std::cout << "dst static buffer " << "own=" << m_own << ", solid=" << m_solid << ", base=" << (void*)m_base_p << std::endl;
            //std::cout << "src static buffer " << "own=" << other.m_own << ", solid=" << other.m_solid << ", base=" << (void*)other.m_base_p << std::endl;

            return *this;
        }

        RecvBuffer &operator=(const DynamicBuffer &other_) {
            DynamicBuffer &other = const_cast<DynamicBuffer &>(other_);
            m_base_p = other.m_base_p;
            m_size = other.Size();
            m_own = other.m_own;
            m_solid = false;
            m_nptr = m_base_p;
            if (m_own) {
                other.m_own = false;
                other.Free();
            }

            //std::cout << "dst static buffer " << "own=" << m_own << ", solid=" << m_solid << ", base=" << (void*)m_base_p << std::endl;
            //std::cout << "src dynamic buffer " << "own=" << other.m_own << ", base=" << (void*)other.m_base_p << std::endl;
            return *this;
        }

        RecvBuffer &Assign(const char *data, size_t len, bool ownership = false) {
            if (m_own && m_base_p) delete[] m_base_p;

            m_base_p = const_cast<char *>(data);
            m_size = len;
            m_own = ownership;
            m_solid = false;
            m_nptr = m_base_p;

            return *this;
        }

        //inline description is unnnecessary?
        inline bool Empty() const { return m_nptr == m_base_p + m_size; }

        inline size_t Remain() const { return m_size - UsedSize(); }

        inline size_t UsedSize() const { return (m_nptr - m_base_p); };

        inline void Solidit() { m_solid = true; }

        inline bool Solided() { return m_own == true || m_solid == true; };

        inline bool Advance(size_t len) {
            if (len > Remain()) {
                //std::cout << "advance fail, to_advance=" << len << ", remain=" << Remain() << std::endl;
                return false;
            }
            m_nptr += len;
            return true;
        }

        bool MemMoveLeft(size_t len) {
            if (len > UsedSize()) {
                return false;
            }
            size_t mv_size = UsedSize() - len;
            memmove(m_base_p, m_base_p + len, mv_size);
            m_nptr = m_base_p + mv_size;
            return true;
        }

        //const char *cstrStr() const { return m_nptr; }
        std::string Str() const { return std::string(m_base_p, UsedSize()); }

        ~RecvBuffer() {
            //std::cout << "~static buffer " << "own=" << m_own << ", solid=" << m_solid << ", base=" << (void*) m_base_p << std::endl;
            if (m_own && m_base_p) delete[] m_base_p;
        }

        char *m_base_p;
        size_t m_size;
        bool m_own;   //only this class is own can free memory by destructor
        bool m_solid; // solid means when own is false, we still don't need to
        // create a dynamic buffer to store the data
        char *m_nptr; // next put pos
    };

}
#endif //SERVER_RECV_BUFFER_H
