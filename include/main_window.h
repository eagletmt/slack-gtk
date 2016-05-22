#ifndef SLACK_GTK_MAIN_WINDOW_H
#define SLACK_GTK_MAIN_WINDOW_H

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/stack.h>
#include <gtkmm/stacksidebar.h>
#include "api_client.h"
#include "channel_window.h"
#include "channels_store.h"
#include "icon_loader.h"
#include "rtm_client.h"
#include "users_store.h"

class MainWindow : public Gtk::ApplicationWindow {
 public:
  MainWindow(const api_client& api_client, const Json::Value& json);
  virtual ~MainWindow();

 private:
  void on_hello_signal(const Json::Value& payload);
  void on_reconnect_url_signal(const Json::Value& payload);
  void on_presence_change_signal(const Json::Value& payload);
  void on_pref_change_signal(const Json::Value& payload);
  void on_message_signal(const Json::Value& payload);
  void on_channel_marked_signal(const Json::Value& payload);
  void on_channel_joined_signal(const Json::Value& payload);
  void on_channel_left_signal(const Json::Value& payload);

  void on_channel_link_clicked(const std::string& channel_id);
  void on_channel_added(Widget* widget);
  void on_channel_unread_count_changed(const std::string& channel_id);

  void append_message(const std::string& text);
  ChannelWindow* add_channel_window(const channel& chan);

  Gtk::Box box_;
  Gtk::StackSidebar channels_sidebar_;
  Gtk::Stack channels_stack_;

  api_client api_client_;
  rtm_client rtm_client_;
  users_store users_store_;
  channels_store channels_store_;
  icon_loader icon_loader_;
};
#endif
