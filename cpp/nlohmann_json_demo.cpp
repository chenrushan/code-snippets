#include "lib/json.hpp"

#include <iostream>
#include <fstream>

using Json = nlohmann::json;

int main(int argc, char *argv[])
{
    // load from file
    std::cout << "[load from file]" << std::endl;
    std::ifstream ifs("data/example.json");
    auto jsn1 = Json::parse(ifs);
    std::cout << jsn1.dump(4) << std::endl;
    std::cout << "=================================" << std::endl;

    // load from string
    std::cout << "[load from string]" << std::endl;
    const std::string json_str = R"(
        {
          "glossary": {
            "title": "example glossary",
            "GlossDiv": {
              "title": "S",
              "GlossList": {
                "GlossEntry": {
                  "ID": "SGML",
                  "SortAs": "SGML",
                  "GlossTerm": "Standard Generalized Markup Language",
                  "Acronym": "SGML",
                  "Abbrev": "ISO 8879:1986",
                  "GlossDef": {
                    "para": "A meta-markup language, used to create markup languages such as DocBook.",
                    "GlossSeeAlso": ["GML", "XML"]
                  },
                  "GlossSee": "markup"
                }
              }
            }
          }
        })";
    std::stringstream ss(json_str);
    auto jsn2 = Json::parse(ss);
    std::cout << jsn2.dump(4) << std::endl;
    std::cout << "=================================" << std::endl;

    // get value
    std::cout << jsn2["glossary"] << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << jsn2["glossary"]["title"] << std::endl;
    std::cout << "=================================" << std::endl;

    // iterate over array
    for (const auto &ele : jsn2["glossary"]["GlossDiv"]["GlossList"]["GlossEntry"]["GlossDef"]["GlossSeeAlso"]) {
        std::cout << ele << std::endl;
    }
    std::cout << "=================================" << std::endl;

    // ======================================================================
    
    // create json
    Json jsn3;
    jsn3["foo"] = 1;
    jsn3["bar"] = { 1, 2, 3 };
    jsn3["zee"] = Json::array();
    jsn3["zee"].push_back("haha");
    jsn3["zee"].push_back("xixi");
    std::cout << jsn3.dump(2) << std::endl;
    
    return 0;
}

