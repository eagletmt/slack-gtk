#include "api_client.h"
#include <iostream>
#include <memory>

api_client::api_client(const std::string endpoint, const std::string token)
    : session_(soup_session_new()), endpoint_(endpoint), token_(token) {
  setup();
}

api_client::api_client(const api_client& other)
    : session_(soup_session_new()),
      endpoint_(other.endpoint_),
      token_(other.token_) {
  setup();
}

api_client::~api_client() {
  g_object_unref(session_);
}

void api_client::setup() {
  soup_session_add_feature_by_type(session_, soup_content_decoder_get_type());
  SoupLogger* logger = soup_logger_new(SOUP_LOGGER_LOG_HEADERS, -1);
  soup_session_add_feature(session_, SOUP_SESSION_FEATURE(logger));
  g_object_unref(logger);
}

boost::optional<Json::Value> api_client::post(
    const std::string& method_name,
    const std::map<std::string, std::string>& params) {
  const std::string url(endpoint_ + "/" + method_name);

  std::unique_ptr<GHashTable, decltype(&g_hash_table_unref)> table(
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free),
      g_hash_table_unref);
  g_hash_table_insert(table.get(), g_strdup("token"), g_strdup(token_.c_str()));
  for (const std::pair<std::string, std::string>& param : params) {
    g_hash_table_insert(table.get(), g_strdup(param.first.c_str()),
                        g_strdup(param.second.c_str()));
  }

  SoupMessage* message =
      soup_form_request_new_from_hash("POST", url.c_str(), table.get());
  soup_message_set_flags(message, SOUP_MESSAGE_NO_REDIRECT);
  soup_session_send_message(session_, message);

  if (SOUP_STATUS_IS_TRANSPORT_ERROR(message->status_code)) {
    std::cerr << "libsoup: (" << message->status_code << ") "
              << soup_status_get_phrase(message->status_code) << std::endl;
    return boost::optional<Json::Value>();
  } else {
    Json::Reader reader;
    Json::Value root;
    reader.parse(message->response_body->data,
                 message->response_body->data + message->response_body->length,
                 root);
    if (reader.good()) {
      return boost::make_optional(root);
    } else {
      std::cerr << "jsoncpp: " << reader.getFormattedErrorMessages()
                << std::endl;
      return boost::optional<Json::Value>();
    }
  }
}
