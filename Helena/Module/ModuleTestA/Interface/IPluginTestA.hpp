#ifndef MODULE_MODULETESTA_IPLUGINTESTA_HPP
#define MODULE_MODULETESTA_IPLUGINTESTA_HPP

#include <Common/IPlugin.hpp>

namespace Helena
{
	// Use an abstract class of your plugin to make some methods available to third party modules
	class IPluginTestA : public IPlugin
	{
	protected:
		virtual bool Initialize() = 0;
		virtual bool Config() = 0;
		virtual bool Execute() = 0;
		virtual bool Update() = 0;
		virtual bool Finalize() = 0;

	public:

	};
}

#endif // MODULE_MODULETESTA_IPLUGINTESTA_HPP