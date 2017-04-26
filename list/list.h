//
// Created by czllo on 2017/4/3.
//

#ifndef LIST_LIST_H
#define LIST_LIST_H

#include <functional>
#include <memory>

namespace cz {

template <typename Data>
struct Node {
    Data val;

    struct Node *pre;
    struct Node *next;
};

template <typename Data, typename EqualKey = std::equal_to<Data>, typename Alloc = std::allocator<Data>>
class List {
public:
    typedef Node<Data> node ;

    List() : nil(NULL) { }
    ~List() { }

private:
    node *create() {
        node *pNew = new node;
        if (NULL != pNew) {
            pNew->pre = NULL;
            pNew->next = NULL;
        }
        return pNew;
    };

public:
    int push(const Data &v) {
        if (NULL == nil) {
            if (NULL == (nil = create() ))
                return -1;
            nil->next = nil;
            nil->pre = nil;
        }

        node *stNew = create();
        if (!stNew) {
            return -2;
        }
        stNew->val = v;

        if (NULL != nil) {
            stNew->next = nil->next;
            stNew->pre = nil;
            nil->next->pre = stNew;
            nil->next = stNew;
        }

        return 0;
    }

    int del(const Data &v) {
        node *findNode =  search(v);

        if (NULL == findNode)
            return -1;

        findNode->pre->next = findNode->next;
        findNode->next->pre = findNode->pre;

        delete findNode;
        findNode = NULL;
    }

    node *search(const Data key) {
        node *curr = nil->next;
        while (nil != curr) {
            if (compare(curr->val, key))
                return curr;
            curr = curr->next;
        }

        return NULL;
    }

    bool compare(const Data &a, const Data &b) {
        return EqualKey()(a, b);
    }

    class iter;

    iter begin() {
        return iter(nil->next);
    }

    iter end() {
        return iter(nil);
    }

    class iter {
    public:
        typedef node* pointer;

        iter() : p(NULL) { }
        iter(pointer p) {
            this->p = p;
        }
        iter(const iter &b) {
            p = b.p;
        }

        ~iter() { }

        iter &operator ++() {
            if (NULL != p) {
                p = p->next;
            }
            return *this;
        }

        iter operator ++(int) {
            iter cRet = *this;
            ++(*this);
            return cRet;
        }

        node &operator *() {
            //if (NULL == p)
            //抛出一个异常？
            //return ;
            return *p;
        }

        node *operator ->() {
            return p;
        }

        bool operator ==(const iter &b) {
            return (p == b.p);
        }

        bool operator !=(const iter &b) {
            return !(operator==(b));
        }

        iter operator=(const iter &b) {
            p = b.p;
            return *this;
        }

    private:
        pointer p;
    };
private:
    node *nil;
};

}



#endif //LIST_LIST_H
