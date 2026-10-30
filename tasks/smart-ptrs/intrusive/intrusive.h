#pragma once

#include <cstddef> // for std::nullptr_t
#include <utility> // for std::exchange / std::swap


class SimpleCounter {
public:
    size_t IncRef(){
        count_ += 1;
        return count_;
    };
    size_t DecRef(){
        count_ -= 1;
        return count_;
    };
    size_t RefCount() const{
        return count_;
    };

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef(){
        size_t ans = counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef(){
        counter_.DecRef();
        if (counter_.RefCount() == 0){
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    };

    // Get current counter value (the number of strong references).
    size_t RefCount() const{
        return counter_.RefCount();
    };

    RefCounted& operator=(const RefCounted&) noexcept {
        return *this;
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr(): ptr_(nullptr){
    };
    IntrusivePtr(std::nullptr_t);
    IntrusivePtr(T* ptr) : ptr_(ptr){
        ptr_->IncRef();
    };

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) : ptr_(other.ptr_){
        if (ptr_ != nullptr){
            ptr_->IncRef();
        }
    };

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) noexcept : ptr_(other.ptr_){
        other.ptr_ = nullptr;
    };

    IntrusivePtr(const IntrusivePtr& other): ptr_(other.ptr_){
        if (ptr_ != nullptr){
            ptr_->IncRef();
        }
    };
    IntrusivePtr(IntrusivePtr&& other) noexcept : ptr_(other.ptr_){
        other.ptr_ = nullptr;
    };

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other){
        if (this->ptr_ == other.ptr_){
            return *this;
        }
        if (ptr_ != nullptr){
            ptr_->DecRef();
        }
        if (other.ptr_ != nullptr){
            other.ptr_->IncRef();
        }
        ptr_ = other.ptr_;
        return *this;
    };
    IntrusivePtr& operator=(IntrusivePtr&& other){
        if (this == &other) {
            return *this;
        }
        auto* tmp = ptr_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        if (tmp != nullptr){
            tmp->DecRef();
        }
        return *this;
    };
    template <typename Y>
    IntrusivePtr& operator=(const IntrusivePtr<Y>& other){
        if constexpr (std::is_same_v<T, Y>) {
        if (this == &other) {
            return *this;
            }
        }
        if (ptr_ != nullptr){
            ptr_->DecRef();
        }
        if (other.ptr_ != nullptr){
            other.ptr_->IncRef();
        }
        ptr_ = other.ptr_;
        return *this;
    };
    template <typename Y>
    IntrusivePtr& operator=(IntrusivePtr<Y>&& other){
        if constexpr (std::is_same_v<T, Y>) {
        if (this == &other) {
            return *this;
            }
        }
        auto* tmp = ptr_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        if (tmp != nullptr){
            tmp->DecRef();
        }
        return *this;
    };

    // Destructor
    ~IntrusivePtr() {
    if (ptr_ != nullptr) {
        ptr_->DecRef();
        }
    }

    // Modifiers
    void Reset(){
        if (ptr_ != nullptr) {
        ptr_->DecRef();
        ptr_ = nullptr;
        }
    };
    void Reset(T* ptr){
        T* tmp = ptr_;
        ptr_ = ptr;
        if (ptr != nullptr){
            ptr->IncRef();
        }
        if (tmp != nullptr){
            tmp->DecRef();
        }
    };
    void Swap(IntrusivePtr& other){
        T* tmp = ptr_;
        ptr_ = other.ptr_;
        other.ptr_ = tmp;
    };

    // Observers
    T* Get() const{
        return ptr_;
    };
    std::add_lvalue_reference_t<T> operator*() const{
        return *ptr_;
    };
    T* operator->() const{
        return ptr_;
    };
    size_t UseCount() const{
        if (ptr_ != nullptr){
            return ptr_->RefCount();
        }
        return 0;
    };
    explicit operator bool() const{
        return (ptr_ != nullptr);
    };
    private:
        T* ptr_;
};

template <typename T, typename ...Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(
        new T(std::forward<Args>(args)...)
    );
}
