#ifndef SLACK_GTK_MESSAGE_ROW_H
#define SLACK_GTK_MESSAGE_ROW_H

#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/listboxrow.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-session.h>
#include "icon_loader.h"
#include "users_store.h"

class MessageRow : public Gtk::ListBoxRow {
 public:
  MessageRow(icon_loader& icon_loader, const users_store& users_store,
             const Json::Value& payload);
  virtual ~MessageRow();

 private:
  void load_user_icon(const std::string& url);
  void on_user_icon_loaded(Glib::RefPtr<Gdk::Pixbuf> pixbuf);
  std::string convert_links(const std::string& slack_markup,
                            bool is_message) const;
  std::string convert_link(const std::string& linker) const;

  Gtk::Box hbox_, vbox_, info_hbox_;
  Gtk::Image user_image_;
  Gtk::Label user_label_;
  Gtk::Label timestamp_label_;
  Gtk::Label message_label_;

  icon_loader& icon_loader_;
  const users_store& users_store_;
};

#endif
