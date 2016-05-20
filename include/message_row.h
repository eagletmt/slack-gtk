#ifndef SLACK_GTK_MESSAGE_ROW_H
#define SLACK_GTK_MESSAGE_ROW_H

#include <gtkmm/label.h>
#include <gtkmm/listboxrow.h>

class MessageRow : public Gtk::ListBoxRow {
 public:
  MessageRow(const std::string& message);
  virtual ~MessageRow();

 private:
  Gtk::Label label_;
};

#endif
