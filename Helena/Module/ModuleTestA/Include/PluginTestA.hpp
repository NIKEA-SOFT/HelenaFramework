#ifndef MODULETESTA_PLUGINTESTA_HPP
#define MODULETESTA_PLUGINTESTA_HPP

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

#endif // MODULETESTA_PLUGINTESTA_HPP