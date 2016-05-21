#ifndef SLACK_GTK_CHANNEL_WINDOW_H
#define SLACK_GTK_CHANNEL_WINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include <json/json.h>
#include "api_client.h"
#include "message_entry.h"
#include "users_store.h"

class ChannelWindow : public Gtk::Box {
 public:
  ChannelWindow(const api_client& api_client, const users_store& users_store,
                const Json::Value& channel);

  const std::string& id() const;
  const std::string& name() const;

  void on_message_signal(const Json::Value& payload);
  void on_channels_history(const boost::optional<Json::Value>& result);

 private:
  Gtk::ScrolledWindow messages_scrolled_window_;
  Gtk::ListBox messages_list_box_;
  MessageEntry message_entry_;

  std::string id_;
  std::string name_;
  api_client api_client_;
  const users_store& users_store_;
};

#endif
