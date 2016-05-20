#include "channel_window.h"
#include "message_row.h"

ChannelWindow::ChannelWindow(const api_client& api_client,
                             const Json::Value& channel)
    : box_(Gtk::ORIENTATION_VERTICAL),
      messages_list_box_(),
      message_entry_(api_client, channel["id"].asString()),

      id_(channel["id"].asString()),
      name_(channel["name"].asString()) {
  set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

  add(box_);

  box_.pack_start(messages_list_box_);
  box_.pack_end(message_entry_, Gtk::PACK_SHRINK);

  show_all_children();
}

const std::string& ChannelWindow::id() const {
  return id_;
}
const std::string& ChannelWindow::name() const {
  return name_;
}

void ChannelWindow::on_message_signal(const Json::Value& payload) {
  std::ostringstream oss;
  oss << "User " << payload["user"] << " sent message "
      << "[ts=" << payload["ts"] << "]: " << payload["text"];

  auto row = Gtk::manage(new MessageRow(oss.str()));
  messages_list_box_.append(*row);
  row->show();
}
