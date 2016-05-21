#include "main_window.h"
#include <iostream>
#include "channel_window.h"

MainWindow::MainWindow(const api_client& api_client, const Json::Value& json)
    : box_(Gtk::ORIENTATION_HORIZONTAL), rtm_client_(json), users_store_(json) {
  add(box_);

  box_.pack_start(channels_sidebar_, Gtk::PACK_SHRINK);
  box_.pack_start(channels_stack_, Gtk::PACK_EXPAND_WIDGET);

  channels_sidebar_.set_stack(channels_stack_);

  rtm_client_.hello_signal().connect(
      sigc::mem_fun(*this, &MainWindow::on_hello_signal));
  rtm_client_.reconnect_url_signal().connect(
      sigc::mem_fun(*this, &MainWindow::on_reconnect_url_signal));
  rtm_client_.presence_change_signal().connect(
      sigc::mem_fun(*this, &MainWindow::on_presence_change_signal));
  rtm_client_.pref_change_signal().connect(
      sigc::mem_fun(*this, &MainWindow::on_pref_change_signal));
  rtm_client_.message_signal().connect(
      sigc::mem_fun(*this, &MainWindow::on_message_signal));
  rtm_client_.channel_marked_signal().connect(
      sigc::mem_fun(*this, &MainWindow::on_channel_marked_signal));

  for (const Json::Value& channel : json["channels"]) {
    const std::string name = channel["name"].asString();
    auto w = Gtk::manage(new ChannelWindow(api_client, users_store_, channel));
    channels_stack_.add(*w, w->id(), w->name());
  }

  rtm_client_.start();
  show_all_children();
}

MainWindow::~MainWindow() {
}

void MainWindow::on_hello_signal(const Json::Value&) {
  append_message("RTM API started");
}

void MainWindow::on_reconnect_url_signal(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "Receive reconnect_url=" << payload["url"].asString();
  append_message(oss.str());
}

void MainWindow::on_presence_change_signal(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "User " << payload["user"] << " changed presence to "
      << payload["presence"];
  append_message(oss.str());
}
void MainWindow::on_pref_change_signal(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "Changed preference " << payload["name"] << ": " << payload["value"];
  append_message(oss.str());
}

void MainWindow::on_message_signal(const Json::Value& payload) {
  const std::string id = payload["channel"].asString();
  Widget* widget = channels_stack_.get_child_by_name(id);
  if (widget == nullptr) {
    std::cerr << "Unknown channel: id=" << id << std::endl;
    std::cerr << payload << std::endl;
  } else {
    static_cast<ChannelWindow*>(widget)->on_message_signal(payload);
  }
}

void MainWindow::on_channel_marked_signal(const Json::Value& payload) {
  std::ostringstream oss;
  oss << payload;
  append_message(oss.str());
}

void MainWindow::append_message(const std::string& text) {
  std::cout << text << std::endl;
}
