#include "message_row.h"
#include <gdkmm/pixbufloader.h>
#include <gtkmm/stock.h>
#include <libsoup/soup-uri.h>
#include <iostream>

MessageRow::MessageRow(const users_store &users_store,
                       const Json::Value &payload)
    : hbox_(Gtk::ORIENTATION_HORIZONTAL),
      vbox_(Gtk::ORIENTATION_VERTICAL),
      user_image_(Gtk::Stock::MISSING_IMAGE,
                  Gtk::IconSize(Gtk::ICON_SIZE_BUTTON)),
      user_label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),
      message_label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER),
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
      if (image64.isString()) {
        load_user_icon(image64.asString());
      } else if (image48.isString()) {
        load_user_icon(image48.asString());
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
  // TODO: Cache user icons
  SoupSession *session = soup_session_new();
  SoupMessage *message = soup_message_new("GET", icon_url.c_str());
  soup_session_queue_message(session, message, load_user_icon_callback, this);
}

void MessageRow::load_user_icon_callback(SoupSession *session,
                                         SoupMessage *message,
                                         gpointer user_data) {
  static_cast<MessageRow *>(user_data)->on_user_icon_loaded(session, message);
}

void MessageRow::on_user_icon_loaded(SoupSession *session,
                                     SoupMessage *message) {
  if (SOUP_STATUS_IS_TRANSPORT_ERROR(message->status_code)) {
    char *uri = soup_uri_to_string(soup_message_get_uri(message), FALSE);
    std::cerr << "libsoup: " << uri << " (" << message->status_code << ") "
              << soup_status_get_phrase(message->status_code) << std::endl;
    g_free(uri);
  } else {
    Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
    loader->write(
        reinterpret_cast<const guint8 *>(message->response_body->data),
        message->response_body->length);
    loader->close();
    user_image_.set(
        loader->get_pixbuf()->scale_simple(36, 36, Gdk::INTERP_BILINEAR));
  }

  g_object_unref(session);
}
