#ifndef MODULE_MODULETESTB_IPLUGINTESTB_HPP
#define MODULE_MODULETESTB_IPLUGINTESTB_HPP

#include <Common/IPlugin.hpp>

namespace Helena
{
	// Use an abstract class of your plugin to make some methods available to third party modules
	class IPluginTestB : public IPlugin
	{
	protected:
		virtual bool Initialize() = 0;
		virtual bool Config() = 0;
		virtual bool Execute() = 0;
		virtual bool Update() = 0;
		virtual bool Finalize() = 0;

	public:
		virtual void SayHello() const = 0;
	};
}

#endif // MODULE_MODULETESTB_IPLUGINTESTB_HPP