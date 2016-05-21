#include "message_row.h"
#include <gdkmm/pixbufloader.h>
#include <gtkmm/stock.h>
#include <libsoup/soup-uri.h>
#include <iostream>

MessageRow::MessageRow(icon_loader &icon_loader, const users_store &users_store,
                       const Json::Value &payload)
    : hbox_(Gtk::ORIENTATION_HORIZONTAL),
      vbox_(Gtk::ORIENTATION_VERTICAL),
      user_image_(Gtk::Stock::MISSING_IMAGE,
                  Gtk::IconSize(Gtk::ICON_SIZE_BUTTON)),
      user_label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),
      message_label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),

      icon_loader_(icon_loader),
      users_store_(users_store) {
  add(hbox_);

  hbox_.pack_start(user_image_, Gtk::PACK_SHRINK);
  hbox_.pack_end(vbox_);

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
      const user &u = ou.get();
      user_label_.set_text(u.name);
      load_user_icon(u.profile.image_72);
    } else {
      std::cerr << "[MessageRow] cannot find user " << user_id;
      user_label_.set_text(user_id);
    }
  } else {
    const std::string subtype = subtype_value.asString();
    if (subtype == "bot_message") {
      user_label_.set_text(payload["username"].asString() + " [BOT]");
      const Json::Value image64 = payload["icons"]["image_64"];
      const Json::Value image48 = payload["icons"]["image_48"];
      const Json::Value bot_id = payload["bot_id"];
      if (image64.isString()) {
        load_user_icon(image64.asString());
      } else if (image48.isString()) {
        load_user_icon(image48.asString());
      } else if (bot_id.isString()) {
        const boost::optional<user> ou = users_store_.find(bot_id.asString());
        if (ou) {
          load_user_icon(ou.get().profile.image_72);
        } else {
          std::cerr << "[MessageRow] cannot find bot user  " << bot_id
                    << std::endl;
        }
      } else {
        std::cerr << "[MessageRow] cannot load bot icon: " << payload
                  << std::endl;
      }
    } else {
      std::cout << "subtype " << subtype << ": \n" << payload << std::endl;
    }
  }

  message_label_.set_text(payload["text"].asString());

  show_all_children();
}

MessageRow::~MessageRow() {
}

void MessageRow::load_user_icon(const std::string &icon_url) {
  if (icon_url.empty()) {
    std::cerr << "Invalid icon_url!" << std::endl;
    return;
  }
  icon_loader_.load(icon_url, std::bind(&MessageRow::on_user_icon_loaded, this,
                                        std::placeholders::_1));
}

void MessageRow::on_user_icon_loaded(Glib::RefPtr<Gdk::Pixbuf> pixbuf) {
  user_image_.set(pixbuf->scale_simple(36, 36, Gdk::INTERP_BILINEAR));
}
