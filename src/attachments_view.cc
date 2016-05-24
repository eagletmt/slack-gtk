#include "attachments_view.h"
#include "message_text_view.h"

AttachmentsView::AttachmentsView(const users_store& users_store,
                                 const channels_store& channels_store,
                                 emoji_loader& emoji_loader,
                                 const Json::Value& attachments) {
  for (const Json::Value& attachment : attachments) {
    auto view = Gtk::manage(
        new MessageTextView(users_store, channels_store, emoji_loader));
    // TODO: Render attachment in rich format
    if (attachment["fallback"].isString()) {
      view->set_text(attachment["fallback"].asString(), true);
    } else {
      view->set_text("No fallback found", true);
    }
    add(*view);
  }

  show_all_children();
}

AttachmentsView::~AttachmentsView() {
}
