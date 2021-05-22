#pragma once
#include "SpinLock.hpp"
#include <fmt/format.h>
#include <chrono>
#include "PhysXDefines.h"
#include <backward.hpp>

namespace RavEngine {

class Debug{
private:
	Debug() = delete;
	static SpinLock mtx;
	
	/**
	 @return  the current time as a string
	 */
	inline static const char* DateString(){
		time_t _tm = time(NULL);
		struct tm * curtime = localtime ( &_tm );
		char* str = asctime(curtime);
		str[strlen(str)-1] = '\0';	//remove newline
		return str;
	}
	
	static inline void LogHelper(FILE* output, const char* message, const char* type){
		mtx.lock();
		fprintf(output,"[%s] %s - %s\n",DateString(),type,message);
		mtx.unlock();
	}
	
public:
	
	/**
	 Log a message to standard output. In release builds, this method is stubbed and logs are disabled.
	 @param message The message to log
	 */
	static inline void LogTemp(const char* message){
#ifdef _DEBUG
		LogHelper(stdout, message, "LOGTEMP");
#endif
	}
	
	/**
	 Log a message to standard output. In release builds, this method is stubbed and logs are disabled.
	 @param formatstr The formatting string to log
	 @param values the optional values to log
	 */
	template <typename ... T>
	static inline void LogTemp(const char* formatstr, T... values){
#ifdef _DEBUG
		LogHelper(stdout,fmt::format(formatstr,values...).c_str(),"LOGTEMP");
#endif
	}
	
	/**
	 Log a message to standard output.
	 @param message The message to log
	 */
	static inline void Log(const char* message){
		LogHelper(stdout, message, "LOG");
	}
	
	/**
	 Log a message to standard output.
	 @param formatstr The formatting string to log
	 @param values the optional values to log
	 */
	template <typename ... T>
	static inline void Log(const char* formatstr, T... values){
		LogHelper(stdout,fmt::format(formatstr,values...).c_str(),"LOG");
	}
	
	/**
	 Log a message to standard error, as a warning.
	 @param message The message to log.
	 */
	static inline void Warning(const char* message){
		LogHelper(stderr, message, "WARN");
	}
	
	/**
	 Log a message to standard error, as a warning.
	 @param formatstr The formatting string to log
	 @param values the optional values to log
	 */
	template <typename ... T>
	static inline void Warning(const char* formatstr, T... values){
		LogHelper(stderr, fmt::format(formatstr,values...).c_str(), "WARN");
	}
	
	static inline void PrintStacktraceHere(){
		backward::StackTrace st;
		st.load_here(32);
		backward::Printer p;
		p.object = true;
		p.address = true;
		p.print(st,stderr);
	}
	
	/**
	 Log a message to standard error, as an error.
	 @param message The message to log.
	 */
	static inline void Error(const char* message){
		LogHelper(stderr, message, "ERROR");
		PrintStacktraceHere();
	}
	
	/**
	 Log a message to standard error, as an error.
	@param formatstr The formatting string to log
	@param values the optional values to log
	*/
	template <typename ... T>
	static inline void Error(const char* formatstr, T... values){
		LogHelper(stderr, fmt::format(formatstr,values...).c_str(), "ERROR");
		PrintStacktraceHere();
	}
	
	/**
	 Log an error message and terminate.
	 @param message The message to log.
	 */
	static inline void Fatal(const char* message){
		Debug::Error(message);
		throw std::runtime_error(message);
	}
	
	/**
	 Log an error message and terminate.
	 @param formatstr The formatting string
	 @param values the optional values
	 */
	template <typename ... T>
	static inline void Fatal(const char* formatstr, T... values){
		Debug::Error(formatstr,values...);
		throw std::runtime_error(fmt::format(formatstr,values...));
	}
    
    /**
     Require a condition to be true
     @param condition the boolean to check
     @param formatstr the formatting string for the failure message
     @param values the optional values for the failure message
     */
    template <typename ... T>
    static inline void Assert(bool condition, const char* formatstr, T... values){
        if (!condition){
            Fatal(formatstr,values...);
        }
    }
    
    /**
     Require a condition to be true
     @param condition the boolean to check
     @param msg the string for the failure message
     */
    static inline void Assert(bool condition, const char* msg){
        if (!condition){
            Fatal(msg);
        }
    }
};
	
}
