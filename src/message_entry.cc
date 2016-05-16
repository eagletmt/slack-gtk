#include "message_entry.h"
#include <iostream>

MessageEntry::MessageEntry(const api_client& api_client)
    : api_client_(api_client) {
}

MessageEntry::~MessageEntry() {
}

void MessageEntry::on_activate() {
  const Glib::ustring text = get_text();
  std::cout << "Post to " << channel_id_ << ": " << text << std::endl;
  set_text("");
  // TODO: Post with api_client
}

void MessageEntry::set_channel_id(const std::string& channel_id) {
  channel_id_ = channel_id;
}
