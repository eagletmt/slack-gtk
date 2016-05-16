#ifndef SLACK_CPP_RTM_CLIENT_H
#define SLACK_CPP_RTM_CLIENT_H

#include <json/json.h>
#include <boost/asio.hpp>
#include <string>
#include "rtm_message_handler.h"

class rtm_client_impl;

class rtm_client {
 public:
  rtm_client(boost::asio::io_service& io_service, const Json::Value& rtm_start,
             const message_handler& message_handler);
  ~rtm_client();

  void start();

 private:
  rtm_client_impl* impl_;
};

#endif
