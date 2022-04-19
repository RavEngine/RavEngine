#pragma once

// boost::filesystem for legacy systems
#if (TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MIN_REQUIRED < 130000)
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
namespace RavEngine{
namespace Filesystem{
    using Path = boost::filesystem::path;

    inline Path CurrentWorkingDirectory(){
        return boost::filesystem::current_path();
    }
}
}
// otherwise use std::filesystem
#else
#include <filesystem>
namespace RavEngine{
namespace Filesystem{
    using Path = std::filesystem::path;

    inline Path CurrentWorkingDirectory(){
        return std::filesystem::current_path();
    }
}
}
#endif
