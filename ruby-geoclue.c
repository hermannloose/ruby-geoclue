#include <glib-2.0/glib.h>
#include <geoclue/geoclue-geocode.h>
#include <geoclue/geoclue-master.h>
#include <geoclue/geoclue-master-client.h>
#include <ruby.h>

#include <stdio.h>

static VALUE rb_mGeoclue = Qnil;
static VALUE rb_cGeoclueAddress = Qnil;
static VALUE rb_cGeoclueGeocode = Qnil;
static VALUE rb_cGeoclueMaster = Qnil;
static VALUE rb_cGeoclueMasterClient = Qnil;
static VALUE rb_cGeocluePosition = Qnil;
static VALUE rb_cGeoclueProvider = Qnil;

static VALUE address_new(VALUE klass, VALUE service, VALUE path)
{
	// FIXME Mark and sweep functions
	return Data_Wrap_Struct(rb_cGeoclueAddress, NULL, NULL, geoclue_address_new(
		StringValuePtr(service),
		StringValuePtr(path)
	));
}

static VALUE address_get_address(VALUE self)
{
	GeoclueAddress *address;
	Data_Get_Struct(self, GeoclueAddress, address);
	int timestamp;
	GHashTable *details;
	GeoclueAccuracy *accuracy;
	GError *error = NULL;

	if (geoclue_address_get_address(address, &timestamp, &details, &accuracy, &error)) {
		VALUE hash = rb_hash_new();

		rb_hash_aset(hash, ID2SYM(rb_intern("timestamp")), INT2FIX(timestamp));

		char *countrycode = g_hash_table_lookup(details, GEOCLUE_ADDRESS_KEY_COUNTRYCODE);
		if (countrycode != NULL) rb_hash_aset(hash, ID2SYM(rb_intern("countrycode")), rb_tainted_str_new2(countrycode));

		char *country = g_hash_table_lookup(details, GEOCLUE_ADDRESS_KEY_COUNTRY);
		if (country != NULL) rb_hash_aset(hash, ID2SYM(rb_intern("country")), rb_tainted_str_new2(country));

		char *region = g_hash_table_lookup(details, GEOCLUE_ADDRESS_KEY_REGION);
		if (region != NULL) rb_hash_aset(hash, ID2SYM(rb_intern("region")), rb_tainted_str_new2(region));

		char *locality = g_hash_table_lookup(details, GEOCLUE_ADDRESS_KEY_LOCALITY);
		if (locality != NULL) rb_hash_aset(hash, ID2SYM(rb_intern("locality")), rb_tainted_str_new2(locality));

		char *area = g_hash_table_lookup(details, GEOCLUE_ADDRESS_KEY_AREA);
		if (area != NULL) rb_hash_aset(hash, ID2SYM(rb_intern("area")), rb_tainted_str_new2(area));

		char *postalcode = g_hash_table_lookup(details, GEOCLUE_ADDRESS_KEY_POSTALCODE);
		if (postalcode != NULL) rb_hash_aset(hash, ID2SYM(rb_intern("postalcode")), rb_tainted_str_new2(postalcode));

		char *street = g_hash_table_lookup(details, GEOCLUE_ADDRESS_KEY_STREET);
		if (street != NULL) rb_hash_aset(hash, ID2SYM(rb_intern("street")), rb_tainted_str_new2(street));

		return hash;
	}
}

static VALUE geocode_new(VALUE klass, VALUE service, VALUE path)
{
	// FIXME Mark and sweep functions
	return Data_Wrap_Struct(rb_cGeoclueGeocode, NULL, NULL, geoclue_geocode_new(
		StringValuePtr(service),
		StringValuePtr(path)
	));
}

static VALUE geocode_address_to_position(VALUE self, VALUE details)
{
	if (TYPE(details) != T_HASH) rb_raise(rb_eTypeError, "Expecting a Hash of address details.");

	GeoclueGeocode *geocode;
	Data_Get_Struct(self, GeoclueGeocode, geocode);

	GHashTable *hashtable = g_hash_table_new(NULL, NULL);

	VALUE countrycode = rb_hash_aref(details, ID2SYM(rb_intern("countrycode")));
	if (!NIL_P(countrycode)) g_hash_table_insert(hashtable, GEOCLUE_ADDRESS_KEY_COUNTRYCODE, StringValuePtr(countrycode));

	VALUE country = rb_hash_aref(details, ID2SYM(rb_intern("country")));
	if (!NIL_P(country)) g_hash_table_insert(hashtable, GEOCLUE_ADDRESS_KEY_COUNTRY, StringValuePtr(country));

	VALUE region = rb_hash_aref(details, ID2SYM(rb_intern("region")));
	if (!NIL_P(region)) g_hash_table_insert(hashtable, GEOCLUE_ADDRESS_KEY_REGION, StringValuePtr(region));

	VALUE locality = rb_hash_aref(details, ID2SYM(rb_intern("locality")));
	if (!NIL_P(locality)) g_hash_table_insert(hashtable, GEOCLUE_ADDRESS_KEY_LOCALITY, StringValuePtr(locality));

	VALUE area = rb_hash_aref(details, ID2SYM(rb_intern("area")));
	if (!NIL_P(area)) g_hash_table_insert(hashtable, GEOCLUE_ADDRESS_KEY_AREA, StringValuePtr(area));

	VALUE postalcode = rb_hash_aref(details, ID2SYM(rb_intern("postalcode")));
	if (!NIL_P(postalcode)) g_hash_table_insert(hashtable, GEOCLUE_ADDRESS_KEY_POSTALCODE, StringValuePtr(postalcode));

	VALUE street = rb_hash_aref(details, ID2SYM(rb_intern("street")));
	if (!NIL_P(street)) g_hash_table_insert(hashtable, GEOCLUE_ADDRESS_KEY_STREET, StringValuePtr(street));

	VALUE hash = rb_hash_new();
	double latitude, longitude, altitude;
	GeoclueAccuracy *accuracy;
	GError *error = NULL;

	GeocluePositionFields validity = geoclue_geocode_address_to_position(
		geocode, hashtable, &latitude, &longitude, &altitude, &accuracy, &error);

	if (validity & GEOCLUE_POSITION_FIELDS_LATITUDE) {
		rb_hash_aset(hash, ID2SYM(rb_intern("latitude")), rb_float_new(latitude));
	}
	if (validity & GEOCLUE_POSITION_FIELDS_LONGITUDE) {
		rb_hash_aset(hash, ID2SYM(rb_intern("longitude")), rb_float_new(longitude));
	}
	if (validity & GEOCLUE_POSITION_FIELDS_ALTITUDE) {
		rb_hash_aset(hash, ID2SYM(rb_intern("altitude")), rb_float_new(altitude));
	}

	return hash;
}

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
		geoclue_position_new(service, path);
		return Data_Wrap_Struct(rb_cGeocluePosition, NULL, NULL, geoclue_position_new(service, path));
	}
	return Qnil;
}

