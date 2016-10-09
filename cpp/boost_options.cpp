// compile with g++ -o boost_options boost_options.cpp -lboost_system
// -lboost_program_options -std=c++11
#include <boost/program_options.hpp>
#include <string>

class ProgramOption {
public:
    std::string str;
    int i;
    std::vector<std::string> vec;

    std::string to_string() {
        // {{{ XXX: customize
        std::ostringstream os;
        os << "==================================================\n"
           << "string option: " << str << "\n"
           << "integer option: " << i << "\n"
           << "vector option:";
        for (const auto &s : vec) {
            os << " || " << s;
        }
        os << "\n";
        os << "==================================================\n";
        // }}}
        return os.str();
    }

    static ProgramOption parse_args(int argc, char **argv) {
        ProgramOption opt;
        auto vm = opt.to_vm(argc, argv);
        opt.to_options(vm);
        return opt;
    }

private:
    boost::program_options::variables_map to_vm(int argc, char **argv) {
        using namespace boost::program_options;

        // {{{ XXX: customize
        options_description desc("example [options]");
        auto op_decl = desc.add_options();
        op_decl("help,h", "produce help message");
        op_decl("str,s", value<std::string>(), "string option");
        op_decl("int,i", value<int>()->required(), "integer option");
        op_decl("vec,v", value<std::vector<std::string>>(),
                "vector option, -v foo -v bar");
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

    void to_options(boost::program_options::variables_map vm) {
        using namespace boost::program_options;

        // {{{ XXX: customize
        if (vm.count("str") == 0) {
            std::cout << "no str option specified" << std::endl;
        } else {
            str = vm["str"].as<std::string>();
        }
        if (vm.count("vec") == 0) {
            std::cout << "no vec option specified" << std::endl;
        } else {
            vec = vm["vec"].as<std::vector<std::string>>();
        }
        i = vm["int"].as<int>();
        // }}}
    }
};

int main(int argc, char *argv[]) {
    auto opts = ProgramOption::parse_args(argc, argv);
    std::cout << opts.to_string() << std::endl;
    return 0;
}
