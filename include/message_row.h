#ifndef SLACK_GTK_MESSAGE_ROW_H
#define SLACK_GTK_MESSAGE_ROW_H

#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/listboxrow.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-session.h>
#include "users_store.h"

class MessageRow : public Gtk::ListBoxRow {
 public:
  MessageRow(const users_store& users_store, const Json::Value& payload);
  virtual ~MessageRow();

 private:
  void load_user_icon(const std::string& url);
  static void load_user_icon_callback(SoupSession* session,
                                      SoupMessage* message, gpointer user_data);
  void on_user_icon_loaded(SoupSession* session, SoupMessage* message);
  static Gtk::IconSize user_icon_size();

  Gtk::Box hbox_, vbox_;
  Gtk::Image user_image_;
  Gtk::Label user_label_;
  Gtk::Label message_label_;

  const users_store& users_store_;
};

#endif
