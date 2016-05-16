#include "main_window.h"
#include <iostream>

ChannelRow::ChannelRow(const Json::Value& channel)
    : label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER) {
  add(label_);

  const std::string id = channel["id"].asString();
  const std::string name = channel["name"].asString();
  label_.set_text("#" + name);

  label_.show();
}

ChannelRow::~ChannelRow() {
}

MessageRow::MessageRow(const std::string& text)
    : label_(text, Gtk::ALIGN_START, Gtk::ALIGN_CENTER) {
  add(label_);
  label_.show();
}

MessageRow::~MessageRow() {
}

MainWindow::MainWindow(const api_client& api_client)
    : box_(Gtk::ORIENTATION_HORIZONTAL),
      right_box_(Gtk::ORIENTATION_VERTICAL),
      message_entry_(api_client) {
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

  show_all_children();
}

MainWindow::~MainWindow() {
  io_service_.stop();
  rtm_client_thread_.join();
}

void MainWindow::start(const Json::Value& rtm) {
  for (const Json::Value& channel : rtm["channels"]) {
    auto row = Gtk::manage(new ChannelRow(channel));
    channels_list_box_.append(*row);
    row->show();
    if (channel["is_general"].asBool()) {
      message_entry_.set_channel_id(channel["id"].asString());
    }
  }

  // TODO: Integrate GTK's main loop with io_service.
  // Don't use threads here.
  rtm_client_thread_ =
      std::thread(std::bind(&MainWindow::rtm_start, this, rtm));
}

void MainWindow::on_hello(const Json::Value&) {
}

void MainWindow::on_reconnect_url(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "Receive reconnect_url=" << payload["url"];
  append_message(oss.str());
}

void MainWindow::on_presence_change(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "User " << payload["user"] << " changed presence to "
      << payload["presence"];
  append_message(oss.str());
}
void MainWindow::on_pref_change(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "Changed preference " << payload["name"] << ": " << payload["value"];
  append_message(oss.str());
}

void MainWindow::on_message(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "User " << payload["user"] << " sent message to " << payload["channel"]
      << "[ts=" << payload["ts"] << "]: " << payload["text"];
  append_message(oss.str());
}
void MainWindow::on_channel_marked(const Json::Value& payload) {
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

void MainWindow::rtm_start(const Json::Value& rtm) {
  using std::placeholders::_1;
  message_handler_.register_handler("hello",
                                    std::bind(&MainWindow::on_hello, this, _1));
  message_handler_.register_handler(
      "reconnect_url", std::bind(&MainWindow::on_reconnect_url, this, _1));
  message_handler_.register_handler(
      "presence_change", std::bind(&MainWindow::on_presence_change, this, _1));
  message_handler_.register_handler(
      "pref_change", std::bind(&MainWindow::on_pref_change, this, _1));
  message_handler_.register_handler(
      "message", std::bind(&MainWindow::on_message, this, _1));
  message_handler_.register_handler(
      "channel_marked", std::bind(&MainWindow::on_channel_marked, this, _1));

  rtm_client rtm_client(io_service_, rtm, message_handler_);
  rtm_client.start();
  io_service_.run();
}
