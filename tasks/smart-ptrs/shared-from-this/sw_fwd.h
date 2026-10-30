#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

template <typename T>
class EnableSharedFromThis;


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
                    IncRefWeak();      
                    DestroyObject();
                    DecRefWeak();

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
            return weakCount_;
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
