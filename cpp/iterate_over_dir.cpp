// compile with -lboost_filesystem -lboost_system
#include <iostream>
#include <fstream>
#include <iostream>
#include <boost/filesystem/operations.hpp>

using namespace std;

int main()
{
    using namespace boost::filesystem;
    recursive_directory_iterator it = recursive_directory_iterator("/path/of/dir/");
    recursive_directory_iterator end;
    while (it != end) {
        if (boost::filesystem::is_regular_file(*it)) {
            cout << *it << endl;
        }
        ++it;
    }

    return 0;
}
