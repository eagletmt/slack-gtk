#include "team.h"
#include "channels_store.h"
#include "emoji_loader.h"
#include "icon_loader.h"
#include "rtm_client.h"
#include "users_store.h"

team::team(std::shared_ptr<api_client> api_client,
           const std::string& emoji_directory, const Json::Value& json)
    : api_client_(api_client),
      rtm_client_(std::make_shared<rtm_client>(json)),
      users_store_(std::make_shared<users_store>(json)),
      channels_store_(std::make_shared<channels_store>(json)),
      // TODO: Use proper directory
      icon_loader_(std::make_shared<icon_loader>("icons")),
      emoji_loader_(std::make_shared<emoji_loader>(emoji_directory)) {
}

team::~team() {
}
