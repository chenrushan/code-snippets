// ------------------------------------------------------------
// compile with g++ -o -lboost_system -lboost_date_time
// for more info
// ------------------------------------------------------------

#include <iostream>
#include <boost/date_time/gregorian/gregorian.hpp>

using namespace std;

int main(int argc, char *argv[]) {
    boost::gregorian::date d(2015, 10, 11);

    cout << boost::gregorian::to_iso_extended_string(d) << endl;

    d += boost::gregorian::days(1);
    cout << boost::gregorian::to_iso_extended_string(d) << endl;

    d += boost::gregorian::months(1);
    cout << boost::gregorian::to_iso_extended_string(d) << endl;

    boost::gregorian::date d2(2015, 11, 30);
    d2 += boost::gregorian::months(1);
    cout << boost::gregorian::to_iso_extended_string(d2) << endl;

    while (d < d2) {
        cout << boost::gregorian::to_iso_extended_string(d) << endl;
        d += boost::gregorian::days(1);
    }

    // get today
    boost::gregorian::date today = boost::gregorian::day_clock::local_day();
    cout << boost::gregorian::to_iso_extended_string(today) << endl;

    return 0;
}
