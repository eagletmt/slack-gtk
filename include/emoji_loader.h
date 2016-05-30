#ifndef SLACK_GTK_EMOJI_LOADER_H
#define SLACK_GTK_EMOJI_LOADER_H

#include <gdkmm/pixbuf.h>
#include <glibmm/refptr.h>
#include <libsoup/soup-session.h>
#include "emoji_data.h"

class emoji_loader {
 public:
  emoji_loader(const std::string& directory);

  Glib::RefPtr<Gdk::Pixbuf> find(const std::string& name) const;
  void add_custom_emoji(const std::string& name, const std::string& url);

 private:
  std::string resolve_alias(const std::string& name) const;
  void cache_emoji(const std::string& name, const std::string& url);
  static void load_callback(SoupSession* session, SoupMessage* message,
                            gpointer user_data);
  void on_load(const std::string& name, SoupMessage* message);

  SoupSession* session_;

  std::string directory_;
  std::string custom_directory_;
  std::map<std::string, emoji_data> dict_;
  std::map<std::string, std::string> aliases_;
  std::map<std::string, std::string> custom_emojis_;
};

#endif
