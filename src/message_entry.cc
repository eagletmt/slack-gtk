#include "message_entry.h"
#include <iostream>

MessageEntry::MessageEntry(const api_client& api_client,
                           const std::string& channel_id)
    : api_client_(api_client), channel_id_(channel_id) {
}

MessageEntry::~MessageEntry() {
}

void MessageEntry::on_activate() {
  const Glib::ustring text = get_text();
  set_text("");
  std::map<std::string, std::string> params;
  params["channel"] = channel_id_;
  params["text"] = text.raw();
  params["as_user"] = "true";
  params["parse"] = "full";
  api_client_.queue_post("chat.postMessage", params,
                         std::bind(&MessageEntry::post_message_finished, this,
                                   std::placeholders::_1));
}

void MessageEntry::post_message_finished(
    const boost::optional<Json::Value>& result) const {
  if (result) {
    std::cout << result.get() << std::endl;
  } else {
    std::cerr << "[MessageEntry] failed to post message" << std::endl;
  }
}
