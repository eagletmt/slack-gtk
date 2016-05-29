#ifndef SLACK_GTK_MAIN_WINDOW_H
#define SLACK_GTK_MAIN_WINDOW_H

#include <giomm/settings.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/stack.h>
#include "channel_window.h"
#include "team.h"

class MainWindow : public Gtk::ApplicationWindow {
 public:
  MainWindow(std::shared_ptr<api_client> api_client,
             const std::string& emoji_directory, const Json::Value& json);
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
  void on_user_typing_signal(const Json::Value& payload);

  void on_channel_link_clicked(const std::string& channel_id);
  void on_channel_added(Widget* widget);
  void on_channel_unread_count_changed(const std::string& channel_id);
  void on_visible_channel_changed();

  void append_message(const std::string& text);
  ChannelWindow* add_channel_window(const channel& chan);

  Gtk::Stack channels_stack_;
  Glib::RefPtr<Gio::Settings> settings_;

  team team_;
};
#endif
