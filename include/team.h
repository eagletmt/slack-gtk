#ifndef SLACK_GTK_TEAM_H
#define SLACK_GTK_TEAM_H

#include <memory>
#include "api_client.h"
#include "channels_store.h"
#include "channels_store.h"
#include "emoji_loader.h"
#include "icon_loader.h"
#include "rtm_client.h"
#include "users_store.h"

class api_client;
class rtm_client;

class team {
 public:
  team(std::shared_ptr<api_client> api_client,
       const std::string& emoji_directory, const Json::Value& json);
  ~team();

  std::shared_ptr<api_client> api_client_;
  std::shared_ptr<rtm_client> rtm_client_;
  std::shared_ptr<users_store> users_store_;
  std::shared_ptr<channels_store> channels_store_;
  std::shared_ptr<icon_loader> icon_loader_;
  std::shared_ptr<emoji_loader> emoji_loader_;
};

#endif
