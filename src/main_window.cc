#include "main_window.h"
#include <iostream>
#include "channel_window.h"

MainWindow::MainWindow(const api_client& api_client, const Json::Value& json)
    : box_(Gtk::ORIENTATION_HORIZONTAL),
      api_client_(api_client),
      rtm_client_(json),
      users_store_(json),
      channels_store_(json),
      // TODO: Use proper directory
      icon_loader_("icons") {
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
  rtm_client_.channel_joined_signal().connect(
      sigc::mem_fun(*this, &MainWindow::on_channel_joined_signal));
  rtm_client_.channel_left_signal().connect(
      sigc::mem_fun(*this, &MainWindow::on_channel_left_signal));

  for (const auto& p : channels_store_.data()) {
    const channel& chan = p.second;
    if (chan.is_member) {
      auto w = Gtk::manage(
          new ChannelWindow(api_client, users_store_, icon_loader_, chan));
      w->channel_link_signal().connect(
          sigc::mem_fun(*this, &MainWindow::on_channel_link_clicked));
      channels_stack_.add(*w, w->id(), w->name());
    }
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

void MainWindow::on_channel_link_clicked(const std::string& channel_id) {
  Widget* widget = channels_stack_.get_child_by_name(channel_id);
  if (widget == nullptr) {
    const boost::optional<channel> result = channels_store_.find(channel_id);
    if (result) {
      std::map<std::string, std::string> params;
      const std::string& name = result.get().name;
      params.emplace(std::make_pair("name", name));
      const boost::optional<Json::Value> join_result =
          api_client_.post("channels.join", params);
      if (join_result) {
        const Json::Value& join_response = join_result.get();
        if (!join_response["ok"].asBool()) {
          std::cerr << "[MainWindow] on_channel_link_clicked: Failed to join #"
                    << name << ": " << join_response << std::endl;
        }
      } else {
        std::cerr << "[MainWindow] on_channel_link_clicked: Failed to join #"
                  << name << std::endl;
      }
    } else {
      std::cerr << "[MainWindow] on_channel_link_clicked: Unknown channel "
                << channel_id << std::endl;
    }
  } else {
    channels_stack_.set_visible_child(channel_id);
  }
}

void MainWindow::on_channel_joined_signal(const Json::Value& payload) {
  const std::string channel_id = payload["channel"]["id"].asString();
  const boost::optional<channel> result = channels_store_.find(channel_id);
  if (result) {
    auto w = Gtk::manage(new ChannelWindow(api_client_, users_store_,
                                           icon_loader_, result.get()));
    w->channel_link_signal().connect(
        sigc::mem_fun(*this, &MainWindow::on_channel_link_clicked));
    channels_stack_.add(*w, w->id(), w->name());
    w->show();
    channels_stack_.set_visible_child(*w);
  } else {
    std::cerr << "[MainWindow] on_channel_joined_signal: Unknown channel "
              << channel_id << std::endl;
  }
}

void MainWindow::on_channel_left_signal(const Json::Value& payload) {
  const std::string channel_id(payload["channel"].asString());

  Widget* widget = channels_stack_.get_child_by_name(channel_id);
  if (widget == nullptr) {
    std::cerr << "[MainWindow] on_channel_left_signal: Cannot find "
                 "ChannelWindow with id="
              << channel_id << std::endl;
  } else {
    channels_stack_.remove(*widget);
    delete widget;
  }
}
