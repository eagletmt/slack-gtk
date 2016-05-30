#include "attachments_view.h"
#include "message_text_view.h"

AttachmentsView::AttachmentsView(team& team,
                                 Glib::RefPtr<Gio::Settings> settings,
                                 const Json::Value& attachments) {
  set_orientation(Gtk::ORIENTATION_VERTICAL);

  for (const Json::Value& attachment : attachments) {
    auto view = Gtk::manage(new MessageTextView(team, settings));
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
