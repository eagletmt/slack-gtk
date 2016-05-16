#ifndef SLACK_CPP_RTM_CLIENT_IMPL_H
#define SLACK_CPP_RTM_CLIENT_IMPL_H
#include <json/json.h>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio.hpp>
#include "rtm_message_handler.h"

class rtm_client_impl {
  typedef rtm_client_impl type;
  typedef websocketpp::config::asio_tls client_config_type;
  typedef websocketpp::client<client_config_type> client_type;

 public:
  rtm_client_impl(boost::asio::io_service& io_service,
                  const Json::Value& rtm_start,
                  const message_handler& message_handler);

  void start();

 private:
  std::shared_ptr<boost::asio::ssl::context> on_tls_init(
      websocketpp::connection_hdl);

  void on_message(websocketpp::connection_hdl,
                  client_config_type::message_type::ptr message);
  void on_close(websocketpp::connection_hdl);

  void connect();

  std::string url_;
  client_type client_;
  const message_handler message_handler_;
};

#endif
