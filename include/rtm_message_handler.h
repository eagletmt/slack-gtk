#ifndef SLACK_CPP_RTM_MESSAGE_HANDLER_H
#define SLACK_CPP_RTM_MESSAGE_HANDLER_H

#include <json/json.h>
#include <functional>

class message_handler {
 public:
  void operator()(const Json::Value& payload) const;
  template <class F>
  void register_handler(const std::string& type, const F& handler) {
    registry_.insert(std::make_pair(type, handler));
  }

 private:
  std::map<std::string, std::function<void(const Json::Value&)> > registry_;
};

#endif
