#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t


namespace details{

    struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
        }
    };

    class BaseControlBlock{
        private:
            int strongCount_ = 0;
            int weakCount_ = 0;
         public:
            void IncRefStrong() {
                strongCount_ += 1;
            }
            void IncRefWeak() {
                weakCount_ += 1;
            }

            void DecRefStrong() {
                strongCount_ -= 1;

                if (strongCount_ == 0) {
                    DestroyObject();

                    if (weakCount_ == 0) {
                        delete this;
                    }
                }
            }
            void DecRefWeak() {
                weakCount_ -= 1;

            if (weakCount_ == 0 && strongCount_ == 0) {
                delete this;
            }
        }

        size_t StrongCount() const {
            return strongCount_;
        }

        size_t WeakCount() const {
            return strongCount_;
        }

        virtual void DestroyObject() = 0;
        virtual ~BaseControlBlock() = default;

    };

    template <class T, class Deleter=DefaultDelete>
    class ControlBlock: public BaseControlBlock{
        public:
        ControlBlock(T* ptr): ptr_(ptr) {};
        void DestroyObject() override {
            Deleter::Destroy(ptr_);
        }
        T* Get() const{
            return ptr_;
        }
        private:
            T* ptr_;
    };

    template<class T>
    class NewControlBlock: public BaseControlBlock{
        private:
            alignas(T) std::byte value_[sizeof(T)];
            T* ptr_;
        public:
            template<typename... Args>
            explicit NewControlBlock(Args&&... args) {
                void* memory = static_cast<void*>(value_);
                ptr_ = ::new (memory) T(std::forward<Args>(args)...);
            }

            T* Get() const {
                return ptr_;
            }

            void DestroyObject() override {
                std::destroy_at(ptr_);
            }
    };
};

template <typename T>
class SharedPtr {
    template <typename Y>
    friend class SharedPtr;
public:
    SharedPtr(): ptr_(nullptr), controlBlock_(nullptr) {};

    SharedPtr(std::nullptr_t): ptr_(nullptr), controlBlock_(nullptr) {};
    explicit SharedPtr(T* ptr): ptr_(ptr), controlBlock_(new details::ControlBlock<T>(ptr)) {
        if (controlBlock_ != nullptr){
            controlBlock_->IncRefStrong();
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
    explicit SharedPtr(const WeakPtr<T>& other);

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
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

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
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
