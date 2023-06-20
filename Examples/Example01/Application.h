#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <Helena/Engine/Engine.hpp>

namespace Example01
{
    class Application : public Helena::Engine::Context {
    private:
		static std::string GetExecPath() noexcept
		{
			std::string path;

		#if defined(HELENA_PLATFORM_WIN)
			path.resize(MAX_PATH);
			(void)GetModuleFileNameA(nullptr, path.data(), MAX_PATH);
		#elif defined(HELENA_PLATFORM_LINUX)
			path.resize(PATH_MAX);
			(void)readlink("/proc/self/exe", path.data(), PATH_MAX);
		#endif

			auto pos = path.rfind(HELENA_SEPARATOR);
			HELENA_ASSERT(pos != std::string::npos);

			path.resize(pos == std::string::npos ? 0 : pos);
			path.shrink_to_fit();

			return path;
		}

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch(uMsg)
			{
				case WM_DESTROY: {
					Helena::Engine::Shutdown("WND Msg destroy");
					//::PostQuitMessage(EXIT_SUCCESS);
				} break;

				default: return ::DefWindowProc(hwnd, uMsg, wParam, lParam);

			}

			return NULL;
		}

	public:
		void Main() override
		{
			m_WindowClassEx.cbSize = sizeof(WNDCLASSEX);
			m_WindowClassEx.style = CS_HREDRAW | CS_VREDRAW;
			m_WindowClassEx.cbClsExtra = 0;
			m_WindowClassEx.cbWndExtra = 0;
			m_WindowClassEx.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
			m_WindowClassEx.hbrBackground = (HBRUSH)COLOR_WINDOW;
			m_WindowClassEx.hIcon = ::LoadIcon(0, IDI_APPLICATION);
			m_WindowClassEx.hIconSm = ::LoadIcon(0, IDI_APPLICATION);
			m_WindowClassEx.lpszClassName = "WND_CL";
			m_WindowClassEx.lpszMenuName = nullptr;
			m_WindowClassEx.hInstance = ::GetModuleHandle(NULL);
			m_WindowClassEx.lpfnWndProc = &WindowProc;

			Helena::Engine::SetTickrate(60.);
			Helena::Engine::SubscribeEvent<Helena::Events::Engine::Tick, &OnTick>();
			Helena::Engine::SubscribeEvent<Helena::Events::Engine::Shutdown, []() {
				auto reason = Helena::Engine::ShutdownReason();
				if(!reason.empty()) {
					reason = Helena::Util::Format("Error:\n{}", reason);
					::MessageBoxA(nullptr, reason.c_str(), "Shutdown with error!", MB_ICONERROR | MB_OK);
				}
			}>();

			if(!::RegisterClassEx(&m_WindowClassEx)) {
				Helena::Engine::Shutdown("RegisterClass window failure!");
				return;
			}

			m_WindowHWND = ::CreateWindow(m_WindowClassEx.lpszClassName, m_AppName.c_str(),
				WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, m_WindowWidth, m_WindowHeight, nullptr, nullptr, ::GetModuleHandle(NULL), nullptr);

			if(!m_WindowHWND) {
				Helena::Engine::Shutdown("CreateWindows failure!");
				return;
			}

			::ShowWindow(m_WindowHWND, SW_SHOW);
			::UpdateWindow(m_WindowHWND);
		};

		static void OnTick(Helena::Events::Engine::Tick) 
		{
			MSG msg {};
			if(::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}

		static void SetWindowSize(std::int32_t width, std::int32_t height) {
			auto& ctx = Helena::Engine::GetContext<Application>();
			ctx.m_WindowWidth = width;
			ctx.m_WindowHeight = height;
		}

		static void SetArgs(std::int32_t argc, char** argv) noexcept {
			Helena::Engine::GetContext<Application>().m_ArgsCount = argc;
		}

		[[nodiscard]] static const std::string_view GetArgs(std::uint32_t index) noexcept {
			return Helena::Engine::GetContext<Application>().m_Args[index];
		}

		[[nodiscard]] static std::uint32_t GetArgsCount() noexcept {
			return Helena::Engine::GetContext<Application>().m_ArgsCount;
		}

		[[nodiscard]] static std::string GetPath(const std::string& path) {
			return path.empty() ? GetPathExec() : GetPathExec() + HELENA_SEPARATOR + path;
		}

		[[nodiscard]] static const std::string& GetPathExec() noexcept {
			auto& ctx = Helena::Engine::GetContext<Application>();
			return !ctx.m_CurrentPath.empty() ? ctx.m_CurrentPath : ctx.m_CurrentPath = GetExecPath();
		}

		static void SetPathConfig(const std::string& path) {
			Helena::Engine::GetContext<Application>().m_ConfigPath = GetPathExec() + HELENA_SEPARATOR + path;
		}

		[[nodiscard]] static const std::string& GetPathConfig() noexcept {
			return Helena::Engine::GetContext<Application>().m_ConfigPath;
		}

		[[nodiscard]] static std::string GetPathConfig(const std::string& path) noexcept {
			const auto& ctx = Helena::Engine::GetContext<Application>();
			HELENA_ASSERT(!ctx.m_ConfigPath.empty(), "Config path is empty!");
			return ctx.m_ConfigPath + HELENA_SEPARATOR + path;
		}

		[[nodiscard]] static std::string_view GetPathConfigPretty(const std::string_view path) noexcept {
			const auto offset = path.find_first_not_of(".\\/", GetPathExec().size());
			return std::string_view{path.cbegin() + offset, path.cend()};
		}

	private:
		std::string m_AppName{"Example01"};
		WNDCLASSEX m_WindowClassEx{};
		HWND m_WindowHWND{};
		std::int32_t m_WindowWidth{960};
		std::int32_t m_WindowHeight{840};

		std::string m_CurrentPath{};
		std::string m_ConfigPath{};

		char** m_Args{};
		std::uint32_t m_ArgsCount{};
    };
}

#endif // APPLICATION_HPP