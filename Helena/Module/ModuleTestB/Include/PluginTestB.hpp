#ifndef MODULE_MODULETESTB_PLUGINTESTB_HPP
#define MODULE_MODULETESTB_PLUGINTESTB_HPP

#include <Interface/IPluginTestB.hpp>

namespace Helena
{
	class PluginTestB : public IPluginTestB
	{
	protected:
		bool Initialize() override;
		bool Config() override;
		bool Execute() override;
		bool Update() override;
		bool Finalize() override;

	public:
		PluginTestB() = default;
		~PluginTestB() = default;

	public:
		void SayHello() const override;

	private:

	};
}

#endif // MODULE_MODULETESTB_PLUGINTESTB_HPP