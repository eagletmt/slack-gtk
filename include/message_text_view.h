#ifndef SLACK_GTK_MESSAGE_TEXT_VIEW_H
#define SLACK_GTK_MESSAGE_TEXT_VIEW_H

#include <giomm/settings.h>
#include <gtkmm/textview.h>
#include "team.h"

class MessageTextView : public Gtk::TextView {
 public:
  MessageTextView(team& team, Glib::RefPtr<Gio::Settings> settings);
  ~MessageTextView() override;

  void set_text(const std::string& text, bool is_message);
  std::string get_text() const;

  sigc::signal<void, const std::string&> signal_user_link_clicked();
  sigc::signal<void, const std::string&> signal_channel_link_clicked();

 private:
  Gtk::TextBuffer::iterator insert_hyperlink(
      Glib::RefPtr<Gtk::TextBuffer> buffer, Gtk::TextBuffer::iterator iter,
      const std::string& linker);
  Gtk::TextBuffer::iterator insert_markdown_text(
      Glib::RefPtr<Gtk::TextBuffer> buffer, Gtk::TextBuffer::iterator iter,
      const std::string& text, bool is_message);

  virtual bool on_motion_notify_event(GdkEventMotion* event) override;
  void on_event_after(GdkEvent* event);

  Glib::RefPtr<Gdk::Cursor> default_cursor_, pointer_cursor_;

  team& team_;
  Glib::RefPtr<Gio::Settings> settings_;

  sigc::signal<void, const std::string &> signal_user_link_clicked_,
      signal_channel_link_clicked_;
};

#endif
