#ifndef HELENA_TYPES_UNDEFINEDCONTAINER_HPP
#define HELENA_TYPES_UNDEFINEDCONTAINER_HPP

#include <Helena/Platform/Assert.hpp>
#include <Helena/Traits/Remove.hpp>
#include <Helena/Types/Hash.hpp>

#include <algorithm>
#include <memory>
#include <utility>

namespace Helena::Types
{
    class UndefinedContainer final
    {
        enum class EOperation : std::uint8_t {
            Copy,
            Move,
            Assign,
            Transfer,
            Delete
        };

        template <typename T>
        using TypeOf = Traits::RemoveCVR<T>;
        using VTableFn = void (EOperation, UndefinedContainer*, UndefinedContainer*);

        using hash_type = Hash<std::uint64_t>;
        using hash_value = typename hash_type::value_type;

        template <typename T>
        static constexpr auto HashOf = hash_type::template From<Traits::RemoveConst<T>>();

    private:
        template <typename T>
        [[nodiscard]] static constexpr auto AlignmentOf() noexcept {
            constexpr std::size_t alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
            constexpr std::size_t alignment_type = alignof(T);
            return std::align_val_t{(std::max)(alignment, alignment_type)};
        }

        template <typename T, typename... Args>
        static constexpr void Placement(T* const ptr, Args&&... args) {
            if constexpr(std::constructible_from<T, Args...>) {
                std::construct_at(ptr, std::forward<Args>(args)...);
            } else if constexpr(requires {T{std::forward<Args>(args)...};}) {
                new (ptr) T{std::forward<Args>(args)...};
            } else []<auto Asserted = true>{
                static_assert(!Asserted, "T not constructible from args pack or narrowing cast!");
            }();
        }

        template <typename T, typename... Args>
        [[nodiscard]] static constexpr auto Construct(Args&&... args) {
            const auto memory = ::operator new(sizeof(T), AlignmentOf<T>());
            Placement(static_cast<T*>(memory), std::forward<Args>(args)...);
            return static_cast<T*>(memory);
        }

        template <typename T>
        static void VTable(EOperation op, UndefinedContainer* ptr, UndefinedContainer* other) noexcept(
            std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>)
        {
            switch(op)
            {
                case EOperation::Copy: {
                    if constexpr(std::is_copy_constructible_v<T>) {
                        ptr->m_Data = Construct<T>(*static_cast<T*>(other->m_Data));
                        ptr->m_VTable = other->m_VTable;
                        ptr->m_Hash = other->m_Hash;
                    } else {
                        HELENA_ASSERT(std::is_copy_constructible_v<T> == true, "Type: {} is not copy constructible!", Traits::NameOf<T>);
                    }
                } break;

                case EOperation::Move: {
                    ptr->m_Data = std::exchange(other->m_Data, nullptr);
                    ptr->m_VTable = std::exchange(other->m_VTable, nullptr);
                    ptr->m_Hash = std::exchange(other->m_Hash, 0);
                } break;

                case EOperation::Assign:
                {
                    if(ptr->m_Hash == other->m_Hash)
                    {
                        if constexpr(std::is_copy_assignable_v<T>) {
                            *static_cast<T*>(ptr->m_Data) = *static_cast<T*>(other->m_Data);
                        } else {
                            HELENA_ASSERT(std::is_copy_assignable_v<T> == true, "Type: {} is not copy assignable!", Traits::NameOf<T>);
                        }
                    } else {
                        if(ptr->m_VTable) {
                            ptr->m_VTable(EOperation::Delete, ptr, nullptr);
                        }

                        other->m_VTable(EOperation::Copy, ptr, other);
                    }
                } break;

                case EOperation::Transfer: {
                    if(ptr->m_VTable) {
                        ptr->m_VTable(EOperation::Delete, ptr, nullptr);
                    }

                    ptr->m_Data = std::exchange(other->m_Data, nullptr);
                    ptr->m_VTable = std::exchange(other->m_VTable, nullptr);
                    ptr->m_Hash = std::exchange(other->m_Hash, 0);
                } break;

                case EOperation::Delete: {
                    if constexpr(std::is_destructible_v<T>) {
                        static_cast<T*>(ptr->m_Data)->~T();
                    }
                    ::operator delete(ptr->m_Data, AlignmentOf<T>());
                    ptr->m_Data = nullptr;
                    ptr->m_VTable = nullptr;
                    ptr->m_Hash = 0;
                } break;
            }
        };

