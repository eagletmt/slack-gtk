#ifndef SLACK_CPP_MAIN_WINDOW_H
#define SLACK_CPP_MAIN_WINDOW_H

#include <gtkmm.h>
#include <json/json.h>
#include <thread>
#include "api_client.h"
#include "message_entry.h"
#include "rtm_client.h"

class ChannelRow : public Gtk::ListBoxRow {
 public:
  ChannelRow(const Json::Value& channel);
  virtual ~ChannelRow();

 private:
  Gtk::Label label_;
};

class MessageRow : public Gtk::ListBoxRow {
 public:
  MessageRow(const std::string& message);
  virtual ~MessageRow();

 private:
  Gtk::Label label_;
};

class MainWindow : public Gtk::Window {
 public:
  MainWindow(const api_client& api_client);
  virtual ~MainWindow();

  void start(const Json::Value& rtm);

 private:
  void rtm_start(const Json::Value& rtm);
  void append_message(const std::string& text);

  void on_hello(const Json::Value& payload);
  void on_reconnect_url(const Json::Value& payload);
  void on_presence_change(const Json::Value& payload);
  void on_pref_change(const Json::Value& payload);
  void on_message(const Json::Value& payload);
  void on_channel_marked(const Json::Value& payload);

  Gtk::Box box_;
  Gtk::ScrolledWindow channels_scrolled_window_;
  Gtk::ListBox channels_list_box_;
  Gtk::ScrolledWindow messages_scrolled_window_;
  Gtk::ListBox messages_list_box_;
  Gtk::Box right_box_;
  MessageEntry message_entry_;

  boost::asio::io_service io_service_;
  std::thread rtm_client_thread_;
  message_handler message_handler_;
};

#endif
