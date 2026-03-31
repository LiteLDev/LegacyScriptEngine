#pragma once
#include <memory>
#include <stdexcept>

namespace DB {

class Stmt;

template <typename T>
class SharedPointer : public std::shared_ptr<T> {

public:
    explicit SharedPointer(T* ptr = nullptr) : std::shared_ptr<T>(ptr) {}
    explicit SharedPointer(std::shared_ptr<T> const& ptr) : std::shared_ptr<T>(ptr) {}
    explicit SharedPointer(std::shared_ptr<T>&& ptr) : std::shared_ptr<T>(ptr) {}
    SharedPointer(SharedPointer<T> const& other) : std::shared_ptr<T>(other) {}
    SharedPointer(SharedPointer<T>&& other) noexcept : std::shared_ptr<T>(other) {}
    ~SharedPointer() = default;
    inline SharedPointer<T>& operator=(SharedPointer<T> const& other) {
        std::shared_ptr<T>::operator=(other);
        return *this;
    }
    inline SharedPointer<T>& operator=(SharedPointer<T>&& other) noexcept {
        std::shared_ptr<T>::operator=(other);
        return *this;
    }

    template <typename U>
    inline SharedPointer<Stmt> operator<<(U const& v) {
        auto ptr = std::shared_ptr<T>::get();
        if (!ptr) throw std::runtime_error("The pointer is nullptr");
        return (*ptr) << v;
    }

    template <typename U>
    inline SharedPointer<Stmt> operator>>(U& v) {
        auto ptr = std::shared_ptr<T>::get();
        if (!ptr) throw std::runtime_error("The pointer is nullptr");
        return (*ptr) >> v;
    }

    template <typename U>
    inline SharedPointer<Stmt> operator,(U v) {
        auto ptr = std::shared_ptr<T>::get();
        if (!ptr) throw std::runtime_error("The pointer is nullptr");
        return ptr->operator,(v);
    }
};

} // namespace DB
