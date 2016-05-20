#include "channel_window.h"
#include "message_row.h"
#include <iostream>

ChannelWindow::ChannelWindow(const api_client& api_client,
                             const Json::Value& channel)
    : box_(Gtk::ORIENTATION_VERTICAL),
      messages_list_box_(),
      message_entry_(api_client, channel["id"].asString()),

      id_(channel["id"].asString()),
      name_(channel["name"].asString()),
      api_client_(api_client) {
  set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

  add(box_);

  box_.pack_start(messages_list_box_);
  box_.pack_end(message_entry_, Gtk::PACK_SHRINK);

  show_all_children();

  std::map<std::string, std::string> params;
  params.insert(std::make_pair("channel", id()));
  api_client_.queue_post("channels.history", params, std::bind(&ChannelWindow::on_channels_history, this, std::placeholders::_1));
}

const std::string& ChannelWindow::id() const {
  return id_;
}
const std::string& ChannelWindow::name() const {
  return name_;
}

void ChannelWindow::on_message_signal(const Json::Value& payload) {
  std::ostringstream oss;
  const Json::Value subtype_value = payload["subtype"];

  if (subtype_value.isNull()) {
    oss << "User " << payload["user"] << " sent message " << payload["text"];
  } else {
    const std::string subtype = subtype_value.asString();
    if (subtype == "bot_message") {
      oss << "Bot " << payload["username"] << " (bot_id=" << payload["bot_id"] << ") sent message " << payload["text"];
    } else {
      oss << "subtype " << subtype << ": " << payload["text"];
    }
  }

  auto row = Gtk::manage(new MessageRow(oss.str()));
  messages_list_box_.append(*row);
  row->show();
}

void ChannelWindow::on_channels_history(const boost::optional<Json::Value>& result) {
  if (result) {
    std::vector<Json::Value> v;
    for (const Json::Value& message : result.get()["messages"]) {
      v.push_back(message);
    }
    for (auto it = v.crbegin(); it != v.crend(); ++it) {
      on_message_signal(*it);
    }
  } else {
    std::cerr << "[channel " << name() << "] failed to load history from channels.history API" << std::endl;
  }
}
