#include "message_row.h"
#include <gdkmm/pixbufloader.h>
#include <glibmm/datetime.h>
#include <gtkmm/stock.h>
#include <libsoup/soup-uri.h>
#include <iostream>

MessageRow::MessageRow(const api_client &api_client, icon_loader &icon_loader,
                       const users_store &users_store,
                       const channels_store &channels_store,
                       const Json::Value &payload)
    : hbox_(Gtk::ORIENTATION_HORIZONTAL),
      vbox_(Gtk::ORIENTATION_VERTICAL),
      info_hbox_(Gtk::ORIENTATION_HORIZONTAL),
      user_image_(Gtk::Stock::MISSING_IMAGE,
                  Gtk::IconSize(Gtk::ICON_SIZE_BUTTON)),
      user_label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),
      timestamp_label_("", Gtk::ALIGN_END, Gtk::ALIGN_CENTER),
      message_text_view_(users_store, channels_store),
      file_image_(Gtk::Stock::MISSING_IMAGE,
                  Gtk::IconSize(Gtk::ICON_SIZE_BUTTON)),

      api_client_(api_client),
      icon_loader_(icon_loader),
      users_store_(users_store),
      channels_store_(channels_store) {
  add(hbox_);

  hbox_.pack_start(user_image_, Gtk::PACK_SHRINK);
  hbox_.pack_end(vbox_);

  vbox_.pack_start(info_hbox_, Gtk::PACK_SHRINK);

  info_hbox_.pack_start(user_label_, Gtk::PACK_SHRINK);
  info_hbox_.pack_end(timestamp_label_, Gtk::PACK_SHRINK);

  Pango::AttrList attrs;
  Pango::Attribute weight =
      Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
  attrs.insert(weight);
  user_label_.set_attributes(attrs);

  const double ts = std::stof(payload["ts"].asString());
  const Glib::DateTime timestamp = Glib::DateTime::create_now_local(gint64(ts));
  timestamp_label_.set_text(timestamp.format("%F %R"));

  const Json::Value subtype_value = payload["subtype"];
  std::string text = payload["text"].asString();

  const std::string user_id = payload["user"].asString();
  const boost::optional<user> o_user = users_store_.find(user_id);
  if (o_user) {
    const user &user = o_user.get();
    user_label_.set_text(user.name);
    load_user_icon(user.profile.image_72);
  }

  bool is_message = false;

  if (subtype_value.isNull()) {
    is_message = true;
  } else {
    const std::string subtype = subtype_value.asString();
    if (subtype == "bot_message") {
      is_message = true;
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
          load_user_icon(ou.get().icons.image_72);
        } else {
          std::cerr << "[MessageRow] cannot find bot user  " << bot_id
                    << std::endl;
        }
      } else {
        const std::string default_icon_url =
            "https://i0.wp.com/slack-assets2.s3-us-west-2.amazonaws.com/8390/"
            "img/avatars/ava_0002-48.png";
        load_user_icon(default_icon_url);
      }
    } else if (subtype == "channel_join") {
      const Json::Value inviter_value = payload["inviter"];
      if (inviter_value.isString()) {
        const boost::optional<user> o_inviter =
            users_store_.find(inviter_value.asString());
        if (o_inviter) {
          const user &inviter = o_inviter.get();
          text.append(" by invitation from <@")
              .append(inviter.id)
              .append("|")
              .append(inviter.name)
              .append(">");

        } else {
          std::cerr << "[MessageRow] cannot find channel_join inviter "
                    << inviter_value << std::endl;
        }
      }
    } else if (subtype == "channel_leave") {
      // nothing special
    } else if (subtype == "bot_add") {
      // nothing special
    } else if (subtype == "bot_remove") {
      // nothing special
    } else if (subtype == "file_share") {
      vbox_.pack_end(file_image_, Gtk::PACK_SHRINK);
      load_shared_file(payload);
    } else {
      std::cout << "Unhandled subtype " << subtype << ": \n"
                << payload << std::endl;
    }
  }
  vbox_.pack_end(message_text_view_);
  message_text_view_.set_text(text, is_message);

  show_all_children();
}

MessageRow::~MessageRow() {
}

void MessageRow::load_user_icon(const std::string &icon_url) {
  icon_loader_.load(icon_url, std::bind(&MessageRow::on_user_icon_loaded, this,
                                        std::placeholders::_1));
}

void MessageRow::on_user_icon_loaded(Glib::RefPtr<Gdk::Pixbuf> pixbuf) {
  user_image_.set(pixbuf->scale_simple(36, 36, Gdk::INTERP_BILINEAR));
}

void MessageRow::load_shared_file(const Json::Value &payload) {
  // TODO: Animated GIF support
  const Json::Value thumb_url = payload["file"]["thumb_360"];
  if (thumb_url.isString()) {
    api_client_.get_shared_file(thumb_url.asString(),
                                std::bind(&MessageRow::on_shared_file_loaded,
                                          this, std::placeholders::_1));
  } else {
    std::cerr << "[MessageRow] Invalid file_share payload " << payload
              << std::endl;
  }
}

void MessageRow::on_shared_file_loaded(Glib::RefPtr<Gdk::Pixbuf> pixbuf) {
  file_image_.set(pixbuf);
}

sigc::signal<void, const std::string &>
MessageRow::signal_channel_link_clicked() {
  return message_text_view_.signal_channel_link_clicked();
}

sigc::signal<void, const std::string &> MessageRow::signal_user_link_clicked() {
  return message_text_view_.signal_user_link_clicked();
}

std::string MessageRow::summary_for_notification() const {
  return user_label_.get_text() + ": " + message_text_view_.get_text();
}
