#include <vector>
#include <string>
#include <sstream>

#include <stdio.h>

using namespace std;

void string_split_char(string *str, char delim, vector<string> **pvec)
{
    stringstream ss(*str);
    string item;
    vector<string> *vec = new vector<string>();
    while (getline(ss, item, delim)) {
        vec->push_back(item);
    }
    *pvec = vec;
}

int main(int argc, char *argv[])
{
    string str("hello, world");
    vector<string> *vec = NULL;

    string_split_char(&str, ',', &vec);

    for (vector<string>::iterator it = vec->begin(); it != vec->end(); ++it) {
        printf("%s\n", (*it).c_str());
    }

    delete vec;
    return 0;
}

