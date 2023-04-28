#include "ShaderTranspiler.hpp"
#include <spirv_msl.hpp>
#import <Foundation/Foundation.h>
#include <TargetConditionals.h>

using namespace shadert;

extern IMResult SPIRVtoMSL(const spirvbytes& bin, const Options& opt, spv::ExecutionModel model);

struct executeProcessResult{
	int code = 0;
	NSData *outdata, *errdata;
};
executeProcessResult executeProcess(NSString* launchPath, NSArray<NSString*>* arguments, NSData* stdinData){
#if !TARGET_OS_IPHONE
	// create task and pipes
	NSTask* task = [NSTask new];
	NSPipe* taskStdin = [NSPipe new];
	NSPipe* taskStdout = [NSPipe new];
	NSPipe* taskStderr = [NSPipe new];
	
	// connect pipes
	[task setStandardInput:taskStdin];
	[task setStandardOutput:taskStdout];
	[task setStandardError:taskStderr];
	task.currentDirectoryPath = [[NSFileManager defaultManager] currentDirectoryPath];
	
	[task setLaunchPath:launchPath];
	[task setArguments:arguments];
	
	// execute the command
	[task launch];
	// provide it with its data
	[[taskStdin fileHandleForWriting] writeData:stdinData];
	[[taskStdin fileHandleForWriting] closeFile];	// this tells xcrun that it can start reading from the pipe
	[task waitUntilExit];
	
	NSData* outdata = [[taskStdout fileHandleForReading] readDataToEndOfFile];
	NSData* errdata = [[taskStderr fileHandleForReading] readDataToEndOfFile];
		
	[[taskStdout fileHandleForReading] closeFile];
	[[taskStderr fileHandleForReading] closeFile];
	
	int code = [task terminationStatus];
	
	// return exit code and the stdout/stderr data
	return {
		code,
		outdata, errdata
	};
#else
    throw std::runtime_error("executeProcess is not available");
#endif
}

IMResult SPIRVtoMBL(const spirvbytes& bin, const Options& opt, spv::ExecutionModel model){
#if !TARGET_OS_IPHONE
	// first make metal source shader
	auto MSLResult = SPIRVtoMSL(bin, opt, model);
	
	// create the AIR
	// the "-" argument tells it to read from stdin
	//auto airResult = executeProcess(@"/usr/bin/xcrun", @[@"-sdk",@"macosx",@"metallib",@"vs.air",@"-o",@"vs.metallib"], nil);
	auto airResult = executeProcess(@"/usr/bin/xcrun",@[@"-sdk",@"macosx",@"metal",@"-c",@"-x",@"metal",@"-",@"-o",@"/dev/stdout"],[NSData dataWithBytes:MSLResult.sourceData.data() length:MSLResult.sourceData.size()]);
	
	if (airResult.code == 0)
	{
		// now need to do it again to make the metallib from the AIR
		
		// need to create a temporary file because metallib does not properly work with the standard output stream when launched by NSTask
		NSString* fileName = [[NSTemporaryDirectory() stringByAppendingPathComponent:[[NSUUID UUID] UUIDString]] stringByAppendingPathExtension:@"mtllib"];
		auto mtllibResult = executeProcess(@"/usr/bin/xcrun", @[@"-sdk",@"macosx",@"metallib",@"-",@"-o",fileName], airResult.outdata);
		if (mtllibResult.code == 0){
			using binary_t = decltype(MSLResult.binaryData)::value_type;
			NSData* data = [NSData dataWithContentsOfFile:fileName];
			
			// this time the output is the final metallib
			MSLResult.binaryData.assign((binary_t*)data.bytes, (binary_t*)data.bytes + data.length);
			
			// delete the temporary
			[NSFileManager.defaultManager removeItemAtPath:fileName error:nil];
			return MSLResult;
		}
		else{
			[NSFileManager.defaultManager removeItemAtPath:fileName error:nil];
			throw std::runtime_error((const char*)mtllibResult.errdata.bytes);
		}
		
	}
	else{
		throw std::runtime_error((const char*)airResult.errdata.bytes);
	}
#else
    throw std::runtime_error("SPIRVtoMBL is not available");
#endif
}
