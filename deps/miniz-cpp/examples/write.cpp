#include <iostream>
#include <zip_file.hpp>

int main(int argc, char *argv[])
{
    if(argc <= 1)
    {
        std::cout << "usage: " << argv[0] << " filename" << std::endl;
        return 0;
    }
    
    miniz_cpp::zip_file file;
    
    file.writestr("file1.txt", "this is file 1");
    file.writestr("file2.txt", "this is file 2");
    file.writestr("file3.txt", "this is file 3");
    file.writestr("file4.txt", "this is file 4");
    file.writestr("file5.txt", "this is file 5");
    
    file.save(argv[1]);
    
    return 0;
}
