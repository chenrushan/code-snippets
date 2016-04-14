#include <iostream>

#include <limits> // for quiet_NaN()
#include <cmath>  // for isnan()

using namespace std;

int main(int argc, char *argv[])
{
    double nan = numeric_limits<double>::quiet_NaN();

    // should use std:: when calling isnan()
    if (std::isnan(nan)) {
        cout << "is nan" << endl;
    }
    
    return 0;
}

