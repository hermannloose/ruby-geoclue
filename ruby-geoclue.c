#include <glib-2.0/glib.h>
#include <geoclue/geoclue-master.h>
#include <geoclue/geoclue-master-client.h>
#include <ruby.h>

static VALUE rb_mGeoclue = Qnil;
static VALUE rb_cGeocluePosition = Qnil;
static VALUE rb_cGeoclueProvider = Qnil;

VALUE method_get_position_provider(VALUE self);

void Init_geoclue()
{
	rb_mGeoclue = rb_define_module("Geoclue");
	rb_define_method(rb_mGeoclue, "get_position_provider", method_get_position_provider, 0);
	rb_cGeoclueProvider = rb_define_class_under(rb_mGeoclue, "Provider", rb_cObject);
	rb_cGeocluePosition = rb_define_class_under(rb_mGeoclue, "Position", rb_cGeoclueProvider);
}
