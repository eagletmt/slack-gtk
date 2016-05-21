#include "icon_loader.h"
#include <gdkmm/pixbufloader.h>
#include <giomm/file.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <libsoup/soup-uri.h>
#include <fstream>
#include <iostream>

icon_loader::icon_loader(const std::string &cache_directory)
    : cache_directory_(cache_directory),
      load_callback_registry_(),
      session_(soup_session_new()) {
}

icon_loader::~icon_loader() {
  g_object_unref(session_);
}

void icon_loader::load(const std::string &url,
                       const load_callback_type &callback) {
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = load_from_cache(url);
  if (pixbuf) {
    callback(pixbuf);
    return;
  }

  auto it = load_callback_registry_.find(url);
  if (it == load_callback_registry_.end()) {
    SoupMessage *message = soup_message_new("GET", url.c_str());
    load_callback_registry_.emplace(std::make_pair(url, callback));
    soup_session_queue_message(session_, message, load_callback, this);
  } else {
    load_callback_registry_.emplace(std::make_pair(url, callback));
  }
}

static std::string build_cache_path(const std::string &base, std::string url) {
  std::replace(url.begin(), url.end(), '/', '_');
  return Glib::build_filename(base, url);
}

static void ensure_directory(const std::string &path) try {
  Gio::File::create_for_path(path)->make_directory_with_parents();
} catch (Gio::Error &e) {
  if (e.code() != Gio::Error::EXISTS) {
    throw e;
  }
}

Glib::RefPtr<Gdk::Pixbuf> icon_loader::load_from_cache(
    const std::string &url) const {
  ensure_directory(cache_directory_);
  const std::string cache_path = build_cache_path(cache_directory_, url);
  if (Glib::file_test(cache_path,
                      Glib::FILE_TEST_IS_REGULAR | Glib::FILE_TEST_EXISTS)) {
    return Gdk::Pixbuf::create_from_file(cache_path);
  } else {
    return Glib::RefPtr<Gdk::Pixbuf>();
  }
}

void icon_loader::store_cache(const std::string &url, const char *data,
                              std::size_t size) const {
  ensure_directory(cache_directory_);
  const std::string cache_path = build_cache_path(cache_directory_, url);
  std::ofstream ofs;
  ofs.open(cache_path, std::ios::binary);
  ofs.write(data, size);
}

void icon_loader::load_callback(SoupSession *, SoupMessage *message,
                                gpointer user_data) {
  static_cast<icon_loader *>(user_data)->on_load(message);
}

void icon_loader::on_load(SoupMessage *message) {
  char *uri_c = soup_uri_to_string(soup_message_get_uri(message), FALSE);
  const std::string uri(uri_c);
  auto equal_range = load_callback_registry_.equal_range(uri);

  if (SOUP_STATUS_IS_TRANSPORT_ERROR(message->status_code)) {
    std::cerr << "[icon_loader] " << uri << " (" << message->status_code << ") "
              << soup_status_get_phrase(message->status_code) << std::endl;
  } else {
    Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
    store_cache(uri, message->response_body->data,
                message->response_body->length);
    loader->write(
        reinterpret_cast<const guint8 *>(message->response_body->data),
        message->response_body->length);
    loader->close();
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = loader->get_pixbuf();
    for (auto it = equal_range.first; it != equal_range.second; ++it) {
      (it->second)(loader->get_pixbuf());
    }
  }

  load_callback_registry_.erase(equal_range.first, equal_range.second);
  g_free(uri_c);
}
