#include "api_client.h"
#include <gdkmm/pixbufloader.h>
#include <glibmm/miscutils.h>
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
  if (!Glib::getenv("SLACK_GTK_LIBSOUP_DEBUG").empty()) {
    SoupLogger* logger = soup_logger_new(SOUP_LOGGER_LOG_BODY, -1);
    soup_session_add_feature(session_, SOUP_SESSION_FEATURE(logger));
    g_object_unref(logger);
  }
}

SoupMessage* api_client::build_message(
    const std::string& method_name,
    const std::map<std::string, std::string>& params) const {
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
  return message;
}

static boost::optional<Json::Value> parse_json(SoupMessage* message) {
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

boost::optional<Json::Value> api_client::post(
    const std::string& method_name,
    const std::map<std::string, std::string>& params) {
  SoupMessage* message = build_message(method_name, params);
  soup_session_send_message(session_, message);
  return parse_json(message);
}

void api_client::queue_post(const std::string& method_name,
                            const std::map<std::string, std::string>& params,
                            const post_callback_type& callback) {
  SoupMessage* message = build_message(method_name, params);
  callback_registry_.emplace(
      std::make_pair(reinterpret_cast<std::intptr_t>(message), callback));
  soup_session_queue_message(session_, message, queue_callback, this);
}

void api_client::queue_callback(SoupSession*, SoupMessage* message,
                                gpointer user_data) {
  static_cast<api_client*>(user_data)->on_queue_callback(message);
}

void api_client::on_queue_callback(SoupMessage* message) {
  auto result = parse_json(message);
  auto it = callback_registry_.find(reinterpret_cast<std::intptr_t>(message));
  if (it == callback_registry_.end()) {
    std::cerr << "[api_client] unknown message is passed to callback. SHOULD "
                 "NOT HAPPEN"
              << std::endl;
  } else {
    (it->second)(result);
    callback_registry_.erase(it);
  }
}

void api_client::get_shared_file(
    const std::string& url, const get_shared_file_callback_type& callback) {
  SoupMessage* message = soup_message_new("GET", url.c_str());
  const std::string auth_header = "Bearer " + token_;
  soup_message_headers_append(message->request_headers, "Authorization",
                              auth_header.c_str());

  get_shared_file_callbacks_.emplace(std::make_pair(url, callback));
  soup_session_queue_message(session_, message, get_shared_file_callback, this);
}

void api_client::get_shared_file_callback(SoupSession*, SoupMessage* message,
                                          gpointer user_data) {
  static_cast<api_client*>(user_data)->on_shared_file_loaded(message);
}

void api_client::on_shared_file_loaded(SoupMessage* message) {
  char* uri_c = soup_uri_to_string(soup_message_get_uri(message), FALSE);
  const std::string uri(uri_c);
  auto equal_range = get_shared_file_callbacks_.equal_range(uri);

  if (SOUP_STATUS_IS_TRANSPORT_ERROR(message->status_code)) {
    std::cerr << "[api_client] Failed to load shared file " << uri << " ("
              << message->status_code << ") "
              << soup_status_get_phrase(message->status_code) << std::endl;
  } else {
    try {
      Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
      loader->write(
          reinterpret_cast<const guint8*>(message->response_body->data),
          message->response_body->length);
      loader->close();
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = loader->get_pixbuf();
      for (auto it = equal_range.first; it != equal_range.second; ++it) {
        (it->second)(loader->get_pixbuf());
      }
    } catch (Gdk::PixbufError& err) {
      std::cerr << uri << ": " << err.what() << std::endl;
    }
  }

  get_shared_file_callbacks_.erase(equal_range.first, equal_range.second);
  g_free(uri_c);
}
