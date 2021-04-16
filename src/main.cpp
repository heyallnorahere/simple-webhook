#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
std::string read_file(const std::string& path) {
	std::ifstream file(path);
	std::stringstream contents;
	std::string line;
	while (std::getline(file, line)) {
		contents << line << '\n';
	}
	file.close();
	return contents.str();
}
// data to pass to read_callback
struct read_callback_data {
	std::string data;
	size_t pos;
};
// libcurl read callback
size_t read_callback(char* buffer, size_t size, size_t nitems, read_callback_data* data) {
	size_t bytes_to_write = size * nitems;
	std::string to_write = data->data.substr(data->pos, bytes_to_write);
	size_t bytes_written = 0;
	for (size_t i = 0; i < to_write.length(); i++) {
		bytes_written++;
		buffer[i] = to_write[i];
	}
	data->pos += bytes_written;
	return bytes_written;
}
std::string create_discord_request(const std::string& content) {
	nlohmann::json request;
	request["content"] = content;
	return request.dump();
}
int main(int argc, const char* argv[]) {
	// check to see that there is at least one additional argument
	assert(argc >= 2);
	// read Still Alive lyrics from a file
	std::string lyrics = read_file("stillalive.txt");
	// init libcurl
	CURL* c = curl_easy_init();
	// set the url to the passed argument
	std::string url = argv[1];
	curl_easy_setopt(c, CURLOPT_URL, url.c_str());
	// im not worried about security, sorry
	curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, false);
	// generate a header list
	curl_slist* list = NULL;
	list = curl_slist_append(list, "Content-Type: application/json");
	curl_easy_setopt(c, CURLOPT_HTTPHEADER, list);
	// we are sending a post request
	curl_easy_setopt(c, CURLOPT_POST, 1L);
	// set readfunction and readdata
	read_callback_data data;
	data.data = create_discord_request(lyrics);
	data.pos = 0;
	curl_easy_setopt(c, CURLOPT_READFUNCTION, read_callback);
	curl_easy_setopt(c, CURLOPT_READDATA, &data);
	// send the request
	CURLcode code = curl_easy_perform(c);
	assert(code == CURLE_OK);
	// free header list
	curl_slist_free_all(list);
	// clean up libcurl
	curl_easy_cleanup(c);
	return 0;
}