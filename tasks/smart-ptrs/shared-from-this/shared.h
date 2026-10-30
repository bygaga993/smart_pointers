#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

template <typename T>
class SharedPtr {
    template <typename Y>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;
public:
    SharedPtr(): ptr_(nullptr), controlBlock_(nullptr) {};

    SharedPtr(std::nullptr_t): ptr_(nullptr), controlBlock_(nullptr) {};
    explicit SharedPtr(T* ptr): ptr_(ptr), controlBlock_(new details::ControlBlock<T>(ptr)) {
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefStrong();
        }
        if constexpr (std::is_convertible_v<T*, EnableSharedFromThis<T>*>) {
            if (ptr_ != nullptr && ptr_->weak_this_.Expired()) {
                ptr_->weak_this_ = WeakPtr<T>(*this);
            }
        }
    };
    template <typename Y>
    explicit SharedPtr(Y* ptr): ptr_(ptr), controlBlock_(new details::ControlBlock<Y>(ptr)) {
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefStrong();
        }
    };

    explicit SharedPtr(details::NewControlBlock<T>* controlBlock): ptr_(controlBlock ? controlBlock->Get() : nullptr),
        controlBlock_(controlBlock){
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefStrong();
        }
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), controlBlock_(other.controlBlock_){
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefStrong();
        }
    };

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) noexcept : ptr_(other.ptr_), controlBlock_(other.controlBlock_){
        other.ptr_ = nullptr;
        other.controlBlock_ = nullptr;
    };

    SharedPtr(const SharedPtr& other): ptr_(other.ptr_), controlBlock_(other.controlBlock_){
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefStrong();
        }
    };

    SharedPtr(SharedPtr&& other) noexcept: ptr_(other.ptr_), controlBlock_(other.controlBlock_){
        other.ptr_ = nullptr;
        other.controlBlock_ = nullptr;
    };

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr){
        controlBlock_ = other.controlBlock_;
        ptr_ = ptr;
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefStrong();
        }
    };

    // // Promote `WeakPtr`
    // // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other): ptr_(other.ptr_), controlBlock_(other.controlBlock_){
        if (other.Expired()){
            throw BadWeakPtr{};
        }
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefStrong();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other){
        if (this->ptr_ == other.ptr_){
            return *this;
        }
        if (controlBlock_ != nullptr){
            controlBlock_->DecRefStrong();
        }
        if (other.controlBlock_ != nullptr){
            other.controlBlock_->IncRefStrong();
        }
        ptr_ = other.ptr_;
        controlBlock_ = other.controlBlock_;
        return *this;
    };
    SharedPtr& operator=(SharedPtr&& other){
        if (this == &other) {
            return *this;
        }
        auto* tmpPtr = ptr_;
        auto* tmpBlock = controlBlock_;
        ptr_ = other.ptr_;
        controlBlock_ = other.controlBlock_;
        other.ptr_ = nullptr;
        other.controlBlock_ = nullptr;
        if (tmpBlock != nullptr){
            tmpBlock->DecRefStrong();
        }
        return *this;
    };


    ~SharedPtr(){
        if (controlBlock_ != nullptr){
            controlBlock_->DecRefStrong();
        }
    };


    void Reset(){
        if (controlBlock_ != nullptr) {
            controlBlock_->DecRefStrong();
            ptr_ = nullptr;
            controlBlock_ = nullptr;
        }
    };

    template<typename Y>
    void Reset(Y* ptr){
        auto* tmpBlock = controlBlock_;
        ptr_ = ptr;
        controlBlock_ = new details::ControlBlock<Y>(ptr);
        if (tmpBlock != nullptr){
            tmpBlock->DecRefStrong();
        }
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefStrong();
        }
    };

    void Swap(SharedPtr& other){
        T* tmp = ptr_;
        ptr_ = other.ptr_;
        other.ptr_ = tmp;

        auto* tmpBlock = controlBlock_;
        controlBlock_ = other.controlBlock_;
        other.controlBlock_ = tmpBlock;
    };


    T* Get() const{
        return ptr_;
    };

    T& operator*() const{
        return *ptr_;
    };
    T* operator->() const{
        return ptr_;
    };
    size_t UseCount() const{
        if (controlBlock_ != nullptr){
            return controlBlock_->StrongCount();
        }
        return 0;
    };
    explicit operator bool() const{
        return (ptr_ != nullptr);
    };

    private:
        T* ptr_;
        details::BaseControlBlock* controlBlock_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right){
    return (left.Get() == right.Get());
};

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args){
    return SharedPtr<T>(new
        details::NewControlBlock<T>(std::forward<Args>(args)...)
    );
};

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
    template <typename Y>
    friend class SharedPtr;
public:
    SharedPtr<T> SharedFromThis(){
        return SharedPtr<T>(weak_this_);
    };
    SharedPtr<const T> SharedFromThis() const{
        return SharedPtr<const T>(weak_this_);
    };

    WeakPtr<T> WeakFromThis() noexcept{
        return weak_this_;
    };
    WeakPtr<const T> WeakFromThis() const noexcept;
    private:
        WeakPtr<T> weak_this_;
};
