#include <stdio.h>
#include <stdlib.h>
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
    lst = g_list_append(lst, (gpointer)6);

    g_list_foreach(lst, print, NULL);
    g_list_free(lst);

    lst = NULL;
    lst = g_list_append(lst, (gpointer)g_strdup("hello"));
    lst = g_list_append(lst, (gpointer)g_strdup("world"));
    g_list_free_full(lst, free);

    return 0;
}

