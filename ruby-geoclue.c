#include <glib-2.0/glib.h>
#include <geoclue/geoclue-master.h>
#include <geoclue/geoclue-master-client.h>
#include <ruby.h>

static VALUE rb_mGeoclue = Qnil;
static VALUE rb_cGeoclueMaster = Qnil;
static VALUE rb_cGeoclueMasterClient = Qnil;
static VALUE rb_cGeocluePosition = Qnil;
static VALUE rb_cGeoclueProvider = Qnil;

static VALUE master_get_default()
{
	return Data_Wrap_Struct(rb_cGeoclueMaster, NULL, NULL, geoclue_master_get_default());
}

static VALUE master_create_client()
{
	return Data_Wrap_Struct(rb_cGeoclueMasterClient, NULL, NULL, geoclue_master_create_client(geoclue_master_get_default(), NULL, NULL));
}

void Init_geoclue()
{
	g_type_init();

	rb_mGeoclue = rb_define_module("Geoclue");

	rb_define_const(rb_mGeoclue, "ACCURACY_NONE", INT2FIX(GEOCLUE_ACCURACY_LEVEL_NONE));
	rb_define_const(rb_mGeoclue, "ACCURACY_COUNTRY", INT2FIX(GEOCLUE_ACCURACY_LEVEL_COUNTRY));
	rb_define_const(rb_mGeoclue, "ACCURACY_REGION", INT2FIX(GEOCLUE_ACCURACY_LEVEL_REGION));
	rb_define_const(rb_mGeoclue, "ACCURACY_LOCALITY", INT2FIX(GEOCLUE_ACCURACY_LEVEL_LOCALITY));
	rb_define_const(rb_mGeoclue, "ACCURACY_POSTALCODE", INT2FIX(GEOCLUE_ACCURACY_LEVEL_POSTALCODE));
	rb_define_const(rb_mGeoclue, "ACCURACY_STREET", INT2FIX(GEOCLUE_ACCURACY_LEVEL_STREET));
	rb_define_const(rb_mGeoclue, "ACCURACY_DETAILED", INT2FIX(GEOCLUE_ACCURACY_LEVEL_DETAILED));

	rb_define_const(rb_mGeoclue, "STATUS_ERROR", INT2FIX(GEOCLUE_STATUS_ERROR));
	rb_define_const(rb_mGeoclue, "STATUS_UNAVAILABLE", INT2FIX(GEOCLUE_STATUS_UNAVAILABLE));
	rb_define_const(rb_mGeoclue, "STATUS_ACQUIRING", INT2FIX(GEOCLUE_STATUS_ACQUIRING));
	rb_define_const(rb_mGeoclue, "STATUS_AVAILABLE", INT2FIX(GEOCLUE_STATUS_AVAILABLE));

	rb_cGeoclueMaster = rb_define_class_under(rb_mGeoclue, "Master", rb_cObject);

	rb_define_module_function(rb_cGeoclueMaster, "get_default", master_get_default, 0);

	rb_define_method(rb_cGeoclueMaster, "create_client", master_create_client, 0);

	rb_cGeoclueMasterClient = rb_define_class_under(rb_mGeoclue, "MasterClient", rb_cObject);

	rb_cGeoclueProvider = rb_define_class_under(rb_mGeoclue, "Provider", rb_cObject);

	rb_cGeocluePosition = rb_define_class_under(rb_mGeoclue, "Position", rb_cGeoclueProvider);

}
