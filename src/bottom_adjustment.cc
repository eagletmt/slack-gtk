#include "bottom_adjustment.h"
#include <cmath>
#include <iostream>

static inline bool double_eq(double x, double y) {
  return std::fabs(x - y) < 1e-6;
}

BottomAdjustment::BottomAdjustment(double value, double lower, double upper,
                                   double step_increment, double page_increment,
                                   double page_size)
    : Gtk::Adjustment(value, lower, upper, step_increment, page_increment,
                      page_size),
      is_bottom(double_eq(value + page_size, lower)) {
}

BottomAdjustment::~BottomAdjustment() {
}

Glib::RefPtr<Gtk::Adjustment> BottomAdjustment::create(
    Glib::RefPtr<Gtk::Adjustment> adj) {
  return Glib::RefPtr<Gtk::Adjustment>(
      new BottomAdjustment(adj->get_value(), adj->get_lower(), adj->get_upper(),
                           adj->get_step_increment(), adj->get_page_increment(),
                           adj->get_page_size()));
}

void BottomAdjustment::on_changed() {
  Adjustment::on_changed();
  if (is_bottom) {
    set_value(get_upper());
  }
}

void BottomAdjustment::on_value_changed() {
  Adjustment::on_value_changed();
  is_bottom = double_eq(get_value() + get_page_size(), get_upper());
}
