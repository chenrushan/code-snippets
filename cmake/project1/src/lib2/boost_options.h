// compile with g++ -o boost_options boost_options.cpp -lboost_system
// -lboost_program_options -std=c++11
#include <boost/program_options.hpp>
#include <string>

#include "lib1/boost_filesystem.h"

class ProgramOption {
public:
    // {{{ XXX: customize. option values
    std::string str;
    int i;
    std::vector<std::string> vec;
    bool flag;
    // }}}

    std::string to_string() {
        // {{{ XXX: customize
        std::ostringstream os;
        os << "==================================================\n"
           << "string option: " << str << "\n"
           << "integer option: " << i << "\n"
           << "bool option: " << flag << "\n"
           << "vector option:";
        for (const auto &s : vec) {
            os << " || " << s;
        }
        os << "\n";
        os << "==================================================\n";
        // }}}
        test_boost_filesystem();
        return os.str();
    }

    static ProgramOption parse_args(int argc, char **argv) {
        ProgramOption opt;
        auto vm = opt.to_vm(argc, argv);
        return opt;
    }

private:
    boost::program_options::variables_map to_vm(int argc, char **argv) {
        using namespace boost::program_options;

        // {{{ XXX: customize
        options_description desc("example [options]");
        auto op_decl = desc.add_options();
        op_decl("help,h", "produce help message");
        op_decl("str,s",
                value<std::string>(&str)->default_value("default string"),
                "string option");
        op_decl("int,i", value<int>(&i)->required(), "integer option");
        op_decl("vec,v", value<std::vector<std::string>>(&vec),
                "vector option, -v foo -v bar");
        op_decl("flag,f", bool_switch(&flag)->default_value(false),
                "bool value");
        // }}}

        variables_map vm;
        try {
            store(parse_command_line(argc, argv, desc), vm);
            if (vm.size() == 0 || vm.count("help")) {
                std::cout << desc;
                std::exit(0);
            }
            notify(vm);
        }
        catch (boost::program_options::error &e) {
            std::cerr << "ERROR: " << e.what() << "\n\n";
            std::cerr << "[USAGE]:\n";
            std::cerr << desc;
            std::exit(1);
        }
        return vm;
    }
};
