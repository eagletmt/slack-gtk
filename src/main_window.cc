#include "main_window.h"
#include <iostream>
#include "channel_row.h"
#include "message_row.h"

MainWindow::MainWindow(const api_client& api_client, const Json::Value& json)
    : box_(Gtk::ORIENTATION_HORIZONTAL),
      right_box_(Gtk::ORIENTATION_VERTICAL),
      message_entry_(api_client),
      api_client_(api_client),
      rtm_client_(json) {
  add(box_);

  channels_scrolled_window_.set_policy(Gtk::POLICY_AUTOMATIC,
                                       Gtk::POLICY_AUTOMATIC);
  channels_scrolled_window_.add(channels_list_box_);
  box_.pack_start(channels_scrolled_window_);

  box_.pack_start(right_box_);

  messages_scrolled_window_.set_policy(Gtk::POLICY_AUTOMATIC,
                                       Gtk::POLICY_AUTOMATIC);
  messages_scrolled_window_.add(messages_list_box_);
  right_box_.pack_start(messages_scrolled_window_);
  right_box_.pack_end(message_entry_, Gtk::PACK_SHRINK);

  rtm_client_.hello_message().connect(
      sigc::mem_fun(*this, &MainWindow::on_hello_message));
  rtm_client_.reconnect_url_message().connect(
      sigc::mem_fun(*this, &MainWindow::on_reconnect_url_message));
  rtm_client_.presence_change_message().connect(
      sigc::mem_fun(*this, &MainWindow::on_presence_change_message));
  rtm_client_.pref_change_message().connect(
      sigc::mem_fun(*this, &MainWindow::on_pref_change_message));
  rtm_client_.message_message().connect(
      sigc::mem_fun(*this, &MainWindow::on_message_message));
  rtm_client_.channel_marked_message().connect(
      sigc::mem_fun(*this, &MainWindow::on_channel_marked_message));

  for (const Json::Value& channel : json["channels"]) {
    auto row = Gtk::manage(new ChannelRow(channel));
    channels_list_box_.append(*row);
    row->show();
    if (channel["is_general"].asBool()) {
      message_entry_.set_channel_id(channel["id"].asString());
    }
  }

  rtm_client_.start();
  show_all_children();
}

MainWindow::~MainWindow() {
}

void MainWindow::on_hello_message(const Json::Value&) {
  append_message("RTM API started");
}

void MainWindow::on_reconnect_url_message(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "Receive reconnect_url=" << payload["url"].asString();
  append_message(oss.str());
}

void MainWindow::on_presence_change_message(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "User " << payload["user"] << " changed presence to "
      << payload["presence"];
  append_message(oss.str());
}
void MainWindow::on_pref_change_message(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "Changed preference " << payload["name"] << ": " << payload["value"];
  append_message(oss.str());
}

void MainWindow::on_message_message(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "User " << payload["user"] << " sent message to " << payload["channel"]
      << "[ts=" << payload["ts"] << "]: " << payload["text"];
  append_message(oss.str());
}
void MainWindow::on_channel_marked_message(const Json::Value& payload) {
  std::ostringstream oss;
  oss << payload;
  append_message(oss.str());
}

void MainWindow::append_message(const std::string& text) {
  std::cout << text << std::endl;
  auto row = Gtk::manage(new MessageRow(text));
  messages_list_box_.append(*row);
  row->show();
}
