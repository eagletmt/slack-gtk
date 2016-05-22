#ifndef SLACK_GTK_MESSAGE_COMPLETION_RECORD_H
#define SLACK_GTK_MESSAGE_COMPLETION_RECORD_H

#include <gtkmm/treemodelcolumn.h>

struct MessageCompletionRecord : public Gtk::TreeModelColumnRecord {
  Gtk::TreeModelColumn<Glib::ustring> short_name;

  MessageCompletionRecord() {
    add(short_name);
  }
};

#endif
