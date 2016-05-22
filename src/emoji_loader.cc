#include "emoji_loader.h"
#include <json/json.h>
#include <fstream>
#include <iostream>

emoji_loader::emoji_loader(const std::string& directory)
    : directory_(directory) {
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

Glib::RefPtr<Gdk::Pixbuf> emoji_loader::find(const std::string& name) const {
  auto it = dict_.find(name);
  if (it == dict_.end()) {
    return Glib::RefPtr<Gdk::Pixbuf>();
  } else {
    const std::string path = directory_ + "/img-google-64/" + it->second.image;
    return Gdk::Pixbuf::create_from_file(path);
  }
}
