#ifndef SLACK_GTK_CHANNEL_ROW_H
#define SLACK_GTK_CHANNEL_ROW_H

#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <json/json.h>

class ChannelRow : public Gtk::ListBoxRow {
 public:
  ChannelRow(const Json::Value& channel);
  virtual ~ChannelRow();

 private:
  Gtk::Label label_;
};

#endif
