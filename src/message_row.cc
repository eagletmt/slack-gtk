#include "message_row.h"

MessageRow::MessageRow(const std::string& text)
    : label_(text, Gtk::ALIGN_START, Gtk::ALIGN_CENTER) {
  add(label_);
  label_.show();
}

MessageRow::~MessageRow() {
}
