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
#include "icon_loader.h"
#include "users_store.h"

class MessageRow : public Gtk::ListBoxRow {
 public:
  MessageRow(const api_client& api_client, icon_loader& icon_loader,
             const users_store& users_store, const Json::Value& payload);
  virtual ~MessageRow();

  std::string summary_for_notification() const;

  sigc::signal<void, const std::string&> channel_link_signal();

 private:
  void load_user_icon(const std::string& url);
  void on_user_icon_loaded(Glib::RefPtr<Gdk::Pixbuf> pixbuf);

  void load_shared_file(const Json::Value& payload);
  void on_shared_file_loaded(Glib::RefPtr<Gdk::Pixbuf> pixbuf);

  bool on_activate_link(const Glib::ustring& uri);

  std::string convert_links(const std::string& slack_markup,
                            bool is_message) const;
  std::string convert_link(const std::string& linker) const;

  Gtk::Box hbox_, vbox_, info_hbox_;
  Gtk::Image user_image_;
  Gtk::Label user_label_;
  Gtk::Label timestamp_label_;
  Gtk::Label message_label_;
  Gtk::Image file_image_;

  api_client api_client_;
  icon_loader& icon_loader_;
  const users_store& users_store_;

  sigc::signal<void, const std::string&> channel_link_signal_;
};

#endif
