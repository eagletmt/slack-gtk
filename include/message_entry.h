#ifndef SLACK_GTK_MESSAGE_ENTRY_H
#define SLACK_GTK_MESSAGE_ENTRY_H

#include <gtkmm/entry.h>
#include "api_client.h"
#include "emoji_loader.h"
#include "message_completion_record.h"

class MessageEntry : public Gtk::Entry {
 public:
  MessageEntry(const api_client& api_client, emoji_loader& emoji_loader,
               const std::string& channel_id);
  virtual ~MessageEntry();

 protected:
  virtual void on_activate() override;

 private:
  void post_message_finished(const boost::optional<Json::Value>& result) const;
  bool match_func(const Glib::ustring& text,
                  const Gtk::TreeModel::const_iterator& iter) const;

  const MessageCompletionRecord completion_columns_;

  api_client api_client_;
  emoji_loader& emoji_loader_;
  std::string channel_id_;
};

#endif
