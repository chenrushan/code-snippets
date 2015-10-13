/* Demostration:
 *   1. basic use of libcurl for GET request
 *   2. set http header
 *   3. put curl result into a string buffer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

/* string buffer to store curl result */
struct string {
    char *str;
    int len;
};

/* Define a callback function to be used by curl, input is pointed to by
 * @input, and its size is (@len * @nmemb). @user_data is provided by
 * user, it could be a string buffer where you can place @input
 * 
 * @user_data should be set by user with
 *   curl_easy_setopt(curl, CURLOPT_WRITEDATA, user_data)
 */
size_t writefunc(void *input, size_t len, size_t nmemb, void *user_data)
{
    struct string *s = user_data;
    size_t new_len = s->len + len * nmemb;

    s->str = realloc(s->str, new_len + 1);
    if (s->str == NULL) {
        fprintf(stderr, "realloc() failed\n");
        return 0; // return 0 means it doesn't write anything
    }
    memcpy(s->str + s->len, input, len * nmemb);
    s->str[new_len] = '\0';
    s->len = new_len;

    return len * nmemb;
}

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
    // const char *url = "http://example.com:port/?para=foo bar";
    const char *url = "http://192.168.1.16:8080/";

    struct curl_slist *headers = NULL;
    /* Accept json response
     * note that you should reassign headers
     */
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    /* {{{ perform simple request */

    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl); // print all results to stdout
    /* Check for errors */ 
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return -1;
    }

    /* }}} */

    /* {{{ save curl result to string */

    struct string str;
    memset(&str, 0, sizeof(str));

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return -1;
    }
    printf("string content: |%s|\n", str.str);
    free(str.str);

    /* }}} */

    /* always cleanup */ 
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}

