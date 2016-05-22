#include "message_entry.h"
#include <gtkmm/liststore.h>
#include <iostream>

MessageEntry::MessageEntry(const api_client& api_client,
                           emoji_loader& emoji_loader,
                           const std::string& channel_id)
    : completion_columns_(),

      api_client_(api_client),
      emoji_loader_(emoji_loader),
      channel_id_(channel_id) {
  Glib::RefPtr<Gtk::EntryCompletion> completion =
      Gtk::EntryCompletion::create();
  set_completion(completion);

  Glib::RefPtr<Gtk::ListStore> list_store =
      Gtk::ListStore::create(completion_columns_);
  for (const auto& p : emoji_loader_.data()) {
    Gtk::TreeRow row = (*list_store->append());
    row[completion_columns_.short_name] = ":" + p.first + ":";
  }
  completion->set_model(list_store);
  completion->set_text_column(completion_columns_.short_name);
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
