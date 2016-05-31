#ifndef SLACK_GTK_MESSAGE_ENTRY_H
#define SLACK_GTK_MESSAGE_ENTRY_H

#include <gtkmm/textview.h>
#include "api_client.h"

class MessageEntry : public Gtk::TextView {
 public:
  MessageEntry(const api_client& api_client, const std::string& channel_id);
  virtual ~MessageEntry();

 protected:
  virtual bool on_key_press_event(GdkEventKey* event) override;

 private:
  bool on_enter();
  void post_message_finished(const boost::optional<Json::Value>& result) const;

  api_client api_client_;
  std::string channel_id_;
};

#endif
