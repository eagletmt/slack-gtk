#include "message_row.h"
#include <iostream>

MessageRow::MessageRow(const users_store& users_store,
                       const Json::Value& payload)
    : vbox_(Gtk::ORIENTATION_VERTICAL),
      user_label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),
      message_label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),
      users_store_(users_store) {
  add(vbox_);

  vbox_.pack_start(user_label_, Gtk::PACK_SHRINK);
  vbox_.pack_end(message_label_);

  Pango::AttrList attrs;
  Pango::Attribute weight =
      Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
  attrs.insert(weight);
  user_label_.set_attributes(attrs);

  message_label_.set_line_wrap(true);
  message_label_.set_line_wrap_mode(Pango::WRAP_WORD_CHAR);

  const Json::Value subtype_value = payload["subtype"];

  if (subtype_value.isNull()) {
    const std::string user_id = payload["user"].asString();
    boost::optional<user> ou = users_store_.find(user_id);
    if (ou) {
      user_label_.set_text(ou.get().name);
    } else {
      std::cerr << "[MessageRow] cannot find user " << user_id;
      user_label_.set_text(user_id);
    }
  } else {
    const std::string subtype = subtype_value.asString();
    if (subtype == "bot_message") {
      user_label_.set_text(payload["username"].asString() + " [BOT]");
    } else {
      std::cout << "subtype " << subtype << ": \n" << payload << std::endl;
    }
  }

  message_label_.set_text(payload["text"].asString());

  show_all_children();
}

MessageRow::~MessageRow() {
}
