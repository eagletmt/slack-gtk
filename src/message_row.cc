#include "message_row.h"
#include <gdkmm/pixbufloader.h>
#include <glibmm/datetime.h>
#include <gtkmm/stock.h>
#include <libsoup/soup-uri.h>
#include <iostream>
#include <regex>

MessageRow::MessageRow(const api_client &api_client, icon_loader &icon_loader,
                       const users_store &users_store,
                       const Json::Value &payload)
    : hbox_(Gtk::ORIENTATION_HORIZONTAL),
      vbox_(Gtk::ORIENTATION_VERTICAL),
      info_hbox_(Gtk::ORIENTATION_HORIZONTAL),
      user_image_(Gtk::Stock::MISSING_IMAGE,
                  Gtk::IconSize(Gtk::ICON_SIZE_BUTTON)),
      user_label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),
      timestamp_label_("", Gtk::ALIGN_END, Gtk::ALIGN_CENTER),
      message_label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),
      file_image_(Gtk::Stock::MISSING_IMAGE,
                  Gtk::IconSize(Gtk::ICON_SIZE_BUTTON)),

      api_client_(api_client),
      icon_loader_(icon_loader),
      users_store_(users_store) {
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

  message_label_.set_line_wrap(true);
  message_label_.set_line_wrap_mode(Pango::WRAP_WORD_CHAR);

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
  vbox_.pack_end(message_label_);

  message_label_.set_markup(convert_links(text, is_message));

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

std::string MessageRow::convert_links(const std::string &slack_markup,
                                      bool is_message) const {
  std::regex markup_re("<([^<>]*)>");
  std::sregex_iterator re_it(slack_markup.begin(), slack_markup.end(),
                             markup_re),
      re_end;
  if (re_it == re_end) {
    return slack_markup;
  }

  std::string pango_markup;
  std::size_t pos = 0;
  static const std::string non_message_color = "#aaa";

  for (; re_it != re_end; ++re_it) {
    if (!is_message) {
      pango_markup.append("<span color=\"")
          .append(non_message_color)
          .append("\">");
    }
    pango_markup.append(slack_markup, pos, re_it->position() - pos);
    if (!is_message) {
      pango_markup.append("</span>");
    }
    pango_markup.append(convert_link((*re_it)[1].str()));
    pos = re_it->position() + re_it->length();
  }
  pango_markup.append(slack_markup, pos, slack_markup.size() - pos);

  return pango_markup;
}

// https://api.slack.com/docs/formatting
std::string MessageRow::convert_link(const std::string &linker) const {
  std::regex url_re("^(.+)\\|(.+)$");
  std::smatch match;
  std::string link_id, link_name;

  if (std::regex_match(linker, match, url_re)) {
    const std::string &left = match[1];
    const std::string &right = match[2];
    link_id = left;
    switch (left[0]) {
      case '@':
      case '#':
        link_name = std::string(left, 0, 1).append(right);
        break;
      default:
        link_id = left;
        link_name = right;
        break;
    }
  } else {
    link_id = linker;
    switch (linker[0]) {
      case '@': {
        const boost::optional<user> o_user =
            users_store_.find(linker.substr(1, linker.size() - 1));
        if (o_user) {
          link_name = "@" + o_user.get().name;
        } else {
          std::cerr << "[MessageRow] cannot find linked user " << linker
                    << std::endl;
          link_name = link_id;
        }
      } break;
      case '#':
        // TODO: Resolve channel name
        link_name = link_id;
        break;
      default:
        link_name = link_id;
        break;
    }
  }

  return std::string("<a href=\"")
      .append(link_id)
      .append("\">")
      .append(link_name)
      .append("</a>");
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
