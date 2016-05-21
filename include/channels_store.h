#ifndef SLACK_GTK_CHANNELS_STORE_H
#define SLACK_GTK_CHANNELS_STORE_H

#include <json/json.h>
#include <boost/optional.hpp>
#include "channel.h"

class channels_store {
 public:
  channels_store(const Json::Value& json);

  boost::optional<channel> find(const std::string& channel_id) const;
  const std::map<std::string, channel>& data() const;

 private:
  std::map<std::string, channel> channels_;
};

#endif
