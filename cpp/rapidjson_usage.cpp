#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h> 
#include <rapidjson/filereadstream.h>

/* NOTE
 * rapidjson is awkward, too bad to use, just use jansson
 */

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

    doc.AddMember("name", "周杰伦", allocator);
    doc.AddMember("gender", "男", allocator);

    /* create an array property */
    Value arr(kArrayType);

    Value alb(kObjectType);
    alb.AddMember("name", "hello", allocator);
    alb.AddMember("year", "2001", allocator);
    /* after adding a member, you can change it like this */
    alb["name"] = "范特西";
    /* this will set alb to a null Value */
    arr.PushBack(alb, allocator);
    assert(alb.IsNull());

    Value alb2(kObjectType);
    alb2.AddMember("name", "八度空间", allocator);
    alb2.AddMember("year", "2002", allocator);
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

