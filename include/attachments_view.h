#ifndef SLACK_GTK_ATTACHMENTS_VIEW_H
#define SLACK_GTK_ATTACHMENTS_VIEW_H

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <json/json.h>
#include "team.h"

class AttachmentsView : public Gtk::Box {
 public:
  AttachmentsView(team& team, Glib::RefPtr<Gio::Settings> settings,
                  const Json::Value& attachments);
  ~AttachmentsView() override;
};

#endif
