#include <stdio.h>
#include <stdlib.h>

#include <jansson.h>

/* for more info, see http://jansson.readthedocs.org/en/latest/ */

int main(int argc, char *argv[])
{
    json_t *json = NULL;
    json_error_t jerr;

    const char *json_file_path = "../data/test.json";
    
    /* load json file */
    json = json_load_file(json_file_path, 0, &jerr);
    if (json == NULL) {
        fprintf(stderr, "error on json file line %d: %s\n", jerr.line, jerr.text);
        return -1;
    }
    if (! json_is_object(json)) {
        fprintf(stderr, "json file is not an object");
        json_decref(json);
        return -1;
    }

    /* query name */
    json_t *name = json_object_get(json, "name");
    if (! json_is_string(name)) {
        fprintf(stderr, "name is not a string");
        json_decref(json);
        return -1;
    }
    printf("%s\n", json_string_value(name));

    /* query albums */
    json_t *albums = json_object_get(json, "albums");
    int i = 0;
    for (i = 0; i < json_array_size(albums); ++i) {
        json_t *alb = json_array_get(albums, i);
        if (! json_is_object(alb)) {
            fprintf(stderr, "album is not an object\n");
            json_decref(json);
            return -1;
        }

        printf("album name: %s\n", json_string_value(json_object_get(alb, "name")));
        printf("album year: %s\n", json_string_value(json_object_get(alb, "year")));
    }

    json_decref(json);
    return 0;
}

