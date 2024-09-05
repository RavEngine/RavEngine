
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <filesystem>
#include <simdjson.h>
#include <iostream>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include "rve_mesh.hpp"

using namespace std;
using namespace RavEngine;

int main(int argc, char** argv){
    cxxopts::Options options("rvemc", "RavEngine Mesh Compiler");
    options.add_options()
        ("f,file", "Input file path", cxxopts::value<std::filesystem::path>())
        ("o,output", "Ouptut file path", cxxopts::value<std::filesystem::path>())
        ("h,help", "Show help menu")
        ;
    
    auto args = options.parse(argc, argv);

    if (args["help"].as<bool>()) {
        cout << options.help() << endl;
        return 0;
    }
    
    
}


