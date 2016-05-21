#ifndef SLACK_GTK_ICON_LOADER_H
#define SLACK_GTK_ICON_LOADER_H

#include <gdkmm/pixbuf.h>
#include <glibmm/refptr.h>
#include <libsoup/soup-session.h>
#include <cstdint>
#include <functional>
#include <string>

class icon_loader {
 public:
  icon_loader(const std::string& cache_directory);
  ~icon_loader();

  typedef std::function<void(Glib::RefPtr<Gdk::Pixbuf>)> load_callback_type;
  void load(const std::string& url, const load_callback_type& callback);

 private:
  static void load_callback(SoupSession* session, SoupMessage* message,
                            gpointer user_data);
  void on_load(SoupMessage* message);
  Glib::RefPtr<Gdk::Pixbuf> load_from_cache(const std::string& url) const;
  void store_cache(const std::string& url, const char* data,
                   std::size_t size) const;

  std::string cache_directory_;
  std::map<std::intptr_t, load_callback_type> load_callback_registry_;
  SoupSession* session_;
};

#endif
