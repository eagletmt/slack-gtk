#include "api_client.h"
#include <iostream>

api_client::api_client(const std::string endpoint, const std::string token)
    : curl_(curl_easy_init()), endpoint_(endpoint), token_(token) {
  setup_curl();
}

api_client::api_client(const api_client& other)
    : curl_(curl_easy_init()),
      endpoint_(other.endpoint_),
      token_(other.token_) {
  setup_curl();
}

api_client::~api_client() {
  curl_easy_cleanup(curl_);
}

void api_client::setup_curl() {
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this);
}

size_t api_client::write_callback(char* ptr, size_t size, size_t nmemb,
                                  void* userdata) {
  return static_cast<api_client*>(userdata)->on_write(ptr, size * nmemb);
}

boost::optional<Json::Value> api_client::get(const std::string method_name) {
  curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
  const std::string url(endpoint_ + "/" + method_name + "?token=" + token_);
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  char errbuf[CURL_ERROR_SIZE] = {0};
  curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER, errbuf);

  CURLcode res = curl_easy_perform(curl_);
  if (res == CURLE_OK) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(response_buffer_, root);
    response_buffer_.clear();
    if (reader.good()) {
      return boost::make_optional(root);
    } else {
      std::cerr << "jsoncpp: " << reader.getFormattedErrorMessages()
                << std::endl;
      return boost::optional<Json::Value>();
    }
  } else {
    std::cerr << "curl: (" << res << ") " << errbuf << std::endl;
    return boost::optional<Json::Value>();
  }
}

size_t api_client::on_write(char* ptr, size_t size) {
  response_buffer_.append(ptr, size);
  return size;
}
