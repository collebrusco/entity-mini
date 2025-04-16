/** 
 * c_abstract.h
 * created 08/03/24 by frank collebrusco 
 */
#ifndef C_ABSTRACT_H
#define C_ABSTRACT_H
#include <stddef.h>
#include <type_traits>

template <class Base, size_t max_size>
struct c_abstract {
static_assert(sizeof(Base) <= max_size);
static_assert(IS_POW2(max_size));

    inline c_abstract() {memset(data, 0x00, max_size);}
    virtual ~c_abstract() = default;

    c_abstract(c_abstract const& other) = delete;
    c_abstract(c_abstract && other) = delete;
    c_abstract& operator=(c_abstract const& other) = delete;
    c_abstract& operator=(c_abstract && other) = delete;

    Base& get() {
        return (*((Base*)data));
    }

    template <typename Sub>
    Sub& downcast() {
static_assert(sizeof(Sub) <= max_size);
static_assert(std::is_base_of<Base, Sub>::value);
        return (*((Sub*)((Base*)data)));
    }

    template <typename Sub, typename ...Args>
    Sub& emplace(Args... args) {
static_assert(sizeof(Sub) <= max_size);
static_assert(std::is_base_of<Base, Sub>::value);
        return *(new (data) Sub(args...));
    }

    void destroy() {
        this->get().~Base();
    }

    template <typename Sub, typename ...Args>
    Sub& replace(Args... args) {
        this->destroy();
        return this->emplace(args...);
    }

private:
    static bool IS_POW2(size_t num) {return ( !((num) & ((num)-1)) );}
    alignas(Base) char data[max_size];
};

#endif /* C_ABSTRACT_H */
