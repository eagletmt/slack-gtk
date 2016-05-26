#ifndef SLACK_GTK_MESSAGE_ROW_H
#define SLACK_GTK_MESSAGE_ROW_H

#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/listboxrow.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-session.h>
#include <sigc++/sigc++.h>
#include "api_client.h"
#include "channels_store.h"
#include "emoji_loader.h"
#include "icon_loader.h"
#include "message_text_view.h"
#include "users_store.h"

class MessageRow : public Gtk::ListBoxRow {
 public:
  MessageRow(const api_client& api_client, icon_loader& icon_loader,
             emoji_loader& emoji_loader, const users_store& users_store,
             const channels_store& channels_store, const Json::Value& payload);
  virtual ~MessageRow();

  std::string summary_for_notification() const;

  sigc::signal<void, const std::string&> signal_user_link_clicked();
  sigc::signal<void, const std::string&> signal_channel_link_clicked();

 private:
  void load_user_icon(const std::string& url);
  void on_user_icon_loaded(Glib::RefPtr<Gdk::Pixbuf> pixbuf);

  Gtk::Box hbox_, vbox_, info_hbox_;
  Gtk::Image user_image_;
  Gtk::Label user_label_;
  Gtk::Label timestamp_label_;
  MessageTextView message_text_view_;

  api_client api_client_;
  icon_loader& icon_loader_;
  const users_store& users_store_;
  const channels_store& channels_store_;
};

#endif
