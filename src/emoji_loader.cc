#include "emoji_loader.h"
#include <giomm/file.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <json/json.h>
#include <libsoup/soup-uri.h>
#include <fstream>
#include <iostream>

emoji_loader::emoji_loader(const std::string& directory)
    : session_(soup_session_new()),
      directory_(directory),
      // TODO: Use proper directory
      custom_directory_("custom-emojis") {
  std::ifstream ifs;
  const std::string path = directory_ + "/emoji.json";
  ifs.open(directory + "/" + "emoji.json");

  if (!ifs.good()) {
    std::cerr << "[emoji_loader] cannot find emoji.json at " << path
              << std::endl;
    return;
  }

  Json::Reader reader;
  Json::Value root;
  reader.parse(ifs, root);
  if (!reader.good()) {
    std::cerr << "[emoji_loader] cannot parse " << path << std::endl;
    return;
  }

  for (const Json::Value v : root) {
    emoji_data data;
    data.short_name = v["short_name"].asString();
    data.image = v["image"].asString();
    dict_.emplace(std::make_pair(data.short_name, data));
  }
}

static std::string build_cache_path(const std::string& base, std::string url) {
  std::replace(url.begin(), url.end(), '/', '_');
  return Glib::build_filename(base, url);
}

std::string emoji_loader::resolve_alias(const std::string& name) const {
  auto it = aliases_.find(name);
  if (it == aliases_.end()) {
    return name;
  } else {
    return it->second;
  }
}

Glib::RefPtr<Gdk::Pixbuf> emoji_loader::find(const std::string& name) const {
  const std::string key = resolve_alias(name);
  auto it = dict_.find(key);
  if (it == dict_.end()) {
    auto jt = custom_emojis_.find(key);
    if (jt == custom_emojis_.end()) {
      return Glib::RefPtr<Gdk::Pixbuf>();
    } else {
      const std::string path = build_cache_path(custom_directory_, jt->second);
      try {
        return Gdk::Pixbuf::create_from_file(path);
      } catch (const Gdk::PixbufError& e) {
        std::cerr << "[emoji_loader] cannot load custom emoji from " << path
                  << " (" << e.code() << ") " << e.what() << std::endl;
        return Glib::RefPtr<Gdk::Pixbuf>();
      }
    }
  } else {
    const std::string path = directory_ + "/img-google-64/" + it->second.image;
    try {
      return Gdk::Pixbuf::create_from_file(path);
    } catch (const Glib::FileError& e) {
      if (e.code() == Glib::FileError::NO_SUCH_ENTITY) {
        return Glib::RefPtr<Gdk::Pixbuf>();
      } else {
        throw e;
      }
    } catch (const Gdk::PixbufError& e) {
      std::cerr << "[emoji_loader] cannot load emoji from " << path << " ("
                << e.code() << ") " << e.what() << std::endl;
      return Glib::RefPtr<Gdk::Pixbuf>();
    }
  }
}

void emoji_loader::add_custom_emoji(const std::string& name,
                                    const std::string& url) {
  if (url.compare(0, 6, "alias:") == 0) {
    aliases_.emplace(std::make_pair(name, url.substr(6)));
  } else {
    cache_emoji(name, url);
  }
}

static void ensure_directory(const std::string& path) try {
  Gio::File::create_for_path(path)->make_directory_with_parents();
} catch (Gio::Error& e) {
  if (e.code() != Gio::Error::EXISTS) {
    throw e;
  }
}

void emoji_loader::cache_emoji(const std::string& name,
                               const std::string& url) {
  ensure_directory(custom_directory_);
  const std::string path = build_cache_path(custom_directory_, url);
  if (Glib::file_test(path,
                      Glib::FILE_TEST_IS_REGULAR | Glib::FILE_TEST_EXISTS)) {
    custom_emojis_.emplace(std::make_pair(name, url));
    return;
  }

  SoupMessage* message = soup_message_new("GET", url.c_str());
  soup_session_queue_message(
      session_, message, load_callback,
      new std::pair<std::string, emoji_loader*>(name, this));
}

void emoji_loader::load_callback(SoupSession*, SoupMessage* message,
                                 gpointer user_data) {
  std::pair<std::string, emoji_loader*>* arg =
      static_cast<decltype(arg)>(user_data);
  arg->second->on_load(arg->first, message);
  delete arg;
}

void emoji_loader::on_load(const std::string& name, SoupMessage* message) {
  char* uri_c = soup_uri_to_string(soup_message_get_uri(message), FALSE);
  const std::string uri(uri_c);
  g_free(uri_c);

  if (SOUP_STATUS_IS_TRANSPORT_ERROR(message->status_code)) {
    std::cerr << "[emoji_loader] " << uri << " (" << message->status_code
              << ") " << soup_status_get_phrase(message->status_code)
              << std::endl;
  } else {
    const std::string path = build_cache_path(custom_directory_, uri);
    std::ofstream ofs;
    ofs.open(path, std::ios::binary);
    ofs.write(message->response_body->data, message->response_body->length);
    custom_emojis_.emplace(std::make_pair(name, uri));
  }
}
