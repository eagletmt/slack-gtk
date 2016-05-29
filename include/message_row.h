#ifndef SLACK_GTK_MESSAGE_ROW_H
#define SLACK_GTK_MESSAGE_ROW_H

#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/listboxrow.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-session.h>
#include <sigc++/sigc++.h>
#include "message_text_view.h"
#include "team.h"

class MessageRow : public Gtk::ListBoxRow {
 public:
  MessageRow(team& team, const Json::Value& payload);
  virtual ~MessageRow();

  std::string summary_for_notification() const;
  const std::string& ts() const;

  sigc::signal<void, const std::string&> signal_user_link_clicked();
  sigc::signal<void, const std::string&> signal_channel_link_clicked();

 private:
  void load_user_icon(const std::string& url);
  void on_user_icon_loaded(Glib::RefPtr<Gdk::Pixbuf> pixbuf);

  Gtk::Image user_image_;
  Gtk::Label user_label_;
  MessageTextView message_text_view_;

  std::string ts_;

  team& team_;
};

#endif
