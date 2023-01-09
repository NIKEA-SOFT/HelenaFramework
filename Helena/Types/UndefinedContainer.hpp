#ifndef HELENA_TYPES_UNDEFINEDCONTAINER_HPP
#define HELENA_TYPES_UNDEFINEDCONTAINER_HPP

#include <Helena/Platform/Assert.hpp>
#include <Helena/Types/Hash.hpp>

#include <memory>

namespace Helena::Types
{
    class UndefinedContainer final
    {
        using hash_type = Hash<std::uint64_t>;
        using hash_value = typename hash_type::value_type;

        template <typename T>
        static constexpr auto HashOf = hash_type::template From<std::remove_const_t<T>>();

    public:
        template <typename T>
        struct DefinedValue {};

        template <typename T>
        static constexpr auto ValueOf = DefinedValue<T>{};

    private:
        struct IUnknownValue {
            virtual ~IUnknownValue() = default;
            [[nodiscard]] virtual void* Get() = 0;
            [[nodiscard]] virtual bool Has() const noexcept = 0;
            [[nodiscard]] virtual bool Equal(hash_value hash) const noexcept = 0;
        };

        template <typename T>
        struct UnknownValue : IUnknownValue
        {
            constexpr UnknownValue() noexcept : m_Value{}, m_Hash{HashOf<T>} {}

            template <typename... Args>
            requires std::constructible_from<T, Args...>
            constexpr explicit UnknownValue(Args&&... args)
                noexcept(std::is_nothrow_constructible_v<T, Args...>)
                : m_Value(std::forward<Args>(args)...), m_Hash{HashOf<T>} {}

            [[nodiscard]] void* Get() override {
                HELENA_ASSERT(m_Hash, "Value not exist");
                return static_cast<void*>(std::addressof(m_Value));
            }

            [[nodiscard]] bool Has() const noexcept override {
                return m_Hash;
            }

            [[nodiscard]] bool Equal(hash_value hash) const noexcept override {
                return m_Hash && m_Hash == hash;
            }

            std::remove_const_t<T> m_Value;
            hash_value m_Hash;
        };

    public:
        UndefinedContainer() : m_Object{} {}

        template <typename T, typename... Args>
        requires std::constructible_from<T, Args...>
        UndefinedContainer(DefinedValue<T>, Args&&... args)
            : m_Object{std::make_unique<UnknownValue<T>>(std::forward<Args>(args)...)} {}

        template <typename T, typename... Args>
        requires std::constructible_from<T, Args...>
        void Create(Args&&... args) {
            m_Object = std::make_unique<UnknownValue<T>>(std::forward<Args>(args)...);
        }

        template <typename T>
        [[nodiscard]] T& Get() noexcept {
            HELENA_ASSERT(Has(), "Container is empty");
            HELENA_ASSERT(m_Object->Equal(HashOf<T>), "Type mismatch");
            return *static_cast<T*>(m_Object->Get());
        }

        template <typename T>
        [[nodiscard]] const T& Get() const noexcept {
            HELENA_ASSERT(Has(), "Container is empty");
            HELENA_ASSERT(m_Object->Equal(HashOf<T>), "Type mismatch");
            return *static_cast<const T*>(m_Object->Get());
        }

        template <typename T>
        [[nodiscard]] T* GetPtr() noexcept {
            const auto has = static_cast<bool>(m_Object) && m_Object->Equal(HashOf<T>);
            return has ? static_cast<T*>(m_Object->Get()) : nullptr;
        }

        template <typename T>
        [[nodiscard]] const T* GetPtr() const noexcept {
            const auto has = static_cast<bool>(m_Object) && m_Object->Equal(HashOf<T>);
            return has ? static_cast<const T*>(m_Object->Get()) : nullptr;
        }

        [[nodiscard]] bool Has() const noexcept {
            return static_cast<bool>(m_Object);
        }

        template <typename T>
        [[nodiscard]] bool Equal() const noexcept {
            HELENA_ASSERT(Has(), "Container is empty");
            return m_Object->Equal(HashOf<T>);
        }

        void Reset() {
            m_Object.reset();
        }

        [[nodiscard]] operator bool() const noexcept {
            return Has();
        }

    private:
        std::unique_ptr<IUnknownValue> m_Object;
    };
}

#endif // HELENA_TYPES_UNDEFINEDCONTAINER_HPP
