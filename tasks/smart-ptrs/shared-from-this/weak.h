#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {

    template <typename Y>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr(): ptr_(nullptr), controlBlock_(nullptr) {};

    WeakPtr(const WeakPtr& other): ptr_(other.ptr_), controlBlock_(other.controlBlock_){
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefWeak();
        }
    };
    WeakPtr(WeakPtr&& other) noexcept: ptr_(other.ptr_), controlBlock_(other.controlBlock_){
        other.ptr_ = nullptr;
        other.controlBlock_ = nullptr;
    };

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other): ptr_(other.ptr_), controlBlock_(other.controlBlock_){
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefWeak();
        }
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other){
        if (this->ptr_ == other.ptr_){
            return *this;
        }
        if (controlBlock_ != nullptr){
            controlBlock_->DecRefWeak();
        }
        if (other.controlBlock_ != nullptr){
            other.controlBlock_->IncRefWeak();
        }
        ptr_ = other.ptr_;
        controlBlock_ = other.controlBlock_;
        return *this;
    };
    WeakPtr& operator=(WeakPtr&& other){
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
            tmpBlock->DecRefWeak();
        }
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr(){
        if (controlBlock_ != nullptr){
            controlBlock_->DecRefWeak();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset(){
        if (controlBlock_ != nullptr) {
            controlBlock_->DecRefWeak();
            ptr_ = nullptr;
            controlBlock_ = nullptr;
        }
    };
    void Swap(WeakPtr& other){
        T* tmp = ptr_;
        ptr_ = other.ptr_;
        other.ptr_ = tmp;

        auto* tmpBlock = controlBlock_;
        controlBlock_ = other.controlBlock_;
        other.controlBlock_ = tmpBlock;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const{
       if (controlBlock_ != nullptr){
            return controlBlock_->StrongCount();
        }
        return 0;
    };
    bool Expired() const{
        if (controlBlock_ != nullptr){
            return ((controlBlock_->StrongCount() == 0) ? 1 : 0);
        }
        return 1;
    };
    SharedPtr<T> Lock() const {
    if (Expired()) {
        return SharedPtr<T>();
    }
        return SharedPtr<T>(*this);
    }
    private:
        T* ptr_;
        details::BaseControlBlock* controlBlock_;
};
