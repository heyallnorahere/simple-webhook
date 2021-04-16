#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cassert>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
// read a plain text file
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
// read a json file
nlohmann::json read_json(const std::string& path) {
	std::ifstream file(path);
	nlohmann::json contents;
	file >> contents;
	file.close();
	return contents;
}
struct line {
	std::string content;
	double delay;
};
void from_json(const nlohmann::json& j, line& l) {
	j["content"].get_to(l.content);
	j["delay"].get_to(l.delay);
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
void send_message(CURL* c, const std::string& content) {
	// create a discord request
	read_callback_data data;
	data.data = create_discord_request(content);
	data.pos = 0;
	// send the request to discord
	curl_easy_setopt(c, CURLOPT_READDATA, &data);
	assert(curl_easy_perform(c) == CURLE_OK);
}
double get_time() {
	return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now().time_since_epoch()).count();
}
int main(int argc, const char* argv[]) {
	// check to see that there is at least one additional argument
	assert(argc >= 2);
	// read Still Alive lyrics from a file
	std::string lyrics = read_file("stillalive.txt");
	std::vector<line> delayed_lyrics = read_json("stillalive.json").get<std::vector<line>>();
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
	// set readfunction
	curl_easy_setopt(c, CURLOPT_READFUNCTION, read_callback);
	// start dumping lyrics
	double start_time = get_time();
	for (const auto& l : delayed_lyrics) {
		while (get_time() - start_time < l.delay) { /* wait */ }
		send_message(c, l.content);
		start_time = get_time();
	}
	// free header list
	curl_slist_free_all(list);
	// clean up libcurl
	curl_easy_cleanup(c);
	return 0;
}