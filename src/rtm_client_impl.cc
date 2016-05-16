#include "rtm_client_impl.h"

rtm_client_impl::rtm_client_impl(boost::asio::io_service& io_service,
                                 const Json::Value& rtm_start,
                                 const message_handler& message_handler)
    : url_(rtm_start["url"].asString()), message_handler_(message_handler) {
  client_.init_asio(&io_service);
  client_.set_tls_init_handler(
      std::bind(&type::on_tls_init, this, std::placeholders::_1));
  client_.set_message_handler(std::bind(
      &type::on_message, this, std::placeholders::_1, std::placeholders::_2));
  client_.set_close_handler(
      std::bind(&type::on_close, this, std::placeholders::_1));
}

void rtm_client_impl::start() {
  connect();
  // client_.run();
}

std::shared_ptr<boost::asio::ssl::context> rtm_client_impl::on_tls_init(
    websocketpp::connection_hdl) {
  return std::make_shared<boost::asio::ssl::context>(
      boost::asio::ssl::context::sslv23);
}

void rtm_client_impl::on_message(
    websocketpp::connection_hdl,
    client_config_type::message_type::ptr message) {
  Json::Reader reader;
  Json::Value payload;
  reader.parse(message->get_payload(), payload);
  if (!reader.good()) {
    std::cerr << "Unparsable payload: " << message->get_payload() << std::endl;
    return;
  }
  message_handler_(payload);
}

void rtm_client_impl::on_close(websocketpp::connection_hdl) {
  std::cerr << "Connection closed. Start reconnecting to " << url_ << std::endl;
  connect();
}

void rtm_client_impl::connect() {
  websocketpp::lib::error_code error;
  client_type::connection_ptr connection = client_.get_connection(url_, error);
  if (error) {
    std::cerr << "get_connection failed: " << error.message() << std::endl;
    return;
  }
  client_.connect(connection);
}
