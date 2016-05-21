#ifndef SLACK_GTK_MESSAGE_ROW_H
#define SLACK_GTK_MESSAGE_ROW_H

#include <gtkmm/label.h>
#include <gtkmm/listboxrow.h>
#include "users_store.h"

class MessageRow : public Gtk::ListBoxRow {
 public:
  MessageRow(const users_store& users_store, const Json::Value& payload);
  virtual ~MessageRow();

 private:
  std::string build_text(const Json::Value& payload);

  Gtk::Label label_;

  const users_store& users_store_;
};

#endif
