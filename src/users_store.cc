#include "users_store.h"

users_store::users_store(const Json::Value& users) {
  for (const Json::Value& u : users) {
    users_.insert(std::make_pair(u["id"].asString(), user(u)));
  }
}

boost::optional<user> users_store::find(const std::string& id) const {
  auto it = users_.find(id);
  if (it == users_.cend()) {
    return boost::optional<user>();
  } else {
    return boost::make_optional(it->second);
  }
}
