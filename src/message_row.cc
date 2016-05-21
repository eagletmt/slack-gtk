#include "message_row.h"
#include <iostream>

MessageRow::MessageRow(const users_store& users_store,
                       const Json::Value& payload)
    : label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),
      users_store_(users_store) {
  add(label_);
  label_.set_line_wrap(true);
  label_.set_line_wrap_mode(Pango::WRAP_WORD_CHAR);
  label_.set_text(build_text(payload));
  label_.show();
}

MessageRow::~MessageRow() {
}

std::string MessageRow::build_text(const Json::Value& payload) {
  std::ostringstream oss;
  const Json::Value subtype_value = payload["subtype"];

  if (subtype_value.isNull()) {
    boost::optional<user> ou = users_store_.find(payload["user"].asString());
    if (ou) {
      const user& u = ou.get();
      oss << "User " << u.name << " sent message " << payload["text"];
    } else {
      oss << "User " << payload["user"] << " sent message " << payload["text"];
    }
  } else {
    const std::string subtype = subtype_value.asString();
    if (subtype == "bot_message") {
      oss << "Bot " << payload["username"] << " (bot_id=" << payload["bot_id"]
          << ") sent message " << payload["text"];
    } else {
      oss << "subtype " << subtype << ": " << payload["text"];
    }
  }

  return oss.str();
}
