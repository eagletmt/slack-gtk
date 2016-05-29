#include <glibmm/miscutils.h>
#include <gtkmm/application.h>
#include <libnotify/notify.h>
#include <fstream>
#include <iostream>
#include "main_window.h"
#include "team.h"

int main(int argc, char* argv[]) {
  const std::string token = Glib::getenv("SLACK_GTK_TOKEN");
  if (token.empty()) {
    std::cerr << "Set SLACK_GTK_TOKEN" << std::endl;
    return 1;
  }
  const std::string emoji_directory = Glib::getenv("SLACK_GTK_EMOJI_DIRECTORY");
  if (emoji_directory.empty()) {
    std::cerr << "Set SLACK_GTK_EMOJI_DIRECTORY" << std::endl;
    return 1;
  }

  std::shared_ptr<api_client> api =
      std::make_shared<api_client>("https://slack.com/api", token);
  const boost::optional<Json::Value> result =
      api->post("rtm.start", std::map<std::string, std::string>());
  if (!result) {
    return 1;
  }
  const Json::Value json = result.get();
  {
    std::ofstream ofs;
    ofs.open("rtm.start.json");
    ofs << json;
  }

  static const char app_name[] = "cc.wanko.slack-gtk";
  notify_init(app_name);

  auto app = Gtk::Application::create(argc, argv, app_name);
  MainWindow window(api, emoji_directory, json);

  return app->run(window);
}
