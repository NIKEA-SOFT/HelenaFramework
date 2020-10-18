#ifndef COMMON_MODULE_HPP
#define COMMON_MODULE_HPP


namespace Helena
{
    // State of module for EntryPoint
    enum class EModuleState : uint8_t {
        Init,
        Free
    };

    class Module final {
        using EntryPoint = void (*)(class ModuleManager*, EModuleState);

    public:
        explicit Module(const std::string_view name) 
        : m_Name(name.data())
        , m_Handle(nullptr) 
        , m_ModuleEP(nullptr) {}

        ~Module() = default;
        Module(const Module&) = delete;
        Module(Module&&) = delete;
        Module& operator=(const Module&) = delete;
        Module& operator=(Module&&) = delete;

    public:
        bool Init() {
            if(m_Handle = static_cast<HF_MODULE_HANDLE>(HF_MODULE_LOAD(m_Nam)); !m_Handle) {
                return false;
            }

            if(m_ModuleEP = reinterpret_cast<EntryPoint>(HF_MODULE_GETSYM(m_Handle, HF_MODULE_CALLBACK)); m_ModuleEP) {
                return false;
            }

            return true;
        }

        void Free() {

        }

        const std::string_view GetName() const noexcept {
            return m_Name;
        }

        EntryPoint GetEntryPoint() const {
            return m_ModuleEP;
        }

    private:
        const char* m_Name;
        HF_MODULE_HANDLE m_Handle;
        EntryPoint m_ModuleEP;
    };
}

#endif // COMMON_MODULE_HPP