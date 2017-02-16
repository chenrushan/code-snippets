#include <boost/filesystem.hpp>

void test_boost_filesystem() {
    // check if a file exists
    if (!boost::filesystem::exists("boost_filesystem.cpp")) {
        std::cout << "file not exists\n";
    } else {
        std::cout << "file exists\n";
    }

    // remove a file
    boost::filesystem::remove("xx");

    // get file size
    std::cout << boost::filesystem::file_size("boost_filesystem.cpp") << "\n";

    // get filename
    boost::filesystem::path p("/home/crs/kgengine.tar.gz");
    std::cout << "path" << p.string() << "\n";
    std::cout << "filename " << p.filename().string() << "\n";
    std::cout << "filename without extension " << p.stem().string() << "\n";
    std::cout << p.parent_path().string() << "\n";

    // remove extension
    boost::filesystem::path p2 = p.replace_extension("");
    std::cout << p2.string() << "\n";
}
