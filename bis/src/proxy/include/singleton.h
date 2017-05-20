//
// Created by czllo on 2017/4/26.
// 这个模板不适合多线程，似乎用boost的singleton比较妥当

#ifndef SERVER_SINGLETON_H
#define SERVER_SINGLETON_H

#include <stdio.h>
#include <stdlib.h>

template <class TYPE>
class Singleton
{
public:
    static TYPE * instance(void)
    {
        if(m_singleton == NULL)
        {
            m_singleton = new Singleton;
        }

        return &(m_singleton->m_instance);
    }

protected:
    Singleton();

protected:
    TYPE m_instance;
    static Singleton<TYPE> * m_singleton;
};

template<class TYPE>
Singleton<TYPE> * Singleton<TYPE>::m_singleton = NULL;

template<class TYPE>
Singleton<TYPE>::Singleton()
{
}

#endif //SERVER_SINGLETON_H
