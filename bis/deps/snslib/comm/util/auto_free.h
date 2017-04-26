#ifndef _AUTO_FREE_H_
#define _AUTO_FREE_H_

namespace snslib
{
class AutoFree
{   
        public:
                AutoFree(void *p, void *expected = NULL) : m_p(p), m_expected(expected) { };
                ~AutoFree() { if(m_p != m_expected) free(m_p); }
        private:
                void *m_p;
                void *m_expected;
};  
}

#endif
