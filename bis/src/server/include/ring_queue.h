/**
 * @file:   ring_queue.h
 * @author: jameyli
 * @date:   2013-09-11
 * @brief:  ring queue 固定大小的环形队列
 *
 * 一个小功能有类似的需求
 * 先简单实现接口，再考虑更好的封装
 * TODO:
 *      1 Alloc?
 *      2 Iter
 */

#ifndef RING_QUEUE_H
#define RING_QUEUE_H

namespace tnt
{

/**
 * @brief: 固定大小的环形队列
 *
 * @tparam T 数据类型
 * @tparam Size 队列大小
 */
template<typename T, size_t Size>
class ring_queue
{
private:
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;

public:
    ring_queue()
    {
        front_ = 0;
        size_ = 0;
        max_size_ = Size + 1;
        rear_ = max_size_ - 1;
    }

    reference front()
    { return data_[front_]; }

    const_reference front() const
    { return data_[front_]; }

    reference back()
    { return data_[rear_]; }

    const_reference back() const
    { return data_[rear_]; }

    void push(const value_type& value)
    {
        if (!full())
        {
            rear_ = (rear_ + 1) % max_size_;
            data_[rear_] = value;
            ++size_;
        }
    }

    void pop()
    {
        if (!empty())
        {
            front_ = (front_ + 1) % max_size_;
            --size_;
        }
    }

    bool empty() const
    {
        return size_ == 0;
    }

    bool full() const
    {
        return size_ == max_size_ - 1;
    }

    size_t size() const
    {
        return size_;
    }

    size_t capacity() const
    {
        return max_size_ - 1;
    }

    reference operator[](size_t idx)
    {
        return data_[(front_ + idx)%max_size_];
    }

    const_reference operator[](size_t idx) const
    {
        return data_[(front_ + idx)%max_size_];
    }

private:
    T data_[Size];
    size_t front_;
    size_t rear_;
    size_t size_;
    size_t max_size_;

}; // class ringque


} // namespace tntlib

#endif //RING_QUEUE_H

