#ifndef SLACK_GTK_MAIN_WINDOW_H
#define SLACK_GTK_MAIN_WINDOW_H

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include "api_client.h"
#include "message_entry.h"
#include "rtm_client.h"

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

  void append_message(const std::string& text);

  Gtk::Box box_;
  Gtk::ScrolledWindow channels_scrolled_window_;
  Gtk::ListBox channels_list_box_;
  Gtk::ScrolledWindow messages_scrolled_window_;
  Gtk::ListBox messages_list_box_;
  Gtk::Box right_box_;
  MessageEntry message_entry_;

  api_client api_client_;
  rtm_client rtm_client_;
};
#endif
