#include <time.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    /* {{{ tm to string */

    struct tm time;
    char str[128];

    time.tm_year = 20; /* year since 1900 */
    time.tm_mon = 3; /* start from 0 */
    time.tm_mday = 30; /* start from 1 */

    /* 1920/04/30 */
    strftime(str, sizeof(str)/sizeof(*str), "%Y/%m/%d", &time);
    printf("%s\n", str);

    /* }}} */

    /* {{{ compare tm */

    struct tm t1, t2;
    t1.tm_year = 2010;
    t1.tm_mon = 9;
    t1.tm_mday = 20;
    t2.tm_year = 2010;
    t2.tm_mon = 8;
    t2.tm_mday = 22;

    time_t d1 = mktime(&t1);
    time_t d2 = mktime(&t2);

    double diff = difftime(d1, d2);
    printf("%lf\n", diff); /* 2419201.000000 */
    diff = difftime(d2, d1);
    printf("%lf\n", diff); /* -2419201.000000 */

    /* }}} */
    
    return 0;
}

