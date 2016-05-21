#ifndef SLACK_GTK_BOTTOM_ADJUSTMENT_H
#define SLACK_GTK_BOTTOM_ADJUSTMENT_H

#include <gtkmm/adjustment.h>

class BottomAdjustment : public Gtk::Adjustment {
 public:
  virtual ~BottomAdjustment() override;

  static Glib::RefPtr<Gtk::Adjustment> create(
      Glib::RefPtr<Gtk::Adjustment> adj);

 protected:
  BottomAdjustment(double value, double lower, double upper,
                   double step_increment, double page_increment,
                   double page_size);

  virtual void on_changed() override;
  virtual void on_value_changed() override;

 private:
  bool is_bottom;
};

#endif
