#ifndef SLACK_CPP_API_CLIENT_H
#define SLACK_CPP_API_CLIENT_H

#include <curl/curl.h>
#include <json/json.h>
#include <boost/optional.hpp>

class api_client {
 public:
  api_client(const std::string endpoint, const std::string token);
  ~api_client();

  boost::optional<Json::Value> get(const std::string method_name);

 private:
  size_t on_write(char* ptr, size_t size);
  static size_t write_callback(char* ptr, size_t size, size_t nmemb,
                               void* userdata);

  CURL* curl_;
  const std::string endpoint_;
  const std::string token_;
  std::string response_buffer_;
};

#endif
