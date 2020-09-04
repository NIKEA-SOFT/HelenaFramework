#ifndef __COMMON_DYNLIB_HPP__
#define __COMMON_DYNLIB_HPP__

#include <cstdint>
#include "Platform.hpp"

namespace Helena
{
	enum class EModuleState : uint8_t 
	{
		Init,
		Free
	};

	class DynLib final 
	{
	public:
		

	private:
		HF_MODULE_HANDLE m_pHandle;
		EModuleState m_State;
		uint8_t m_Version;
	};
}

#endif // __COMMON_DYNLIB_HPP__