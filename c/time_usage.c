#include <time.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    struct tm time;
    char str[128];

    time.tm_year = 20; /* year since 1900 */
    time.tm_mon = 3; /* start from 0 */
    time.tm_mday = 30; /* start from 1 */

    /* 1920/04/30 */
    strftime(str, sizeof(str)/sizeof(*str), "%Y/%m/%d", &time);

    printf("%s\n", str);
    
    return 0;
}

