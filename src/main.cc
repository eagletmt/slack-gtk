#include <glibmm/miscutils.h>
#include <gtkmm/application.h>
#include <iostream>
#include "api_client.h"
#include "main_window.h"

int main(int argc, char* argv[]) {
  const std::string token = Glib::getenv("SLACK_GTK_TOKEN");
  if (token.empty()) {
    std::cerr << "Set SLACK_GTK_TOKEN" << std::endl;
    return 1;
  }
  api_client api_client("https://slack.com/api", token);
  const boost::optional<Json::Value> result = api_client.get("rtm.start");
  if (!result) {
    return 1;
  }
  const Json::Value json = result.get();
  std::cout << json << std::endl;

  auto app = Gtk::Application::create(argc, argv, "cc.wanko.slack-gtk");
  MainWindow window;
  window.start(json);

  return app->run(window);
}
