#ifndef SLACK_GTK_CHANNEL_H
#define SLACK_GTK_CHANNEL_H

#include <json/json.h>

struct channel {
  std::string id;
  std::string name;
  bool is_member;

  channel(const Json::Value& c)
      : id(c["id"].asString()),
        name(c["name"].asString()),
        is_member(c["is_member"].asBool()) {
  }
};

#endif
