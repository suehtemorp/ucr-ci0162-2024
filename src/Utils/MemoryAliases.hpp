#ifndef UTILS_HPP
#define UTILS_HPP

#include <memory>

namespace Memory {
    template <class T> 
    using atomic_ptr = std::atomic<std::shared_ptr<T>>;
}

#endif