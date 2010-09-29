#include <glib-2.0/glib.h>
#include <geoclue/geoclue-master.h>
#include <geoclue/geoclue-master-client.h>
#include <ruby.h>

#include <stdio.h>

static VALUE rb_mGeoclue = Qnil;
static VALUE rb_cGeoclueAddress = Qnil;
static VALUE rb_cGeoclueMaster = Qnil;
static VALUE rb_cGeoclueMasterClient = Qnil;
static VALUE rb_cGeocluePosition = Qnil;
static VALUE rb_cGeoclueProvider = Qnil;

static VALUE master_get_default()
{
	// FIXME Mark and sweep functions.
	return Data_Wrap_Struct(rb_cGeoclueMaster, NULL, NULL, geoclue_master_get_default());
}

static VALUE master_create_client(VALUE self)
{
	GeoclueMaster *master;
	Data_Get_Struct(self, GeoclueMaster, master);
	// FIXME Mark and sweep functions.
	return Data_Wrap_Struct(rb_cGeoclueMasterClient, NULL, NULL, geoclue_master_create_client(master, NULL, NULL));
}

static VALUE master_client_create_position(VALUE self)
{
	GeoclueMasterClient *client;
	Data_Get_Struct(self, GeoclueMasterClient, client);
	return Data_Wrap_Struct(rb_cGeocluePosition, NULL, NULL, geoclue_master_client_create_position(client, NULL));
}

static VALUE master_client_get_address_provider(VALUE self)
{
	GeoclueMasterClient *client;
	Data_Get_Struct(self, GeoclueMasterClient, client);
	char *name, *description, *service, *path;
	if (geoclue_master_client_get_position_provider(client, &name, &description, &service, &path, NULL)) {
		// FIXME Mark and sweep functions
		return Data_Wrap_Struct(rb_cGeoclueAddress, NULL, NULL, geoclue_address_new(service, path));
	}
	return Qnil;
}

static VALUE master_client_get_position_provider(VALUE self)
{
	GeoclueMasterClient *client;
	Data_Get_Struct(self, GeoclueMasterClient, client);
	char *name, *description, *service, *path;
	if (geoclue_master_client_get_position_provider(client, &name, &description, &service, &path, NULL)) {
		// FIXME Mark and sweep functions
		printf("Before\n");
		geoclue_position_new(service, path);
		printf("After\n");
		return Data_Wrap_Struct(rb_cGeocluePosition, NULL, NULL, geoclue_position_new(service, path));
	}
	return Qnil;
}

static VALUE position_latitude(VALUE self)
{
	GeocluePosition *position;
	double latitude;
	Data_Get_Struct(self, GeocluePosition, position);
	geoclue_position_get_position(position, NULL, &latitude, NULL, NULL, NULL, NULL);
	return rb_float_new(latitude);
}

static VALUE provider_get_status(VALUE self)
{
	GeoclueProvider *provider;
	GeoclueStatus status;
	GError *error;
	Data_Get_Struct(self, GeoclueProvider, provider);
	// FIXME Error handling
	if (geoclue_provider_get_status(provider, &status, &error)) {
		return INT2FIX(status);
	} else {
		return INT2FIX(GEOCLUE_STATUS_ERROR);
	}
}

void Init_geoclue()
{
	g_type_init();

	rb_mGeoclue = rb_define_module("Geoclue");

	// Geoclue constants

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

	// Geoclue classes

	rb_cGeoclueMaster = rb_define_class_under(rb_mGeoclue, "Master", rb_cObject);

	rb_define_module_function(rb_cGeoclueMaster, "get_default", master_get_default, 0);

	rb_define_method(rb_cGeoclueMaster, "create_client", master_create_client, 0);

	rb_cGeoclueMasterClient = rb_define_class_under(rb_mGeoclue, "MasterClient", rb_cObject);
	rb_define_method(rb_cGeoclueMasterClient, "create_position", master_client_create_position, 0);
	rb_define_method(rb_cGeoclueMasterClient, "get_position_provider", master_client_get_position_provider, 0);

	rb_cGeoclueProvider = rb_define_class_under(rb_mGeoclue, "Provider", rb_cObject);
	rb_define_method(rb_cGeoclueProvider, "get_status", provider_get_status, 0);

	// Geoclue::Provider classes

	rb_cGeoclueAddress = rb_define_class_under(rb_mGeoclue, "Address", rb_cGeoclueProvider);

	rb_cGeocluePosition = rb_define_class_under(rb_mGeoclue, "Position", rb_cGeoclueProvider);
	rb_define_method(rb_cGeocluePosition, "latitude", position_latitude, 0);
}
