#pragma once

#include <Common/Common.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

template<typename T>
class atomic_list
{
public:
    class iterator;
    class const_iterator;
    class node;
    typedef node node_type;

    atomic_list() : _next(nullptr)
    {
    }

    atomic_list(const atomic_list<T>& base) : _next(nullptr)
    {
        set_next(const_cast<atomic_list<T>&>(base).next());
    }

    bool empty() const
    {
        return nullptr == next();
    }

    node* next()
    {
        return _next.load();
    }

    const node* next() const
    {
        return _next.load();
    }

    bool try_insert_next(node* next_value)
    {
        auto prev = next();
        if (next_value)
        {
            next_value->set_next(prev);
        }
        return _next.compare_exchange_weak(prev, next_value);
    }

    atomic_list<T>& insert_next(node* next_value)
    {
        while (!try_insert_next(next_value)) {}
        return *this;
    }

    bool try_set_next(node* next_value)
    {
        auto prev = next();
        return _next.compare_exchange_weak(prev, next_value);
    }

    atomic_list<T>& set_next(node* next_value)
    {
        while (!try_set_next(next_value)) {}
        return *this;
    }

    bool remove_next(node** out = nullptr)
    {
        node* skipper;
        node* keeper;
        do
        {
            skipper = next();
            keeper = nullptr;
            if (skipper)
            {
                keeper = skipper->next();
            }
        } while (!try_set_next(keeper) && keeper);

        if (out)
        {
            *out = skipper;
        }
        return nullptr != skipper;
    }

    atomic_list<T>& append(node* next_value)
    { 
        auto last = this;
        auto end = last->next();
        while (nullptr != end)
        {
            last = end;
            end = last->next();
        }

        return last->insert_next(next_value);
    }

    iterator begin()
    {
        return iterator(*this);
    }
    
    iterator end()
    {
        return iterator();
    }
    
    const_iterator begin() const
    {
        return const_iterator(*this);
    }
    
    const_iterator end() const
    {
        return const_iterator();
    }
     
protected:
    node* last()
    {
        T* last = nullptr;
        T* next = next();
        do
        {
            last = next;
            next = last->next();
        } while (next);
        return last;
    }

    const node* last() const
    {
        return const_cast<atomic_list<T>*>(this)->last();
    }

    atomic<node*> _next;
};


// --------------------------------------------------------------------------------------------------------------------
template<typename T>
class atomic_list<T>::node : public atomic_list<T>
{
public:
    node() : atomic_list<T>()
    {}

    node(T data) : atomic_list<T>(), _data(data)
    {}

    node(const node& data) : atomic_list<T>(data), _data(data._data)
    {}

    T& get() 
    {
        return _data;
    }
    const T& get() const 
    {
        return _data;
    }

protected:
    T _data;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
class atomic_list<T>::iterator : public atomic_list<T>
{
public:
    iterator() {}
    iterator(atomic_list<T>& focus) : atomic_list<T>(focus) {}
    iterator(const iterator& rhs) : atomic_list<T>(rhs) {}

    T& operator*()
    {
        return next()->get();
    }

    const T& operator*() const
    {
        return *next()->get();
    }

    T* operator->()
    {
        return &next()->get();
    }

    const T* operator->() const
    {
        return &next()->get();
    }

    iterator operator++(int)
    {
        iterator now(*this);
        operator++();
        return now;
    }

    iterator& operator++()
    {
        node* now;
        do {
            now = next();
        } while (now && !try_set_next(now->next()));
        return *this;
    }

    iterator& operator+=(int i)
    {
        while (i--)
        {
            operator++();
        }
        return *this;
    }

    iterator operator+(int i) const
    {
        return iterator(*this) += i;
    }

    bool operator==(const iterator& rhs) const
    {
        return next() == rhs.next(); 
    }

    bool operator!=(const iterator& rhs) const
    {
        return next() != rhs.next();
    }
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
class atomic_list<T>::const_iterator : public atomic_list<T>
{
public:
    const_iterator() 
    {}

    const_iterator(const atomic_list<T>& focus) : atomic_list<T>(focus)
    {}

    const_iterator(const const_iterator& rhs) : atomic_list<T>(rhs)
    {}

    const T& operator*() const
    {
        return next()->get();
    }

    const T* operator->() const
    {
        return &next()->get();
    }

    const_iterator operator++(int)
    {
        iterator now(*this);
        operator++();
        return now;
    }

    const_iterator& operator++()
    {
        node* now;
        do
        {
            now = next();
        } while (now && !try_set_next(now));
        return *this;
    }

    const_iterator& operator+=(int i)
    {
        while (i--)
        {
            operator++(i);
        }
        return *this;
    }

    const_iterator operator+(int i) const
    {
        return iterator(*this) += i;
    }

    bool operator==(const const_iterator& rhs) const
    {
        return next() == rhs.next();
    }

    bool operator!=(const const_iterator& rhs) const
    {
        return next() != rhs.next();
    }
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
