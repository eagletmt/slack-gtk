#include "message_entry.h"
#include <iostream>

MessageEntry::MessageEntry(const api_client& api_client)
    : api_client_(api_client) {
}

MessageEntry::~MessageEntry() {
}

void MessageEntry::on_activate() {
  const Glib::ustring text = get_text();
  set_text("");
  std::cout << "Post to " << channel_id_ << ": " << text << std::endl;
  std::map<std::string, std::string> params;
  params["channel"] = channel_id_;
  params["text"] = text.raw();
  params["as_user"] = "true";
  auto result = api_client_.post("chat.postMessage", params);
  if (result) {
    std::cout << result.get() << std::endl;
  }
}

void MessageEntry::set_channel_id(const std::string& channel_id) {
  channel_id_ = channel_id;
}
