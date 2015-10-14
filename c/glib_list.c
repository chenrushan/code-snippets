#include <stdio.h>
#include <glib.h>

void print(gpointer item, gpointer user_data)
{
    printf("%d\n", (int)item);
}

int main(int argc, char *argv[])
{
    GList *lst = NULL;

    lst = g_list_append(lst, (gpointer)3);
    lst = g_list_append(lst, (gpointer)5);

    g_list_foreach(lst, print, NULL);

    return 0;
}

