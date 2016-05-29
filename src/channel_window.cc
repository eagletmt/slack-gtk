#include "channel_window.h"
#include <gtkmm/scrollbar.h>
#include <gtkmm/scrolledwindow.h>
#include <libnotify/notification.h>
#include <chrono>
#include <iostream>
#include "bottom_adjustment.h"
#include "message_entry.h"
#include "message_row.h"

ChannelWindow::ChannelWindow(team& team, Glib::RefPtr<Gio::Settings> settings,
                             const channel& chan)
    : Glib::ObjectBase(typeid(ChannelWindow)),
      Gtk::Box(),
      settings_(settings),
      messages_list_box_(),
      unread_count_(*this, "unread-count", chan.unread_count),
      history_loaded_(false),

      id_(chan.id),
      name_(chan.name),
      team_(team) {
  set_orientation(Gtk::ORIENTATION_VERTICAL);
  Gtk::ScrolledWindow* messages_scrolled_window =
      Gtk::manage(new Gtk::ScrolledWindow());
  pack_start(*messages_scrolled_window);
  pack_end(*Gtk::manage(new MessageEntry(*team_.api_client_, chan.id)),
           Gtk::PACK_SHRINK);

  messages_scrolled_window->add(messages_list_box_);
  messages_scrolled_window->set_policy(Gtk::POLICY_NEVER,
                                       Gtk::POLICY_AUTOMATIC);
  messages_scrolled_window->set_vadjustment(
      BottomAdjustment::create(messages_scrolled_window->get_vadjustment()));

  messages_list_box_.set_selection_mode(Gtk::SELECTION_NONE);

  show_all_children();
}

const std::string& ChannelWindow::id() const {
  return id_;
}
const std::string& ChannelWindow::name() const {
  return name_;
}

void ChannelWindow::load_history() {
  if (history_loaded_) {
    return;
  }

  // TODO: Show loading indicator
  std::map<std::string, std::string> params;
  params.emplace(std::make_pair("channel", id()));
  const MessageRow* row =
      static_cast<decltype(row)>(messages_list_box_.get_row_at_index(0));
  if (row != nullptr) {
    params["latest"] = row->ts();
  }
  team_.api_client_->queue_post("channels.history", params,
                                std::bind(&ChannelWindow::on_channels_history,
                                          this, std::placeholders::_1));
}

void ChannelWindow::on_message_signal(const Json::Value& payload) {
  MessageRow* row = append_message(payload);
  if (!is_visible() || !get_child_visible()) {
    unread_count_.set_value(unread_count() + 1);
  }
  send_notification(row);
}

void ChannelWindow::on_channel_marked(const Json::Value& payload) {
  unread_count_.set_value(payload["unread_count"].asInt());
}

MessageRow* ChannelWindow::append_message(const Json::Value& payload) {
  auto row = Gtk::manage(new MessageRow(team_, payload));
  messages_list_box_.append(*row);
  row->signal_channel_link_clicked().connect(
      sigc::mem_fun(*this, &ChannelWindow::on_channel_link_clicked));
  row->show();
  return row;
}

MessageRow* ChannelWindow::prepend_message(const Json::Value& payload) {
  auto row = Gtk::manage(new MessageRow(team_, payload));
  messages_list_box_.prepend(*row);
  row->signal_channel_link_clicked().connect(
      sigc::mem_fun(*this, &ChannelWindow::on_channel_link_clicked));
  row->show();
  return row;
}

void ChannelWindow::send_notification(const MessageRow* row) const {
  const std::string title = "slack-gtk #" + name();
  NotifyNotification* notification = notify_notification_new(
      title.c_str(), row->summary_for_notification().c_str(), nullptr);
  notify_notification_set_timeout(notification,
                                  settings_->get_uint("notification-timeout"));
  notify_notification_set_urgency(notification, NOTIFY_URGENCY_LOW);

  GError* error = nullptr;
  if (!notify_notification_show(notification, &error)) {
    std::cerr << "[ChannelWindow] send_notification: " << error->message
              << std::endl;
  }
}

void ChannelWindow::on_channels_history(
    const boost::optional<Json::Value>& result) {
  if (result) {
    for (const Json::Value& message : result.get()["messages"]) {
      prepend_message(message);
    }
    history_loaded_ = true;
  } else {
    std::cerr << "[channel " << name()
              << "] failed to load history from channels.history API"
              << std::endl;
  }
}

sigc::signal<void, const std::string&> ChannelWindow::channel_link_signal() {
  return channel_link_signal_;
}

void ChannelWindow::on_channel_link_clicked(const std::string& channel_id) {
  channel_link_signal_.emit(channel_id);
}

Glib::PropertyProxy<int> ChannelWindow::property_unread_count() {
  return unread_count_.get_proxy();
}

int ChannelWindow::unread_count() const {
  return unread_count_.get_value();
}

void ChannelWindow::on_channel_visible() {
  load_history();
  const std::uint64_t ts =
      std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  mark_as_read(std::to_string(ts));
}

static void channels_mark_finished(const boost::optional<Json::Value>& result) {
  if (result) {
    if (!result.get()["ok"].asBool()) {
      std::cerr << "[ChannelWindow] channels_mark_finished: failed: "
                << result.get() << std::endl;
    }
  } else {
    std::cerr << "[ChannelWindow] channels_mark_finished: failed" << std::endl;
  }
}

void ChannelWindow::mark_as_read(const std::string& ts) {
  std::map<std::string, std::string> params;
  params["channel"] = id();
  params["ts"] = ts;
  team_.api_client_->queue_post("channels.mark", params,
                                channels_mark_finished);
}
