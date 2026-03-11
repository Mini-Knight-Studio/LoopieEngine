#pragma once


#include <unordered_map>
#include <functional>

struct _MonoType;	

namespace Loopie {

	class ScriptGlue
	{
	public:
		static void RegisterComponents();
		static void RegisterFunctions();

	};
}