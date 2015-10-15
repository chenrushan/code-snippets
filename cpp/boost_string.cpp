#include <stdio.h>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

using namespace std;

void string_split_demo()
{
    vector<string> vec;
    string str("hello,,world");

    /* split string
     * vec: the place where each part is stored
     * str: string to split
     * boost::is_any_of(): a function return true if one of the chars in arguments is found
     *   
     * By default, every two separators delimit a token
     */
    boost::split(vec, str, boost::is_any_of(","));
    /* result:
     *  0: hello
     *  1:
     *  2: world
     */
    for (int i = 0; i < vec.size(); ++i) {
        printf("%d: %s\n", i, vec[i].c_str());
    }

    /* vec is reused and the old result are overwritten
     * by specifying boost::algorithm::token_compress_on, empty field will
     * be discarded
     */
    boost::split(vec, str, boost::is_any_of(","), boost::algorithm::token_compress_on);
    /* result:
     *  0: hello
     *  1: world
     */
    for (int i = 0; i < vec.size(); ++i) {
        printf("%d: %s\n", i, vec[i].c_str());
    }
}

int main(int argc, char *argv[])
{
    string_split_demo();

    return 0;
}

