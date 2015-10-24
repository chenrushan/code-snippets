#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h> 
#include <rapidjson/filereadstream.h>

using namespace rapidjson;
using namespace std;

void load_and_query()
{
    /* {{{ load from file */
    FILE *fp = fopen("../data/test.json", "r");
    if (fp == NULL) {
        fprintf(stderr, "fail to read file\n");
        return;
    }
    char read_buf[65536];
    FileReadStream is(fp, read_buf, sizeof(read_buf));
    Document doc;
    doc.ParseStream(is);
    if (! doc.IsObject()) {
        fprintf(stderr, "doc not a object\n");
        return;
    }
    /* }}} */

    /* simple property */
    printf("name: %s\n", doc["name"].GetString());
    printf("gender: %s\n", doc["gender"].GetString());

    /* array */
    const Value &albs = doc["albums"];
    if (! albs.IsArray()) {
        fprintf(stderr, "albums is not an array\n");
        return;
    }
    printf("albums\n");
    for (Value::ConstValueIterator it = albs.Begin();
         it != albs.End();
         ++it) {
        printf("\tname: %s\n", (*it)["name"].GetString());
        printf("\tyear: %s\n", (*it)["year"].GetString());
    }

    fclose(fp);
}

void create_and_pretty_format()
{
    Document doc(kObjectType);
    Document::AllocatorType &allocator = doc.GetAllocator();

    /* create a string property */
    Value name(kObjectType);
    name.SetString("周杰伦", strlen("周杰伦"), allocator);
    doc.AddMember("name", name, allocator);

    /* create a string property */
    Value gender(kObjectType);
    gender.SetString("男", strlen("男"), allocator);
    doc.AddMember("gender", gender, allocator);

    /* create an array property */
    Value arr(kArrayType);

    Value alb(kObjectType);
    Value alb_name(kObjectType), alb_year(kObjectType);
    alb_name.SetString("范特西", strlen("范特西"), allocator);
    alb_year.SetString("2001", strlen("2001"), allocator);
    alb.AddMember("name", alb_name, allocator);
    alb.AddMember("year", alb_year, allocator);
    arr.PushBack(alb, allocator);

    Value alb2(kObjectType);
    Value alb_name2(kObjectType), alb_year2(kObjectType);
    alb_name2.SetString("八度空间", strlen("八度空间"), allocator);
    alb_year2.SetString("2002", strlen("2002"), allocator);
    alb2.AddMember("name", alb_name2, allocator);
    alb2.AddMember("year", alb_year2, allocator);
    arr.PushBack(alb2, allocator);

    doc.AddMember("albums", arr, allocator);

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    doc.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
    puts(sb.GetString());
}

int main(int argc, char *argv[])
{
    load_and_query();
    create_and_pretty_format();
    return 0;
}

