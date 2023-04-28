// include the library
#include <ShaderTranspiler/ShaderTranspiler.hpp>
#include <iostream>

using namespace std;

//the library's namespace
using namespace shadert;

int main(){
	
	//create an instance
	ShaderTranspiler s;
	
	//Create a CompileTask with the path to your shader and its stage.
	//The path is required because this library supports the OpenGL #include extension
	MemoryCompileTask task{ R"(#version 430
			layout(early_fragment_tests) in;
			layout(location = 0) out vec4 color;
			void main(){
				color = vec4(1,0,0,1);
			}
		)",ShaderStage::Fragment};
	
	//configure the compile with an Options object
	Options opt;
	opt.mobile = false; //used for OpenGL ES or Metal iOS
	opt.version = 23;   //stores the major and minor version, for Vulkan 1.5 use 15
	
	try{
		//call CompileTo and pass the CompileTask and the Options
		CompileResult result = s.CompileTo(task, TargetAPI::Metal, opt);
		
		//the shader data is stored in the data field
		cout << (result.isBinary? "Binary" : "Plain text") << "shader created, source = " << endl;
		cout << result.data << endl;
	}
	catch(exception& e){
		//library will throw on errors
		cerr << e.what() << endl;
		return 1;
	}
	
	return 0;
}
