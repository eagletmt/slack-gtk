#include "message_entry.h"
#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>
#include <iostream>

MessageEntry::MessageEntry(const api_client& api_client,
                           const std::string& channel_id)
    : api_client_(api_client), channel_id_(channel_id) {
  set_wrap_mode(Gtk::WRAP_WORD);
  Glib::RefPtr<Gtk::CssProvider> provider = Gtk::CssProvider::create();
  provider->load_from_resource("/cc/wanko/slack-gtk/message_entry.css");
  get_style_context()->add_provider(provider,
                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

MessageEntry::~MessageEntry() {
}

bool MessageEntry::on_key_press_event(GdkEventKey* event) {
  switch (event->keyval) {
    case GDK_KEY_Return:
    case GDK_KEY_KP_Enter:
      return on_enter();
  }
  return Gtk::TextView::on_key_press_event(event);
}

bool MessageEntry::on_enter() {
  Glib::RefPtr<Gtk::TextBuffer> buffer = get_buffer();
  const Glib::ustring text = buffer->get_text();
  buffer->set_text("");
  std::map<std::string, std::string> params;
  params["channel"] = channel_id_;
  params["text"] = text.raw();
  params["as_user"] = "true";
  params["parse"] = "full";
  api_client_.queue_post("chat.postMessage", params,
                         std::bind(&MessageEntry::post_message_finished, this,
                                   std::placeholders::_1));
  return true;
}

void MessageEntry::post_message_finished(
    const boost::optional<Json::Value>& result) const {
  if (result) {
    std::cout << result.get() << std::endl;
  } else {
    std::cerr << "[MessageEntry] failed to post message" << std::endl;
  }
}
