#include "channel_row.h"

ChannelRow::ChannelRow(const Json::Value& channel)
    : label_("", Gtk::ALIGN_START, Gtk::ALIGN_CENTER) {
  add(label_);

  const std::string id = channel["id"].asString();
  const std::string name = channel["name"].asString();
  label_.set_text("#" + name);

  label_.show();
}

ChannelRow::~ChannelRow() {
}
