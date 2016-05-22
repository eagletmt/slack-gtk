#include "main_window.h"
#include <iostream>

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

  channels_stack_.signal_add().connect(
      sigc::mem_fun(*this, &MainWindow::on_channel_added));
  for (const auto& p : channels_store_.data()) {
    const channel& chan = p.second;
    if (chan.is_member) {
      add_channel_window(chan);
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
    auto w = add_channel_window(result.get());
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

void MainWindow::on_channel_added(Widget*) {
  std::map<std::string, Widget*> children;
  for (Widget* widget : channels_stack_.get_children()) {
    children.emplace(
        std::make_pair(static_cast<ChannelWindow*>(widget)->name(), widget));
  }
  int position = 0;
  for (auto p : children) {
    channels_stack_.child_property_position(*p.second).set_value(position);
    ++position;
  }
}

static std::string build_channel_title(const ChannelWindow& window) {
  std::ostringstream oss;
  oss << window.name() << " (" << window.unread_count() << ")";
  return oss.str();
}

ChannelWindow* MainWindow::add_channel_window(const channel& chan) {
  auto w = Gtk::manage(
      new ChannelWindow(api_client_, users_store_, icon_loader_, chan));
  w->channel_link_signal().connect(
      sigc::mem_fun(*this, &MainWindow::on_channel_link_clicked));
  w->property_unread_count().signal_changed().connect(sigc::bind(
      sigc::mem_fun(*this, &MainWindow::on_channel_unread_count_changed),
      chan.id));
  channels_stack_.add(*w, w->id(), build_channel_title(*w));
  return w;
}

void MainWindow::on_channel_unread_count_changed(
    const std::string& channel_id) {
  Widget* widget = channels_stack_.get_child_by_name(channel_id);
  if (widget == nullptr) {
    std::cerr
        << "[MainWindow] on_channel_unread_count_changed: unknown channel_id="
        << channel_id << std::endl;
    return;
  }

  ChannelWindow* window = static_cast<decltype(window)>(widget);

  channels_stack_.child_property_title(*window).set_value(
      build_channel_title(*window));
}
