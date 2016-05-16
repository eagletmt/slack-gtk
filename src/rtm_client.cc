#include "rtm_client.h"
#include "rtm_client_impl.h"

rtm_client::rtm_client(boost::asio::io_service& io_service,
                       const Json::Value& rtm_start,
                       const message_handler& message_handler)
    : impl_(new rtm_client_impl(io_service, rtm_start, message_handler)) {
}

rtm_client::~rtm_client() {
  delete impl_;
}

void rtm_client::start() {
  impl_->start();
}
