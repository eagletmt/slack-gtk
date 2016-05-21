#ifndef SLACK_GTK_USERS_STORE_H
#define SLACK_GTK_USERS_STORE_H

#include <json/json.h>
#include <boost/optional.hpp>
#include "user.h"

class users_store {
 public:
  users_store(const Json::Value& users);

  boost::optional<user> find(const std::string& user_id) const;

 private:
  std::map<std::string, user> users_;
};

#endif
