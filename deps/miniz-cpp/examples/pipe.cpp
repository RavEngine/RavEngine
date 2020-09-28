#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

// Platform-specific includes
#ifdef _WIN32
#include <fcntl.h> // For O_BINARY
#include <io.h> // For _setmode, _fileno, _isatty
#else // POSIX
#include <unistd.h> // For fileno, isatty
#endif

#include <zip_file.hpp>

namespace {

// Returns true if program is being run directly
// If this is false, we assume that input will be given through stdin
bool is_tty()
{
#ifdef _WIN32
    return _isatty(_fileno(stdin)) != 0;
#else
    return isatty(fileno(stdin)) != 0;
#endif
}

} // namespace

// Either 1. Print directory structure of zip file given as filename or piped through stdin
// or 2. Print contents of file contained in aforementioned zip file
int main(int argc, char *argv[])
{
    miniz_cpp::zip_file file;
    std::vector<std::string> args;
    
    for(int i = 1; i < argc; i++)
    {
        args.push_back(argv[i]);
    }
    
    if(is_tty()) // Expect first argument to be zip file path
    {
        if(args.size() < 1)
        {
            std::cout << "usage: " << argv[0] << " zip_file [file_to_print]" << std::endl;
            std::cout << "    (zip_file can be replaced by data piped from standard input)" << std::endl;
            return 1;
        }

        file.load(args.front());
        args.erase(args.begin());
    }
    else
    {
#ifdef _WIN32
        // Ensure that we are reading the raw binary data when using a pipe in Windows.
        _setmode(_fileno(stdin), O_BINARY);
#endif
        file.load(std::cin);
    }
    
    if(args.empty())
    {
        file.printdir();
    }
    else
    {
        std::cout << file.read(args.front()) << std::endl;
    }
    
    return 0;
}
