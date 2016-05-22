#include "message_text_view.h"
#include <iostream>
#include <regex>

MessageTextView::MessageTextView(const users_store& users_store,
                                 const channels_store& channels_store)
    : users_store_(users_store), channels_store_(channels_store) {
}

MessageTextView::~MessageTextView() {
}

static void create_tags(Glib::RefPtr<Gtk::TextBuffer> buffer) {
  auto tag = buffer->create_tag("info_message");
  tag->property_foreground() = "gray";
}

static Gtk::TextBuffer::iterator insert_user_link(
    Glib::RefPtr<Gtk::TextBuffer> buffer, Gtk::TextBuffer::iterator iter,
    const std::string& user_id, const std::string& user_name) {
  Glib::RefPtr<Gtk::TextTag> tag = buffer->create_tag();
  tag->property_foreground() = "blue";
  tag->property_underline() = Pango::UNDERLINE_SINGLE;
  tag->set_data("user_id", g_strdup(user_id.c_str()), g_free);
  return buffer->insert_with_tag(iter, user_name, tag);
}

static Gtk::TextBuffer::iterator insert_channel_link(
    Glib::RefPtr<Gtk::TextBuffer> buffer, Gtk::TextBuffer::iterator iter,
    const std::string& channel_id, const std::string& channel_name) {
  Glib::RefPtr<Gtk::TextTag> tag = buffer->create_tag();
  tag->property_foreground() = "blue";
  tag->property_underline() = Pango::UNDERLINE_SINGLE;
  tag->set_data("channel_id", g_strdup(channel_id.c_str()), g_free);
  return buffer->insert_with_tag(iter, channel_name, tag);
}

static Gtk::TextBuffer::iterator insert_url_link(
    Glib::RefPtr<Gtk::TextBuffer> buffer, Gtk::TextBuffer::iterator iter,
    const std::string& url, const std::string& link_name) {
  Glib::RefPtr<Gtk::TextTag> tag = buffer->create_tag();
  tag->property_foreground() = "blue";
  tag->property_underline() = Pango::UNDERLINE_SINGLE;
  tag->set_data("url", g_strdup(url.c_str()), g_free);
  return buffer->insert_with_tag(iter, link_name, tag);
}

Gtk::TextBuffer::iterator MessageTextView::insert_hyperlink(
    Glib::RefPtr<Gtk::TextBuffer> buffer, Gtk::TextBuffer::iterator iter,
    const std::string& linker) {
  std::regex url_re("^(.+)\\|(.+)$");
  std::smatch match;

  if (std::regex_match(linker, match, url_re)) {
    const std::string& left = match[1];
    const std::string& right = match[2];
    switch (left[0]) {
      case '@':
        iter = insert_user_link(buffer, iter, left, "@" + right);
        break;
      case '#':
        iter = insert_channel_link(buffer, iter, left, "#" + right);
        break;
      default:
        iter = insert_url_link(buffer, iter, left, right);
        break;
    }
  } else {
    switch (linker[0]) {
      case '@': {
        const boost::optional<user> o_user =
            users_store_.find(linker.substr(1, linker.size() - 1));
        if (o_user) {
          iter =
              insert_user_link(buffer, iter, linker, "@" + o_user.get().name);
        } else {
          std::cerr << "[MessageTextView] cannot find linked user " << linker
                    << std::endl;
          iter = insert_user_link(buffer, iter, linker, linker);
        }
      } break;
      case '#': {
        const boost::optional<channel> o_channel =
            channels_store_.find(linker.substr(1, linker.size() - 1));
        if (o_channel) {
          iter = insert_channel_link(buffer, iter, linker,
                                     "#" + o_channel.get().name);
        } else {
          std::cerr << "[MessageTextView] cannot find linked channel " << linker
                    << std::endl;
          iter = insert_channel_link(buffer, iter, linker, linker);
        }
      } break;
      default:
        iter = insert_url_link(buffer, iter, linker, linker);
        break;
    }
  }
  return iter;
}

static Gtk::TextBuffer::iterator insert_markdown_text(
    Glib::RefPtr<Gtk::TextBuffer> buffer, Gtk::TextBuffer::iterator iter,
    const std::string& text, bool is_message) {
  // TODO: format markdown-like text
  if (is_message) {
    return buffer->insert(iter, text);
  } else {
    return buffer->insert_with_tag(iter, text, "info_message");
  }
}

void MessageTextView::set_text(const std::string& text, bool is_message) {
  Glib::RefPtr<Gtk::TextBuffer> buffer = Gtk::TextBuffer::create();
  create_tags(buffer);
  Gtk::TextBuffer::iterator iter = buffer->get_iter_at_offset(0);

  std::regex markup_re("<([^<>]*)>");
  std::sregex_iterator re_it(text.begin(), text.end(), markup_re), re_end;

  std::size_t pos = 0;

  for (; re_it != re_end; ++re_it) {
    iter = insert_markdown_text(
        buffer, iter, text.substr(pos, re_it->position() - pos), is_message);
    iter = insert_hyperlink(buffer, iter, (*re_it)[1].str());
    pos = re_it->position() + re_it->length();
  }
  iter = insert_markdown_text(buffer, iter, text.substr(pos), is_message);

  set_buffer(buffer);
}

std::string MessageTextView::get_text() const {
  return get_buffer()->get_text();
}
