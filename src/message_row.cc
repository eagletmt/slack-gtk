#include "message_row.h"
#include <gdkmm/pixbufloader.h>
#include <glibmm/datetime.h>
#include <gtkmm/stock.h>
#include <libsoup/soup-uri.h>
#include <iostream>
#include "attachments_view.h"

MessageRow::MessageRow(team &team, const Json::Value &payload)
    : user_image_(Gtk::Stock::MISSING_IMAGE,
                  Gtk::IconSize(Gtk::ICON_SIZE_BUTTON)),
      user_label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),
      message_text_view_(team),

      ts_(payload["ts"].asString()),

      team_(team) {
  Gtk::Box *hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
  add(*hbox);

  Gtk::Box *vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
  hbox->pack_start(user_image_, Gtk::PACK_SHRINK);
  hbox->pack_end(*vbox);
  user_image_.set_alignment(Gtk::ALIGN_CENTER, Gtk::ALIGN_START);

  Gtk::Box *info_hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
  vbox->pack_start(*info_hbox, Gtk::PACK_SHRINK);

  Gtk::Label *timestamp_label =
      Gtk::manage(new Gtk::Label("", Gtk::ALIGN_END, Gtk::ALIGN_CENTER));
  info_hbox->pack_start(user_label_, Gtk::PACK_SHRINK);
  info_hbox->pack_end(*timestamp_label, Gtk::PACK_SHRINK);

  Pango::AttrList attrs;
  Pango::Attribute weight =
      Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
  attrs.insert(weight);
  user_label_.set_attributes(attrs);

  const Glib::DateTime timestamp =
      Glib::DateTime::create_now_local(gint64(std::stof(ts())));
  timestamp_label->set_text(timestamp.format("%F %R"));

  const Json::Value subtype_value = payload["subtype"];
  std::string text = payload["text"].asString();

  const std::string user_id = payload["user"].asString();
  const boost::optional<user> o_user = team_.users_store_->find(user_id);
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
      std::string username = payload["username"].asString();
      const Json::Value image64 = payload["icons"]["image_64"];
      const Json::Value image48 = payload["icons"]["image_48"];
      const Json::Value bot_id = payload["bot_id"];
      if (image64.isString()) {
        load_user_icon(image64.asString());
      } else if (image48.isString()) {
        load_user_icon(image48.asString());
      } else if (bot_id.isString()) {
        const boost::optional<user> ou =
            team_.users_store_->find(bot_id.asString());
        if (ou) {
          const user &u = ou.get();
          load_user_icon(u.icons.image_72);
          if (username.empty()) {
            username = u.name;
          }
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
      user_label_.set_text(username);
    } else if (subtype == "channel_join") {
      const Json::Value inviter_value = payload["inviter"];
      if (inviter_value.isString()) {
        const boost::optional<user> o_inviter =
            team_.users_store_->find(inviter_value.asString());
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
      // nothing special
    } else {
      std::cout << "Unhandled subtype " << subtype << ": \n"
                << payload << std::endl;
    }
  }
  if (payload["attachments"].isArray()) {
    auto attachments_view =
        Gtk::manage(new AttachmentsView(team_, payload["attachments"]));
    vbox->pack_end(*attachments_view);
  }
  vbox->pack_end(message_text_view_);
  message_text_view_.set_text(text, is_message);

  show_all_children();
}

MessageRow::~MessageRow() {
}

void MessageRow::load_user_icon(const std::string &icon_url) {
  team_.icon_loader_->load(icon_url, std::bind(&MessageRow::on_user_icon_loaded,
                                               this, std::placeholders::_1));
}

void MessageRow::on_user_icon_loaded(Glib::RefPtr<Gdk::Pixbuf> pixbuf) {
  user_image_.set(pixbuf->scale_simple(36, 36, Gdk::INTERP_BILINEAR));
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

const std::string &MessageRow::ts() const {
  return ts_;
}
