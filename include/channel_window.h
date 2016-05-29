#ifndef SLACK_GTK_CHANNEL_WINDOW_H
#define SLACK_GTK_CHANNEL_WINDOW_H

#include <giomm/settings.h>
#include <glibmm/property.h>
#include <gtkmm/box.h>
#include <gtkmm/listbox.h>
#include <json/json.h>
#include "channel.h"
#include "team.h"

class MessageRow;

class ChannelWindow : public Gtk::Box {
 public:
  ChannelWindow(team& team, Glib::RefPtr<Gio::Settings> settings,
                const channel& chan);

  const std::string& id() const;
  const std::string& name() const;
  sigc::signal<void, const std::string&> channel_link_signal();

  Glib::PropertyProxy<int> property_unread_count();
  int unread_count() const;

  void mark_as_read(const std::string& ts);
  void load_history();

  void on_message_signal(const Json::Value& payload);
  void on_channel_marked(const Json::Value& payload);
  MessageRow* append_message(const Json::Value& payload);
  MessageRow* prepend_message(const Json::Value& payload);
  void on_channels_history(const boost::optional<Json::Value>& result);
  void on_channel_link_clicked(const std::string& channel_id);
  void on_channel_visible();

 private:
  void send_notification(const MessageRow* row) const;

  Glib::RefPtr<Gio::Settings> settings_;
  Gtk::ListBox messages_list_box_;

  Glib::Property<int> unread_count_;
  bool history_loaded_;

  std::string id_;
  std::string name_;
  team& team_;

  sigc::signal<void, const std::string&> channel_link_signal_;
};

#endif
