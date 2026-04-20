#pragma once

//#define LP_PROFILER_ENABLE

#ifdef LP_PROFILER_ENABLE
	#include <tracy/Tracy.hpp>
#endif

#ifdef LP_PROFILER_ENABLE
	#define LP_SCOPE() ZoneScoped
	#define LP_SCOPE_N(name) ZoneScopedN(name)
	#define LP_FUNC() ZoneScoped
#else
	#define LP_SCOPE()
	#define LP_SCOPE_N(name)
	#define LP_FUNC()
#endif

#ifdef LP_PROFILER_ENABLE
	#define LP_FRAME() FrameMark
#else
	#define LP_FRAME()
#endif