static VALUE position_new(VALUE klass, VALUE service, VALUE path)
{
	return Data_Wrap_Struct(rb_cGeocluePosition, NULL, NULL, geoclue_position_new(
		StringValuePtr(service),
		StringValuePtr(path)
	));
}

static VALUE position_get_position(VALUE self)
{
	GeocluePosition *position;
	int timestamp;
	double latitude, longitude, altitude;
	GError *error = NULL;
	Data_Get_Struct(self, GeocluePosition, position);
	GeocluePositionFields validity = geoclue_position_get_position(position, &timestamp, &latitude, &longitude, &altitude, NULL, &error);

	VALUE hash = rb_hash_new();

	rb_hash_aset(hash, rb_str_intern(rb_str_new2("timestamp")), INT2FIX(timestamp));
	if (validity & GEOCLUE_POSITION_FIELDS_LATITUDE) {
		rb_hash_aset(hash, ID2SYM(rb_intern("latitude")), rb_float_new(latitude));
	}
	if (validity & GEOCLUE_POSITION_FIELDS_LONGITUDE) {
		rb_hash_aset(hash, ID2SYM(rb_intern("longitude")), rb_float_new(longitude));
	}
	if (validity & GEOCLUE_POSITION_FIELDS_ALTITUDE) {
		rb_hash_aset(hash, ID2SYM(rb_intern("altitude")), rb_float_new(altitude));
	}

	return hash;
}

static VALUE provider_get_info(VALUE self)
{
	GeoclueProvider *provider;
	Data_Get_Struct(self, GeoclueProvider, provider);
	char *name, *description;
	GError *error = NULL;
	if (geoclue_provider_get_provider_info(provider, &name, &description, &error)) {
		VALUE hash = rb_hash_new();
		rb_hash_aset(hash, ID2SYM(rb_intern("name")), rb_tainted_str_new2(name));
		rb_hash_aset(hash, ID2SYM(rb_intern("description")), rb_tainted_str_new2(description));
		return hash;
	} else {
		return Qnil;
	}
}

static VALUE provider_get_status(VALUE self)
{
	GeoclueProvider *provider;
	GeoclueStatus status;
	GError *error = NULL;
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

	rb_define_module_function(rb_cGeoclueMaster, "get_default", master_get_default, 0);

	// Geoclue classes

	rb_cGeoclueMaster = rb_define_class_under(rb_mGeoclue, "Master", rb_cObject);
	rb_define_method(rb_cGeoclueMaster, "create_client", master_create_client, 0);

	rb_cGeoclueMasterClient = rb_define_class_under(rb_mGeoclue, "MasterClient", rb_cObject);
	rb_define_method(rb_cGeoclueMasterClient, "create_position", master_client_create_position, 0);
	rb_define_method(rb_cGeoclueMasterClient, "get_position_provider", master_client_get_position_provider, 0);

	rb_cGeoclueProvider = rb_define_class_under(rb_mGeoclue, "Provider", rb_cObject);
	rb_define_method(rb_cGeoclueProvider, "info", provider_get_info, 0);
	rb_define_method(rb_cGeoclueProvider, "status", provider_get_status, 0);

	// Geoclue::Provider classes

	rb_cGeoclueAddress = rb_define_class_under(rb_mGeoclue, "Address", rb_cGeoclueProvider);
	rb_define_singleton_method(rb_cGeoclueAddress, "new", address_new, 2);
	rb_define_method(rb_cGeoclueAddress, "details", address_get_address, 0);

	rb_cGeoclueGeocode = rb_define_class_under(rb_mGeoclue, "Geocode", rb_cGeoclueProvider);
	rb_define_singleton_method(rb_cGeoclueGeocode, "new", geocode_new, 2);
	rb_define_method(rb_cGeoclueGeocode, "position", geocode_address_to_position, 1);

	rb_cGeocluePosition = rb_define_class_under(rb_mGeoclue, "Position", rb_cGeoclueProvider);
	rb_define_singleton_method(rb_cGeocluePosition, "new", position_new, 2);
	rb_define_method(rb_cGeocluePosition, "details", position_get_position, 0);
}
