// compile with -lboost_filesystem -lboost_system
#include <boost/filesystem.hpp>

int main(int argc, char *argv[])
{
    // check if a file exists
    if (!boost::filesystem::exists("boost_filesystem.cpp")) {
        std::cout << "file not exists" << std::endl;
    } else {
        std::cout << "file exists" << std::endl;
    }

    // remove a file
    boost::filesystem::remove("xx");

    // get file size
    std::cout << boost::filesystem::file_size("boost_filesystem.cpp") << std::endl;

    // get filename
    boost::filesystem::path p("/home/crs/kgengine.tar.gz");
    std::cout << "path" << p << std::endl;
    std::cout << "filename " << p.filename().string() << std::endl;
    std::cout << "filename without extension " << p.stem().string() << std::endl;
    
    return 0;
}

