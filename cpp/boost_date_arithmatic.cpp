// ------------------------------------------------------------
// see <http://www.boost.org/doc/libs/1_59_0/doc/html/date_time/gregorian.html#date_time.gregorian.date_class>
// for more info
// ------------------------------------------------------------

#include <iostream>
#include <boost/date_time/gregorian/gregorian.hpp>

using namespace std;

int main(int argc, char *argv[])
{
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

    return 0;
}

