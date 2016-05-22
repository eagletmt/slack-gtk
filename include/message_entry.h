#ifndef SLACK_GTK_MESSAGE_ENTRY_H
#define SLACK_GTK_MESSAGE_ENTRY_H

#include <gtkmm/entry.h>
#include "api_client.h"

class MessageEntry : public Gtk::Entry {
 public:
  MessageEntry(const api_client& api_client, const std::string& channel_id);
  virtual ~MessageEntry();

 protected:
  virtual void on_activate() override;

 private:
  void post_message_finished(const boost::optional<Json::Value>& result) const;

  api_client api_client_;
  std::string channel_id_;
};

#endif
