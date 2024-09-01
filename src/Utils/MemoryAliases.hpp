#pragma once

#include <memory>

namespace Memory {
    template <class T> 
    using atomic_ptr = std::atomic<std::shared_ptr<T>>;

    template <class T, void F(T*)>
    struct PointerDeleter {
        public:
            void operator()(T* target) const {
                if (target != nullptr) {
                    F(target);
                }
            }
    };

    template <class T, void F(T*)>
    using unique_ptr_with_deleter = std::unique_ptr<T, PointerDeleter<T, F>>;
}
