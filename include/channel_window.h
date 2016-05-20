#ifndef SLACK_GTK_CHANNEL_WINDOW_H
#define SLACK_GTK_CHANNEL_WINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include <json/json.h>
#include "api_client.h"
#include "message_entry.h"

class ChannelWindow : public Gtk::ScrolledWindow {
 public:
  ChannelWindow(const api_client& api_client, const Json::Value& channel);

  const std::string& id() const;
  const std::string& name() const;

  void on_message_signal(const Json::Value& payload);
  void on_channels_history(
      const boost::optional<Json::Value>& result);

 private:
  Gtk::Box box_;
  Gtk::ListBox messages_list_box_;
  MessageEntry message_entry_;

  std::string id_;
  std::string name_;
  api_client api_client_;
};

#endif
