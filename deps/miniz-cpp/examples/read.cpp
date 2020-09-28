#include <iostream>
#include <zip_file.hpp>

int main(int argc, char *argv[])
{
    if(argc <= 1)
    {
        std::cout << "usage: " << argv[0] << " filename" << std::endl;
        return 0;
    }
    
    miniz_cpp::zip_file file(argv[1]);
    file.printdir();
    
    return 0;
}
