all: json_usage curl_usage

json_usage: json_usage.c
	gcc -o $@ $^ -ljansson
curl_usage: curl_usage.c
	gcc -o $@ $^ -lcurl

clean:
	find -type f -executable -exec rm {} \;
