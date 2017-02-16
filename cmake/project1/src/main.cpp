#include "lib1/boost_filesystem.h"
#include "lib2/boost_options.h"

int main(int argc, char *argv[]) {
    auto opts = ProgramOption::parse_args(argc, argv);
    std::cout << opts.to_string() << std::endl;

    test_boost_filesystem();
    return 0;
}
