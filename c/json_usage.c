#include <stdio.h>
#include <stdlib.h>

#include <jansson.h>

/* for more info, see http://jansson.readthedocs.org/en/latest/ */

void load_from_file_and_query()
{
    json_t *json = NULL;
    json_error_t jerr;

    const char *json_file_path = "../data/test.json";
    
    /* load json file */
    json = json_load_file(json_file_path, 0, &jerr);
    if (json == NULL) {
        fprintf(stderr, "error on json file line %d: %s\n",
                jerr.line, jerr.text);
        return;
    }
    if (! json_is_object(json)) {
        fprintf(stderr, "json file is not an object");
        json_decref(json);
        return;
    }

    /* query name */
    json_t *name = json_object_get(json, "name");
    if (! json_is_string(name)) {
        fprintf(stderr, "name is not a string");
        json_decref(json);
        return;
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
            return;
        }

        printf("album name: %s\n",
                json_string_value(json_object_get(alb, "name")));
        printf("album year: %s\n",
                json_string_value(json_object_get(alb, "year")));
    }

    json_decref(json);
}

void load_from_string()
{
    json_error_t jerr;
    const char *json = "{ \"hello\": \"world\"}";
    json_t *jsn = json_loads(json, 0, &jerr);
    if (jsn == NULL) {
        fprintf(stderr, "fail to load from string\n");
        return;
    }
    json_decref(jsn);
}

void create_json_and_dump()
{
    json_t *json = json_object();
    if (json == NULL) {
        fprintf(stderr, "fail to create json object\n");
        return;
    }
    json_t *jay = json_string("周杰伦");
    if (jay == NULL) {
        fprintf(stderr, "fail to create jay\n");
        return;
    }
    /* NOTE that use *_new function in case you no longer need jay
     * after this call. This will do yourself a favor when free json
     * object, otherwise, you'll have to free all object you create */
    if (json_object_set_new(json, "name", jay) != 0) {
        fprintf(stderr, "fail to set name\n");
        return;
    }

    /* XXX: error checking are omitted below */
    json_object_set_new(json, "gender", json_string("男"));
    json_t *albums = json_array();
    json_t *alb1 = json_object();
    json_object_set_new(alb1, "name", json_string("范特西"));
    json_object_set_new(alb1, "year", json_string("2001"));
    json_array_append_new(albums, alb1);
    json_t *alb2 = json_object();
    json_object_set_new(alb2, "name", json_string("八度空间"));
    json_object_set_new(alb2, "year", json_string("2002"));
    json_array_append_new(albums, alb2);
    json_object_set_new(json, "albums", albums);

    /*
     * {
     *   "name": "周杰伦",
     *   "albums": [
     *     {
     *       "name": "范特西",
     *       "year": "2001"
     *     },
     *     {
     *       "name": "八度空间",
     *       "year": "2002"
     *     }
     *   ],
     *   "gender": "男"
     * }
     */
    const char *jstr = json_dumps(json, JSON_INDENT(2));
    printf("%s\n", jstr);
    json_decref(json);
    free((void *)jstr);
}

int main(int argc, char *argv[])
{
    load_from_file_and_query();
    load_from_string();
    create_json_and_dump();
    return 0;
}

