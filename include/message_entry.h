#ifndef SLACK_GTK_MESSAGE_ENTRY_H
#define SLACK_GTK_MESSAGE_ENTRY_H

#include <gtkmm/entry.h>
#include "api_client.h"

class MessageEntry : public Gtk::Entry {
 public:
  MessageEntry(const api_client& api_client);
  virtual ~MessageEntry();

  void set_channel_id(const std::string& channel_id);

 protected:
  virtual void on_activate();

 private:
  api_client api_client_;
  std::string channel_id_;
};

#endif
