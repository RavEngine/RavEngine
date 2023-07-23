#pragma once

#include <filesystem>
namespace RavEngine{
namespace Filesystem{
    using Path = std::filesystem::path;

    inline Path CurrentWorkingDirectory(){
        return std::filesystem::current_path();
    }
}
}
