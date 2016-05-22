#ifndef SLACK_GTK_EMOJI_LOADER_H
#define SLACK_GTK_EMOJI_LOADER_H

#include <gdkmm/pixbuf.h>
#include <glibmm/refptr.h>
#include "emoji_data.h"

class emoji_loader {
 public:
  emoji_loader(const std::string& directory);

  Glib::RefPtr<Gdk::Pixbuf> find(const std::string& name) const;

 private:
  std::string directory_;
  std::map<std::string, emoji_data> dict_;
};

#endif
