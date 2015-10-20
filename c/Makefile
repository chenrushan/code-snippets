all: json_usage curl_usage glib_list time_usage

json_usage: json_usage.c
	gcc -o $@ $^ -ljansson
curl_usage: curl_usage.c
	gcc -o $@ $^ -lcurl
glib_list: glib_list.c
	gcc -o $@ $^ -lglib-2.0 -I /usr/include/glib-2.0/ -I /usr/lib/glib-2.0/include/
time_usage: time_usage.c
	gcc -o $@ $^

clean:
	find -type f -executable -exec rm {} \;
