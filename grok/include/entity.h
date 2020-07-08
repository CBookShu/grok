#pragma once
#pragma push_macro("new")
#undef new
#include <iostream>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <atomic>
#include <map>
#include <unordered_map>
#include <array>
#include <memory>
#include <typeindex>
#include <cassert>

namespace grok
{
    class BaseHolder
    {
    public:
        virtual ~BaseHolder() {}
        virtual void* get() = 0;
        virtual void clear() = 0;
        virtual void destroy() = 0;

        virtual void mask() = 0;
        virtual bool test() = 0;
        virtual void unmask() = 0;

        enum class HolderType
        {
            NONE,
            CLASS,
            POINTER,
            SHARED_PTR
        };
        virtual bool isNone() const
        {
            return ht_ == HolderType::NONE;
        }
        virtual bool isClass() const
        {
            return ht_ == HolderType::CLASS;
        }

        virtual bool isPointer() const
        {
            return ht_ == HolderType::POINTER;
        }

        virtual bool isSharedPtr() const
        {
            return ht_ == HolderType::SHARED_PTR;
        }

        HolderType ht_ = HolderType::NONE;
    };

    template <typename T>
    class BasicHolder : public BaseHolder
    {
    public:
        enum { chunk_size = sizeof(T) + 1 };
        typedef std::array<unsigned char, chunk_size>   Chunk;

        BasicHolder()
        {
            clear();
            this->ht_ = HolderType::CLASS;
        }

        virtual ~BasicHolder()
        {
            if (test())
            {
                destroy();
            }
        }

        virtual void destroy() override
        {
            T* t = (T*)get();
            t->~T();
            clear();
        }

        virtual void* get() override
        {
            return &chunk_[0];
        }

        virtual void clear() override
        {
            chunk_.fill(0);
        }

        virtual void mask() override
        {
            *chunk_.rbegin() = 1;
        }

        virtual bool test() override
        {
            return *chunk_.rbegin() == 1;
        }

        virtual void unmask() override
        {
            *chunk_.rbegin() = 0;
        }

    protected:
        Chunk   chunk_;
    };

    template <typename T>
    class PtrHolder : public BasicHolder<T*>
    {
    public:
        PtrHolder()
        {
            this->ht_ = BaseHolder::HolderType::POINTER;
        }
        virtual void destroy() override
        {
            BasicHolder<T*>::clear();
        }
    };

    template <typename T>
    class SharePtrHolder : public BasicHolder<std::shared_ptr<T>>
    {
    public:
        typedef std::shared_ptr<T>              TPtr;
        typedef BasicHolder<std::shared_ptr<T>> Base;
        SharePtrHolder()
        {
            this->ht_ = BaseHolder::HolderType::SHARED_PTR;
        }
    };

    class Entity
    {
    public:
        Entity() = default;
        ~Entity();

        Entity(const Entity&) = delete;
        Entity& operator = (const Entity&) = delete;

        template <typename C, typename... Args>
        C* assign(Args&& ... args)
        {
            auto* holder = accomodate_component<C>();
            if (holder->test())
            {
                holder->destroy();
            }
            C* c = new (holder->get()) C(std::forward<Args>(args)...);
            holder->mask();

            return c;
        }

        template <typename C, class = typename std::enable_if<std::is_pointer<C>::value, C>>
        C assign(C h)
        {
            auto* holder = accomodate_component<C>();
            holder->destroy();
            memcpy(holder->get(), &h, sizeof(void*));
            holder->mask();
            return h;
        }

        template <typename C>
        std::shared_ptr<C> assign(std::shared_ptr<C> ptr)
        {
            auto* holder = accomodate_component_share_ptr<C>();
            if (holder->test())
            {
                holder->destroy();
            }
            new (holder->get()) std::shared_ptr<C>(ptr);
            holder->mask();
            return ptr;
        }

        template <typename C, typename...Args>
        std::shared_ptr<C> share_assign(Args&& ...args)
        {
            auto* holder = accomodate_component_share_ptr<C>();
            if (holder->test())
            {
                holder->destroy();
            }
            std::shared_ptr<C>* sptr = new (holder->get()) std::shared_ptr<C>(new C(std::forward<Args>(args)...));
            holder->mask();
            return *sptr;
        }

        template <typename C>
        void remove()
        {
            auto index = std::type_index(typeid(C));
            auto it = type_holders_.find(index);
            if (it != type_holders_.end() && it->second->test())
            {
                it->second->destroy();
            }
        }

        template <typename C>
        bool has_component() const
        {
            auto index = std::type_index(typeid(C));
            auto it = type_holders_.find(index);
            if (it == type_holders_.end())
            {
                return false;
            }
            return it->second->test();
        }

        template < typename C, class = typename std::enable_if < !std::is_pointer<C>::value, C >::type >
        C* component()
        {
            auto index = std::type_index(typeid(C));
            auto it = type_holders_.find(index);
            if (it == type_holders_.end())
            {
                return nullptr;
            }
            if (!it->second->test())
            {
                return nullptr;
            }
            assert(it->second->isClass() || it->second->isSharedPtr());
            return static_cast<C*>(it->second->get());
        }

        template <typename C, class = typename std::enable_if<std::is_pointer<C>::value, C>::type>
        C component()
        {
            auto index = std::type_index(typeid(C));
            auto it = type_holders_.find(index);
            if (it == type_holders_.end())
            {
                return nullptr;
            }
            if (!it->second->test())
            {
                return nullptr;
            }
            assert(it->second->isPointer());
            C h;
            memcpy(&h, it->second->get(), sizeof(void*));
            return h;
        }

        template <typename C>
        std::shared_ptr<C> share_component()
        {
            auto index = std::type_index(typeid(std::shared_ptr<C>));
            auto it = type_holders_.find(index);
            if (it == type_holders_.end())
            {
                return nullptr;
            }
            if (!it->second->test())
            {
                return nullptr;
            }
            assert(it->second->isSharedPtr());
            std::shared_ptr<C>* sptr = (std::shared_ptr<C>*)it->second->get();
            return *sptr;
        }

        void destroy();

        static void Init();
        static Entity& GetEntity();
        static void Uinit();

    protected:

        template <typename T>
        BaseHolder* accomodate_component()
        {
            auto index = std::type_index(typeid(T));
            auto it = type_holders_.find(index);
            BaseHolder* holder = nullptr;
            if (it == type_holders_.end())
            {
                typedef typename std::remove_pointer<T>::type TrueType;
                typedef typename std::conditional<std::is_pointer<T>::value, PtrHolder<TrueType>, BasicHolder<TrueType>>::type HolderType;
                holder = new HolderType();
                type_holders_[index] = holder;
            }
            else
            {
                holder = it->second;
            }
            return holder;
        }

        template<typename T>
        BaseHolder* accomodate_component_share_ptr()
        {
            auto index = std::type_index(typeid(std::shared_ptr<T>));
            auto it = type_holders_.find(index);
            BaseHolder* holder = nullptr;
            if (it == type_holders_.end())
            {
                holder = new SharePtrHolder<T>();
                type_holders_[index] = holder;
            }
            else
            {
                holder = it->second;
            }
            return holder;
        }

    private:
        std::map<std::type_index, BaseHolder*>              type_holders_;

        static Entity instance;;
    };
};





#pragma pop_macro("new")