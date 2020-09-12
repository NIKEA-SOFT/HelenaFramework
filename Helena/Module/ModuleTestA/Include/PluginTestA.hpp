#ifndef MODULE_MODULETESTA_PLUGINTESTA_HPP
#define MODULE_MODULETESTA_PLUGINTESTA_HPP

#include <Interface/IPluginTestA.hpp>

namespace Helena
{
	class PluginTestA : public IPluginTestA
	{
	protected:
		bool Initialize() override;
		bool Config() override;
		bool Execute() override;
		bool Update() override;
		bool Finalize() override;

	public:
		PluginTestA() = default;
		~PluginTestA() = default;

	private:
		
	};
}

#endif // MODULE_MODULETESTA_PLUGINTESTA_HPP