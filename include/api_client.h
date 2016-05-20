#ifndef SLACK_GTK_API_CLIENT_H
#define SLACK_GTK_API_CLIENT_H

#include <json/json.h>
#include <libsoup/soup.h>
#include <boost/optional.hpp>

class api_client {
 public:
  api_client(const std::string endpoint, const std::string token);
  ~api_client();
  api_client(const api_client& other);

  boost::optional<Json::Value> post(
      const std::string& method_name,
      const std::map<std::string, std::string>& params);

 private:
  void setup();
  size_t on_write(char* ptr, size_t size);
  static size_t write_callback(char* ptr, size_t size, size_t nmemb,
                               void* userdata);

  SoupSession* session_;
  const std::string endpoint_;
  const std::string token_;
  std::string response_buffer_;
};

#endif
