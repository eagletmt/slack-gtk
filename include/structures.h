#ifndef SLACK_CPP_STRUCTURES_H
#define SLACK_CPP_STRUCTURES_H

#include <string>

struct channel {
  std::string id;
  bool is_channel;
  bool is_general;
  bool is_member;
  double last_read;
  std::vector<std::string> members;
  std::string name;
  int unread_count;
  int unread_count_display;
};

#endif
