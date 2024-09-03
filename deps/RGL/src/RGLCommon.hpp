#pragma once
#include <RGL/Core.hpp>

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#define _UWP 1   
#else
#define _UWP 0
#endif

#define RGL_STATIC(a) decltype(a) a

namespace RGL {
	extern API currentAPI;

	void LogMessage(MessageSeverity, const std::string&);
	void FatalError(const std::string&);
	void FatalError(const std::wstring&);

	template<typename T>
	static void Assert(bool cond, const T& errormsg) {
		if (!cond) {
			FatalError(errormsg);
		}
	}

#if defined __APPLE__ || __STDC_VERSION__ >= 199901L    //check for C99
#define stackarray(name, type, size) type name[size]    //prefer C VLA on supported systems
#else
#define stackarray(name, type, size) type* name = (type*)alloca(sizeof(type) * size) //warning: alloca may not be supported in the future
#endif
}
