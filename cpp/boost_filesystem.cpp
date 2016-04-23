// compile with -lboost_filesystem -lboost_system
#include <boost/filesystem.hpp>

using namespace std;

int main(int argc, char *argv[])
{
    // check if a file exists
    if (!boost::filesystem::exists("boost_filesystem.cpp")) {
        cout << "file not exists" << endl;
    } else {
        cout << "file exists" << endl;
    }

    // remove a file
    boost::filesystem::remove("xx");

    // get file size
    cout << boost::filesystem::file_size("boost_filesystem.cpp") << endl;

    // get filename
    boost::filesystem::path p("/home/crs/kgengine.tar.gz");
    cout << "path" << p.string() << endl;
    cout << "filename " << p.filename().string() << endl;
    cout << "filename without extension " << p.stem().string() << endl;
    cout << p.parent_path().string() << endl;

    // remove extension
    boost::filesystem::path p2 = p.replace_extension("");
    cout << p2.string() << endl;
    
    return 0;
}

