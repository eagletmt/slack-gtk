#ifndef SLACK_GTK_MESSAGE_TEXT_VIEW_H
#define SLACK_GTK_MESSAGE_TEXT_VIEW_H

#include <gtkmm/textview.h>
#include "channels_store.h"
#include "users_store.h"

class MessageTextView : public Gtk::TextView {
 public:
  MessageTextView(const users_store& users_store,
                  const channels_store& channels_store);
  ~MessageTextView() override;

  void set_text(const std::string& text, bool is_message);
  std::string get_text() const;

  sigc::signal<void, const std::string&> signal_user_link_clicked();
  sigc::signal<void, const std::string&> signal_channel_link_clicked();

 private:
  Gtk::TextBuffer::iterator insert_hyperlink(
      Glib::RefPtr<Gtk::TextBuffer> buffer, Gtk::TextBuffer::iterator iter,
      const std::string& linker);

  bool on_motion_notify_event(GdkEventMotion* event);
  void on_event_after(GdkEvent* event);

  Glib::RefPtr<Gdk::Cursor> default_cursor_, pointer_cursor_;

  const users_store& users_store_;
  const channels_store& channels_store_;

  sigc::signal<void, const std::string &> signal_user_link_clicked_,
      signal_channel_link_clicked_;
};

#endif
