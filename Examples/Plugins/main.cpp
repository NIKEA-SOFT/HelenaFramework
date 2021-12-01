#include <Helena/Systems/PluginManager.hpp>

class GameApplication
{
public:
	GameApplication(std::uint32_t val) : value{val} {}
	~GameApplication() = default;

	std::uint32_t value{};
};

HELENA_PLUGIN_API void PluginInit(std::shared_ptr<Helena::Engine::Context> ctx) {
	Helena::Engine::Context::Initialize(ctx);

	Helena::Engine::RegisterSystem<GameApplication>(100'000);
}

HELENA_PLUGIN_API void PluginEnd() {
	Helena::Engine::RemoveSystem<GameApplication>();
}