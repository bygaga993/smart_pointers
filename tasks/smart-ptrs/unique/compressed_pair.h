#pragma once

#include <type_traits>
#include <memory>
#include <iostream>

template <typename F, typename S, typename Enable = void>
class CompressedPair {
public:
    CompressedPair() {
        first_ = F{};
        second_ = S{};
    };

    template <typename U, typename V>
    CompressedPair(U&& first, V&& second)
        : first_(std::forward<U>(first)), second_(std::forward<V>(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    F first_;
    S second_;
};

template <typename F, typename S>
struct CompressedPair<F, S,
                      std::enable_if_t<std::is_empty<F>::value && !std::is_empty<S>::value &&
                                       !std::is_final<F>::value>> : private F {
public:
    CompressedPair(const F& first, const S& second) : second_(second){};

    F GetFirst() const {
        return F{};
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    }

private:
    S second_;
};

template <typename F, typename S>
struct CompressedPair<F, S,
                      std::enable_if_t<std::is_empty<S>::value && !std::is_empty<F>::value &&
                                       !std::is_final<S>::value>> : private S {
public:
    CompressedPair(const F& first, const S& second) : first_(first){};

    S& GetSecond() {
        return static_cast<S&>(*this);
    }

    const S& GetSecond() const {
        return static_cast<const S&>(*this);
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

private:
    F first_;
};

template <typename F, typename S>
struct CompressedPair<F, S,
                      std::enable_if_t<std::is_empty<S>::value && std::is_empty<F>::value &&
                                       !std::is_final<S>::value && !std::is_final<F>::value &&
                                       !std::is_same<F, S>::value>> : private F,
                                                                      private S {
public:
    CompressedPair(const F& first, const S& second){};

    S GetSecond() const {
        return S{};
    }

    F GetFirst() const {
        return F{};
    }
};
