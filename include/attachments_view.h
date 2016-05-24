#ifndef SLACK_GTK_ATTACHMENTS_VIEW_H
#define SLACK_GTK_ATTACHMENTS_VIEW_H

#include <gtkmm/box.h>
#include <json/json.h>
#include "channels_store.h"
#include "emoji_loader.h"
#include "users_store.h"

class AttachmentsView : public Gtk::Box {
 public:
  AttachmentsView(const users_store& users_store,
                  const channels_store& channels_store,
                  emoji_loader& emoji_loader, const Json::Value& attachments);
  ~AttachmentsView() override;
};

#endif
