#ifndef SLACK_GTK_API_CLIENT_H
#define SLACK_GTK_API_CLIENT_H

#include <gdkmm/pixbuf.h>
#include <glibmm/refptr.h>
#include <json/json.h>
#include <libsoup/soup.h>
#include <boost/optional.hpp>
#include <cstdint>

class api_client {
 public:
  api_client(const std::string endpoint, const std::string token);
  ~api_client();
  api_client(const api_client& other);

  typedef std::function<void(const boost::optional<Json::Value>&)>
      post_callback_type;

  boost::optional<Json::Value> post(
      const std::string& method_name,
      const std::map<std::string, std::string>& params);
  void queue_post(const std::string& method_name,
                  const std::map<std::string, std::string>& params,
                  const post_callback_type& callback);

  typedef std::function<void(Glib::RefPtr<Gdk::Pixbuf>)>
      get_shared_file_callback_type;
  void get_shared_file(const std::string& url,
                       const get_shared_file_callback_type& callback);

 private:
  void setup();
  SoupMessage* build_message(
      const std::string& method_name,
      const std::map<std::string, std::string>& params) const;
  static void queue_callback(SoupSession*, SoupMessage* message,
                             gpointer user_data);
  void on_queue_callback(SoupMessage* message);
  static void get_shared_file_callback(SoupSession* session,
                                       SoupMessage* message,
                                       gpointer user_data);
  void on_shared_file_loaded(SoupMessage* message);

  SoupSession* session_;
  const std::string endpoint_;
  const std::string token_;
  std::map<std::intptr_t, post_callback_type> callback_registry_;
  std::multimap<std::string, get_shared_file_callback_type>
      get_shared_file_callbacks_;
};

#endif
