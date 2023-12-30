#pragma once
#include "SpinLock.hpp"
#include "PhysXDefines.h"
#include "Utilities.hpp"

namespace RavEngine {

class Debug{
private:
	Debug() = delete;
	static SpinLock mtx;
	constexpr static uint16_t bufsize = 512;

	static void LogHelper(FILE* output, const std::string_view, const char* type);
    
    static void InvokeUserHandler(const std::string_view msg);
	
public:
	
	/**
	 Log a message to standard output. In release builds, this method is stubbed and logs are disabled.
	 @param message The message to log
	 */
	static inline void LogTemp(const std::string_view message){
#ifndef NDEBUG
		LogHelper(stdout, message, "LOGTEMP");
#endif
	}
	
	/**
	 Log a message to standard output. In release builds, this method is stubbed and logs are disabled.
	 @param formatstr The formatting string to log
	 @param values the optional values to log
	 */
	template <typename ... T>
	static inline void LogTemp(const std::string_view formatstr, T&& ... values){
#ifndef NDEBUG
		LogHelper(stdout, std::format(formatstr,values...),"LOGTEMP");
#endif
	}
	
	/**
	 Log a message to standard output.
	 @param message The message to log
	 */
	static inline void Log(const std::string_view message){
		LogHelper(stdout, message, "LOG");
	}
	
	/**
	 Log a message to standard output.
	 @param formatstr The formatting string to log
	 @param values the optional values to log
	 */
	template <typename ... T>
	static constexpr inline void Log(const std::string_view formatstr, T&& ... values){
		LogHelper(stdout, std::vformat(formatstr, std::make_format_args(std::forward<T>(values)...)),"LOG");
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
	static inline void Warning(const std::string_view formatstr, T&& ... values){
		LogHelper(stderr, std::vformat(formatstr, std::make_format_args(std::forward<T>(values)...)), "WARN");
	}
	
	static inline void PrintStacktraceHere(){
		//TODO: get stack trace and print
	}
	
	/**
	 Log a message to standard error, as an error.
	 @param message The message to log.
	 */
	static inline void Error(const std::string_view message){
		LogHelper(stderr, message, "ERROR");
		PrintStacktraceHere();
	}
	
	/**
	 Log a message to standard error, as an error.
	@param formatstr The formatting string to log
	@param values the optional values to log
	*/
	template <typename ... T>
	static inline void Error(const std::string_view formatstr, T&& ... values){
		LogHelper(stderr, std::vformat(formatstr, std::make_format_args(std::forward<T>(values)...)), "ERROR");
		PrintStacktraceHere();
	}
	
	/**
	 Log an error message and terminate.
	 @param message The message to log.
	 */
	static inline void Fatal(const std::string_view message){
		Debug::Error(message);
        InvokeUserHandler(message);
		throw std::runtime_error(std::string(message));
	}
	
	/**
	 Log an error message and terminate.
	 @param formatstr The formatting string
	 @param values the optional values
	 */
	template <typename ... T>
	static inline void Fatal(const std::string_view formatstr, T&& ... values){
		Debug::Error(formatstr,values...);
        auto formattedMsg = std::vformat(formatstr, std::make_format_args(std::forward<T>(values)...));
        InvokeUserHandler(formattedMsg);
		throw std::runtime_error(formattedMsg);
	}
    
    /**
     Require a condition to be true
     @param condition the boolean to check
     @param formatstr the formatting string for the failure message
     @param values the optional values for the failure message
     */
    template <typename ... T>
    static inline void Assert(bool condition, const char* formatstr, T&& ... values){
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
    
    /**
     Asserts that the value is in range, then returns the value cast to the checked type
     */
    template<typename U,typename T>
    static inline U AssertSize(T val, const char* name = ""){
        Debug::Assert(val <=  std::numeric_limits<U>::max(), "Size ({}) is greater than maximum allowed ({})",val, std::numeric_limits<U>::max());
        return static_cast<U>(val);
    }
};
	
}