        void Construct(EOperation op, UndefinedContainer* other) noexcept(noexcept(other->m_VTable(op, this, other)))
        {
            if(other->m_VTable) {
                other->m_VTable(op, this, other);
            } else {
                m_Data = nullptr;
                m_VTable = nullptr;
                m_Hash = 0;
            }
        }

    public:
        UndefinedContainer() noexcept : m_Data{}, m_VTable{}, m_Hash{} {}

        template <typename T, typename... Args>
        UndefinedContainer(std::in_place_type_t<T>, Args&&... args)
            requires (std::constructible_from<TypeOf<T>, Args...> ||
            requires {TypeOf<T>{std::forward<Args>(args)...};})
            : m_Data{Construct<TypeOf<T>>(std::forward<Args>(args)...)}
            , m_VTable{VTable<TypeOf<T>>}
            , m_Hash{HashOf<TypeOf<T>>} {}

        ~UndefinedContainer() {
            Reset();
        }

        UndefinedContainer(const UndefinedContainer& other) {
            Construct(EOperation::Copy, &const_cast<UndefinedContainer&>(other));
        }

        UndefinedContainer(UndefinedContainer&& other) noexcept {
            Construct(EOperation::Move, &other);
        }

        UndefinedContainer& operator=(const UndefinedContainer& other) {
            Construct(EOperation::Assign, &const_cast<UndefinedContainer&>(other));
            return *this;
        }

        UndefinedContainer& operator=(UndefinedContainer&& other) noexcept {
            Construct(EOperation::Transfer, &other);
            return *this;
        }

        template <typename T, typename... Args>
        void Create(Args&&... args)
        {
            if(m_Hash == HashOf<TypeOf<T>>) {
                std::destroy_at(static_cast<TypeOf<T>*>(m_Data));
                Placement(static_cast<TypeOf<T>*>(m_Data), std::forward<Args>(args)...);
                return;
            }

            Reset();
            m_Data = Construct<TypeOf<T>>(std::forward<Args>(args)...);
            m_VTable = VTable<TypeOf<T>>;
            m_Hash = HashOf<TypeOf<T>>;
        }

        template <typename T>
        [[nodiscard]] T& Ref() noexcept {
            HELENA_ASSERT(Has(), "Container is empty");
            HELENA_ASSERT(Equal<T>(), "Undefined behaviour detected");
            return *static_cast<TypeOf<T>*>(m_Data);
        }

        template <typename T>
        [[nodiscard]] const T& Ref() const noexcept {
            HELENA_ASSERT(Has(), "Container is empty");
            HELENA_ASSERT(Equal<T>(), "Undefined behaviour detected");
            return *static_cast<TypeOf<T>*>(m_Data);
        }

        [[nodiscard]] void* Data() noexcept {
            return m_Data;
        }

        [[nodiscard]] const void* Data() const noexcept {
            return m_Data;
        }

        template <typename T>
        [[nodiscard]] T* Ptr() noexcept {
            return m_Hash == HashOf<TypeOf<T>> ? static_cast<TypeOf<T>*>(m_Data) : nullptr;
        }

        template <typename T>
        [[nodiscard]] const T* Ptr() const noexcept {
            return m_Hash == HashOf<TypeOf<T>> ? static_cast<const TypeOf<T>*>(m_Data) : nullptr;
        }

        [[nodiscard]] bool Has() const noexcept {
            return m_Hash;
        }

        template <typename T>
        [[nodiscard]] bool Equal() const noexcept {
            return m_Hash == HashOf<TypeOf<T>>;
        }

        void Reset() noexcept {
            if(m_VTable) {
                m_VTable(EOperation::Delete, this, nullptr);
            }
        }

        template <typename T>
        requires (!std::same_as<TypeOf<T>, UndefinedContainer>)
        UndefinedContainer& operator=(T&& value)
        {
            if(m_Hash == HashOf<TypeOf<T>>) {
                Ref<T>() = std::forward<T>(value);
            } else {
                Reset();
                m_Data = Construct<TypeOf<T>>(std::forward<T>(value));
                m_VTable = VTable<TypeOf<T>>;
                m_Hash = HashOf<TypeOf<T>>;
            }

            return *this;
        }

        [[nodiscard]] explicit operator bool() const noexcept {
            return Has();
        }

    private:
        void* m_Data;
        VTableFn* m_VTable;
        hash_value m_Hash;
    };
}

#endif // HELENA_TYPES_UNDEFINEDCONTAINER_HPP
