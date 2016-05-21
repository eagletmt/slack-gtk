#include "message_row.h"

MessageRow::MessageRow(const std::string& text)
    : label_(text, Gtk::ALIGN_START, Gtk::ALIGN_CENTER) {
  add(label_);
  label_.set_line_wrap(true);
  label_.set_line_wrap_mode(Pango::WRAP_WORD_CHAR);
  label_.show();
}

MessageRow::~MessageRow() {
}
