// modify based on https://gist.github.com/ivangolo/a9edd93527d9ae4c9e04
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

void showhelpinfo(char *s);

int main (int argc,char *argv[])
{
    char o;

    // if the program is ran witout options ,it will show the usgage and exit
    if (argc == 1) {
        showhelpinfo(argv[0]);
        exit(1);
    }

    // use function getopt to get the arguments with option."hu:p:s:v" indicate 
    // that option h,v are the options without arguments while u,p,s are the
    // options with arguments
    while ((o = getopt(argc, argv, "hu:p:s:v")) != -1) {
        switch(o) {
        // option h show the help infomation
        case 'h':
            showhelpinfo(argv[0]);
            break;
        // option u present the username
        case 'u':
            cout << "Your username is " << optarg << endl;
            break;
        // option p present the password 
        case 'p':
            cout << "Your password is " << optarg << endl;
            break;
        // option s present the save option  
        case 's':
            if(strcmp(optarg,"1") == 0) {
                cout << "You have saved the password" << endl;
            } else if(strcmp(optarg,"0") == 0) {
                cout << "You have chosen to forget the password" << endl;
            } else {
                showhelpinfo(argv[0]);
            }
            break;
        // option v show the version infomation
        case 'v':
            cout << "The current version is 0.1" << endl;
            break;
        // invail input will get the heil infomation
        default:
            showhelpinfo(argv[0]);
            break;
        }
    }
    return 0;
}

// funcion that show the help information
void showhelpinfo(char *s)
{
    cout << "Usage:   " << s << " [-option] [argument]" << endl;
    cout << "option:  " << "-h show help information" << endl;
    cout << "         " << "-u username" << endl;
    cout << "         " << "-p password" << endl;
    cout << "         " << "-s save the password: 0(save password) 1(forget password)" << endl;
    cout << "         " << "-v show version infomation" << endl;
    cout << "example: " << s << " -uusername -ppassword -s1" << endl;
}

