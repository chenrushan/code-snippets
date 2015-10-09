/* Demostration:
 *   1. basic use of libcurl for GET request
 *   2. set http header
 */

#include <stdio.h>
#include <curl/curl.h>

int main(void)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (! curl) {
        fprintf(stderr, "fail to init curl\n");
        return -1;
    }

    /* for GET request, just add all parameters to url, and no escape is needed */
    const char *url = "http://example.com:port/?para=foo bar";

    struct curl_slist *headers = NULL;
    /* Accept json response
     * note that you should reassign headers
     */
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);

    /* Check for errors */ 
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    /* always cleanup */ 
    curl_easy_cleanup(curl);

    return 0;
}

