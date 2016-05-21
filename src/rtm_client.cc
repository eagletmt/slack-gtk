#include "rtm_client.h"
#include <glibmm/miscutils.h>
#include <libsoup/soup-logger.h>
#include <libsoup/soup-session-feature.h>
#include <iostream>

rtm_client::rtm_client(const Json::Value &rtm_start)
    : url_(rtm_start["url"].asString()), session_(soup_session_new()) {
  if (!Glib::getenv("SLACK_GTK_LIBSOUP_DEBUG").empty()) {
    SoupLogger *logger = soup_logger_new(SOUP_LOGGER_LOG_HEADERS, -1);
    soup_session_add_feature(session_, SOUP_SESSION_FEATURE(logger));
    g_object_unref(logger);
  }
  // FIXME: libsoup doesn't handle wss protocol correctly.
  if (url_.substr(0, 6) == "wss://") {
    url_.replace(0, 3, "https");
  }
}

rtm_client::~rtm_client() {
  g_object_unref(session_);
}

void rtm_client::start() {
  session_ = soup_session_new();
  SoupMessage *message = soup_message_new("GET", url_.c_str());
  soup_session_websocket_connect_async(session_, message, nullptr, nullptr,
                                       nullptr, session_connect_callback, this);
}

void rtm_client::session_connect_callback(GObject *source, GAsyncResult *result,
                                          gpointer user_data) {
  static_cast<rtm_client *>(user_data)->on_session_connect(SOUP_SESSION(source),
                                                           result);
}
void rtm_client::closed_callback(SoupWebsocketConnection *,
                                 gpointer user_data) {
  static_cast<rtm_client *>(user_data)->on_closed();
}
void rtm_client::closing_callback(SoupWebsocketConnection *,
                                  gpointer user_data) {
  static_cast<rtm_client *>(user_data)->on_closing();
}
void rtm_client::error_callback(SoupWebsocketConnection *, GError *error,
                                gpointer user_data) {
  static_cast<rtm_client *>(user_data)->on_error(error);
}
void rtm_client::message_callback(SoupWebsocketConnection *, gint type,
                                  GBytes *message, gpointer user_data) {
  static_cast<rtm_client *>(user_data)->on_message(
      static_cast<SoupWebsocketDataType>(type), message);
}

void rtm_client::on_session_connect(SoupSession *session,
                                    GAsyncResult *result) {
  GError *error = nullptr;
  connection_ = soup_session_websocket_connect_finish(session, result, &error);
  g_assert_no_error(error);

  g_signal_connect(connection_, "closed", G_CALLBACK(closed_callback), this);
  g_signal_connect(connection_, "closing", G_CALLBACK(closing_callback), this);
  g_signal_connect(connection_, "error", G_CALLBACK(error_callback), this);
  g_signal_connect(connection_, "message", G_CALLBACK(message_callback), this);
}

void rtm_client::on_closed() {
  std::cout << "on_closed" << std::endl;
}
void rtm_client::on_closing() {
  std::cout << "on_closing" << std::endl;
}
void rtm_client::on_error(GError *error) {
  std::cout << "on_error(" << error->code << ": " << error->message << ")"
            << std::endl;
}
void rtm_client::on_message(SoupWebsocketDataType type, GBytes *message) {
  switch (type) {
    case SOUP_WEBSOCKET_DATA_TEXT: {
      gsize size = 0;
      const char *ptr =
          static_cast<decltype(ptr)>(g_bytes_get_data(message, &size));
      handle_payload(ptr, size);
      break;
    }
    case SOUP_WEBSOCKET_DATA_BINARY:
      std::cerr << "on_message: binary message isn't supported" << std::endl;
      break;
  }
}

void rtm_client::handle_payload(const char *payload, size_t size) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(std::string(payload, payload + size), root);
  if (reader.good()) {
    const Json::Value type_value = root["type"];
    if (type_value.isString()) {
      const std::string type = type_value.asString();
      if (type == "hello") {
        hello_signal_.emit(root);
      } else if (type == "reconnect_url") {
        reconnect_url_signal_.emit(root);
      } else if (type == "presence_change") {
        presence_change_signal_.emit(root);
      } else if (type == "pref_change") {
        pref_change_signal_.emit(root);
      } else if (type == "message") {
        message_signal_.emit(root);
      } else if (type == "channel_marked") {
        channel_marked_signal_.emit(root);
      } else if (type == "channel_joined") {
        channel_joined_signal_.emit(root);
      } else if (type == "channel_left") {
        channel_left_signal_.emit(root);
      } else {
        std::cerr << "rtm_client: unknown message type=" << type << std::endl;
        std::cerr << root << std::endl;
      }
    } else {
      std::cerr << "jsoncpp: invalid payload (missing type)" << std::endl;
      std::cerr << root << std::endl;
    }
  } else {
    std::cerr << "jsoncpp: " << reader.getFormattedErrorMessages() << std::endl;
  }
}

rtm_client::message_signal_type rtm_client::hello_signal() {
  return hello_signal_;
}
rtm_client::message_signal_type rtm_client::reconnect_url_signal() {
  return reconnect_url_signal_;
}
rtm_client::message_signal_type rtm_client::presence_change_signal() {
  return presence_change_signal_;
}
rtm_client::message_signal_type rtm_client::pref_change_signal() {
  return pref_change_signal_;
}
rtm_client::message_signal_type rtm_client::message_signal() {
  return message_signal_;
}
rtm_client::message_signal_type rtm_client::channel_marked_signal() {
  return channel_marked_signal_;
}
rtm_client::message_signal_type rtm_client::channel_joined_signal() {
  return channel_joined_signal_;
}
rtm_client::message_signal_type rtm_client::channel_left_signal() {
  return channel_left_signal_;
}
