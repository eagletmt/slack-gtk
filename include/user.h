#ifndef SLACK_GTK_USER_H
#define SLACK_GTK_USER_H

#include <json/json.h>
#include "user_profile.h"

// https://api.slack.com/types/user

struct user {
  std::string id;
  std::string name;
  user_profile profile;

  user(const Json::Value& user)
      : id(user["id"].asString()),
        name(user["name"].asString()),
        profile(user["profile"]) {
  }
};

#endif
