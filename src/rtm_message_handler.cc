#include "rtm_message_handler.h"
#include <iostream>

void message_handler::operator()(const Json::Value& payload) const {
  Json::Value json_type = payload["type"];
  if (!json_type.isString()) {
    std::cerr << "Invalid payload: " << payload << std::endl;
    return;
  }
  const std::string type = json_type.asString();
  auto it = registry_.find(type);
  if (it == registry_.end()) {
    std::cerr << "Unknown payload type: " << type << std::endl;
    std::cerr << payload << std::endl;
  } else {
    it->second(payload);
  }
}
