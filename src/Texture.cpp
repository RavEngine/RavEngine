#include "Texture.hpp"
#include <bimg/bimg.h>
#include <bx/bx.h>
#include <bx/readerwriter.h>
#include "App.hpp"

using namespace std;
using namespace RavEngine;


Texture::Texture(const std::string& name){
	//read from resource
	
	auto data = App::Resources->FileContentsAt("textures/" + name);
	
	char bytes[] = {1,2,3,4};
	
	texture = bgfx::createTexture(bgfx::copy(bytes, sizeof(bytes)));
	
	if(!bgfx::isValid(texture)){
		throw runtime_error("Cannot create texture");
	}
}

void Texture::Bind(int id, const Uniform &uniform){
	bgfx::setTexture(id, uniform, texture);
}
