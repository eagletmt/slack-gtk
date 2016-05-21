#include "channels_store.h"

channels_store::channels_store(const Json::Value& json) {
  for (const Json::Value& c : json["channels"]) {
    channels_.emplace(std::make_pair(c["id"].asString(), channel(c)));
  }
}

boost::optional<channel> channels_store::find(
    const std::string& channel_id) const {
  auto it = channels_.find(channel_id);
  if (it == channels_.cend()) {
    return boost::optional<channel>();
  } else {
    return boost::make_optional(it->second);
  }
}

const std::map<std::string, channel>& channels_store::data() const {
  return channels_;
}
