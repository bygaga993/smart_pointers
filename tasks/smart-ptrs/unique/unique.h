#pragma once

#include "compressed_pair.h"
#include "deleters.h"

#include <cstddef>

template <typename T>
struct Slug{
    Slug() = default;

    // template<typename U>
    // Slug(const Slug<U>& other){
    // }

    template<typename U>
    Slug(Slug<U>&& other){
    }
    
    void operator()(T* value) noexcept{
        delete value;
    }
};

template <typename T>
struct Slug<T[]> {
    Slug() = default;

    template<typename U>
    Slug(const Slug<U>& other){
    }

    template<typename U>
    Slug(Slug<U>&& other){
    }
    void operator()(T* value) noexcept{
        delete [] value;
    }
};

template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
private:
    CompressedPair<T*, Deleter> data_;
public:
    explicit UniquePtr(T* ptr = nullptr): data_(ptr, Deleter{}) {
    };

    UniquePtr(const UniquePtr&)=delete;
    UniquePtr& operator=(const UniquePtr&)=delete;

    UniquePtr(T* ptr, Deleter deleter): data_(ptr, std::move(deleter)){};
    UniquePtr& operator=(std::nullptr_t){
        this->Reset();
        return *this;
    };


    UniquePtr(UniquePtr<T, Deleter>&& other) noexcept : data_(other.Release(),
        std::move(other.GetDeleter())){
    };

    template<typename U, typename UDeleter=Slug<U>>
    UniquePtr(UniquePtr<U, UDeleter>&& other) noexcept :  data_(other.Release(),
        std::move(other.GetDeleter())){
    };

    UniquePtr& operator=(UniquePtr<T, Deleter>&& other) noexcept{
        this->Reset(other.Release());
        data_.GetSecond() = std::move(other.GetDeleter());
        return *this;
    };


    template<typename U, typename UDeleter=Slug<U>>
    UniquePtr& operator=(UniquePtr<U, UDeleter>&& other) noexcept{
        this->Reset(other.Release());
        data_.GetSecond() = std::move(other.GetDeleter());
        return *this;
    };

    ~UniquePtr(){
        data_.GetSecond()(data_.GetFirst());
    };


    T* Release(){
        T* tmp = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return tmp;
    };

    void Reset(T* ptr = nullptr){
        T* tmp = data_.GetFirst();
        data_.GetFirst() = ptr;
        if (tmp != nullptr){
            data_.GetSecond()(tmp);
        }
    };

    void Swap(UniquePtr& other){
        T* tmp = this->Release();
        auto tmp_deleter = std::move(this->GetDeleter());
        *this = std::move(other);
        other.data_.GetFirst() = tmp;
        other.data_.GetSecond() = std::move(tmp_deleter);
    };


    T* Get() const{
        return data_.GetFirst();
    };
    Deleter& GetDeleter(){
        return data_.GetSecond();
    };
    const Deleter& GetDeleter() const{
        return data_.GetSecond();
    };
    explicit operator bool() const{
        return data_.GetFirst() != nullptr;
    };


    std::add_lvalue_reference_t<T> operator*() const{
        return *data_.GetFirst();
    };
    T* operator->() const{
        return data_.GetFirst();
    };
};


template <typename T, typename Deleter>
class UniquePtr<T[], Deleter>{
    private:
        CompressedPair<T*, Deleter> data_;

    public:
    explicit UniquePtr(T* array = nullptr): data_(array, Deleter{}) {};
    UniquePtr(const UniquePtr&)=delete;
    UniquePtr& operator=(const UniquePtr&)=delete;

    UniquePtr(T* ptr, Deleter deleter): data_(ptr, std::move(deleter)){};
    UniquePtr& operator=(std::nullptr_t){
        this->Reset();
        return *this;
    };

    void Reset(T* ptr = nullptr){
        T* tmp = data_.GetFirst();
        data_.GetFirst() = ptr;
        if (tmp != nullptr){
            data_.GetSecond()(tmp);
        }
    };

    UniquePtr(UniquePtr<T, Deleter>&& other) noexcept : data_(other.Release(),
        std::move(other.GetDeleter())){
    };

    template<typename U, typename UDeleter=Slug<U>>
    UniquePtr(UniquePtr<U, UDeleter>&& other) noexcept :  data_(other.Release(),
        std::move(other.GetDeleter())){
    };

    UniquePtr& operator=(UniquePtr<T, Deleter>&& other) noexcept{
        this->Reset(other.Release());
        data_.GetSecond() = std::move(other.GetDeleter());
        return *this;
    };
    template<typename U, typename UDeleter=Slug<U>>
    UniquePtr& operator=(UniquePtr<U, UDeleter>&& other) noexcept{
        this->Reset(other.Release());
        data_.GetSecond() = std::move(other.GetDeleter());
        return *this;
    };

     T* Release(){
        T* tmp = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return tmp;
    };

    void Swap(UniquePtr& other){
        T* tmp = this->Release();
        auto tmp_deleter = std::move(this->GetDeleter());
        *this = std::move(other);
        other.data_.GetFirst() = tmp;
        other.data_.GetSecond() = std::move(tmp_deleter);
    };
    T* Get() const{
        return data_.GetFirst();
    };
    Deleter& GetDeleter(){
        return data_.GetSecond();
    };
    const Deleter& GetDeleter() const{
        return data_.GetSecond();
    };
    explicit operator bool() const{
        return data_.GetFirst() != nullptr;
    };


    T& operator[](size_t index){
        return data_.GetFirst()[index];
    }
    ~UniquePtr(){
        data_.GetSecond()(data_.GetFirst());
    };
};
