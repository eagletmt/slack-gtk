#ifndef SLACK_GTK_USER_PROFILE_H
#define SLACK_GTK_USER_PROFILE_H

#include <json/json.h>

// https://api.slack.com/types/user

struct user_profile {
  std::string image_72;

  user_profile(const Json::Value& profile)
      : image_72(profile["image_72"].asString()) {
  }
};

#endif
