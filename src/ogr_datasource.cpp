
#include "ogr_common.hpp"
#include "ogr_layer.hpp"
#include "ogr_datasource.hpp"
#include "ogr_geometry.hpp"
#include "ogr_driver.hpp"

using namespace node_ogr;

Persistent<FunctionTemplate> Datasource::constructor;
ObjectCache<OGRDataSource*> Datasource::cache;

void Datasource::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Datasource::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Datasource"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getName", getName);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getDriver", getDriver);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getLayerByName", getLayerByName);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getLayerCount", getLayerCount);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getLayer", getLayer);
	NODE_SET_PROTOTYPE_METHOD(constructor, "deleteLayer", deleteLayer);
	NODE_SET_PROTOTYPE_METHOD(constructor, "testCapability", testCapability);
	NODE_SET_PROTOTYPE_METHOD(constructor, "executeSQL", executeSQL);
	NODE_SET_PROTOTYPE_METHOD(constructor, "syncToDisk", syncToDisk);
	NODE_SET_PROTOTYPE_METHOD(constructor, "createLayer", createLayer);
	NODE_SET_PROTOTYPE_METHOD(constructor, "copyLayer", copyLayer);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "releaseResultSet", releaseResultSet);
	NODE_SET_PROTOTYPE_METHOD(constructor, "destroy", destroy);

	target->Set(String::NewSymbol("Datasource"), constructor->GetFunction());
}

Datasource::Datasource(OGRDataSource *ds)
	: ObjectWrap(),
	  this_(ds),
	  result_sets()
{}

Datasource::Datasource()
	: ObjectWrap(),
	  this_(0),
	  result_sets()
{
}

Datasource::~Datasource()
{
	//Destroy at garbage collection time if not already explicitly destroyed
	dispose();
}

void Datasource::dispose()
{
	OGRLayer *lyr;
	Layer *lyr_wrapped;

	if (this_) {
		//dispose of all wrapped child layers
		int n = this_->GetLayerCount();
		for(int i = 0; i < n; i++) {
			lyr = this_->GetLayer(i);
			if (Layer::cache.has(lyr)) {
				lyr_wrapped = ObjectWrap::Unwrap<Layer>(Layer::cache.get(lyr));
				lyr_wrapped->dispose();
			}
		}

		//dispose of all result sets
		n = result_sets.size();
		for(int i = 0; i < n; i++) {
			lyr = result_sets[i];
			if (Layer::cache.has(lyr)) {
				lyr_wrapped = ObjectWrap::Unwrap<Layer>(Layer::cache.get(lyr));
				lyr_wrapped->dispose();
			}
		}
		result_sets.clear();

#ifdef VERBOSE_GC
		printf("Disposing datasource [%p]\n", this_);
#endif
		OGRDataSource::DestroyDataSource(this_);
		this_ = NULL;
	}
}


Handle<Value> Datasource::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		Datasource *f =  static_cast<Datasource *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create datasource directly. Create with driver instead.");
	}

	return args.This();
}

Handle<Value> Datasource::New(OGRDataSource *raw)
{
	HandleScope scope;

	if (!raw) {
		return v8::Null();
	}
	if (cache.has(raw)) {
		return cache.get(raw);
	}

	Datasource *wrapped = new Datasource(raw);
	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Datasource::constructor->GetFunction()->NewInstance(1, &ext);

	cache.add(raw, obj);

	return scope.Close(obj);
}

Handle<Value> Datasource::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("Datasource"));
}

NODE_WRAPPED_METHOD_WITH_RESULT(Datasource, getName, SafeString, GetName);
NODE_WRAPPED_METHOD_WITH_RESULT(Datasource, getLayerCount, Integer, GetLayerCount);
NODE_WRAPPED_METHOD_WITH_RESULT(Datasource, getDriver, node_ogr::Driver, GetDriver);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(Datasource, syncToDisk, SyncToDisk);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_INTEGER_PARAM(Datasource, deleteLayer, DeleteLayer, "layer index to delete");
NODE_WRAPPED_METHOD_WITH_RESULT_1_STRING_PARAM(Datasource, testCapability, Boolean, TestCapability, "capability to test");
//NODE_WRAPPED_METHOD_WITH_1_WRAPPED_PARAM(Datasource, releaseResultSet, ReleaseResultSet, Layer, "layer to release");

Handle<Value> Datasource::destroy(const Arguments& args)
{
	Datasource *ds = ObjectWrap::Unwrap<Datasource>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Datasource object already destroyed");
	}
	ds->dispose();
	return Undefined();
}

Handle<Value> Datasource::executeSQL(const Arguments& args)
{
	HandleScope scope;
	std::string sql;
	std::string sql_dialect;
	Geometry *spatial_filter = NULL;

	NODE_ARG_STR(0, "sql text", sql);
	NODE_ARG_WRAPPED_OPT(1, "spatial filter geometry", Geometry, spatial_filter);
	NODE_ARG_OPT_STR(2, "sql dialect", sql_dialect);

	Datasource *ds = ObjectWrap::Unwrap<Datasource>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Datasource object already destroyed");
	}

	OGRLayer *layer = ds->this_->ExecuteSQL(sql.c_str(),
											spatial_filter ? spatial_filter->get() : NULL,
											sql_dialect.empty() ? NULL : sql_dialect.c_str());

	if (layer) {
		ds->result_sets.push_back(layer);
		return scope.Close(Layer::New(layer, ds->this_, true));
	} else {
		return NODE_THROW("Error executing SQL");
	}
}


Handle<Value> Datasource::createLayer(const Arguments& args)
{
	HandleScope scope;
	std::string layer_name;
	std::string spatial_ref = "";
	OGRwkbGeometryType geom_type = wkbUnknown;
	Handle<Array> layer_options = Array::New(0);

	NODE_ARG_STR(0, "layer name", layer_name);
	NODE_ARG_OPT_STR(1, "spatial reference", spatial_ref);
	NODE_ARG_ENUM_OPT(2, "geometry type", OGRwkbGeometryType, geom_type);
	NODE_ARG_ARRAY_OPT(3, "layer creation options", layer_options);

	Datasource *ds = ObjectWrap::Unwrap<Datasource>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Datasource object already destroyed");
	}

	char **options = NULL;

	if (layer_options->Length() > 0) {
		options = new char* [layer_options->Length()];
		for (unsigned int i = 0; i < layer_options->Length(); ++i) {
			options[i] = TOSTR(layer_options->Get(i));
		}
	}

	OGRLayer *layer = ds->this_->CreateLayer(layer_name.c_str(),
					  NULL,
					  geom_type,
					  options);

	if (options) {
		delete [] options;
	}

	if (layer) {
		return scope.Close(Layer::New(layer, ds->this_, false));
	} else {
		return NODE_THROW("Error creating layer");
	}
}

Handle<Value> Datasource::getLayerByName(const Arguments& args)
{
	HandleScope scope;

	Datasource *ds = ObjectWrap::Unwrap<Datasource>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Datasource object already destroyed");
	}

	std::string name("");
	NODE_ARG_STR(0, "layer name", name);

	return scope.Close(Layer::New(ds->this_->GetLayerByName(name.c_str()), ds->this_));
}

Handle<Value> Datasource::getLayer(const Arguments& args)
{
	HandleScope scope;

	Datasource *ds = ObjectWrap::Unwrap<Datasource>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Datasource object already destroyed");
	}

	int i;
	NODE_ARG_INT(0, "layer index", i);

	return scope.Close(Layer::New(ds->this_->GetLayer(i), ds->this_));
}

Handle<Value> Datasource::copyLayer(const Arguments& args)
{
	HandleScope scope;
	Layer *layer_to_copy;
	std::string new_name = "";
	Handle<Array> layer_options = Array::New(0);

	NODE_ARG_WRAPPED(0, "layer to copy", Layer, layer_to_copy);
	NODE_ARG_STR(1, "new layer name", new_name);
	NODE_ARG_ARRAY_OPT(2, "layer creation options", layer_options);

	Datasource *ds = ObjectWrap::Unwrap<Datasource>(args.This());
	if (!ds->this_) {
		return NODE_THROW("Datasource object already destroyed");
	}

	char **options = NULL;

	if (layer_options->Length() > 0) {
		options = new char* [layer_options->Length()];
		for (unsigned int i = 0; i < layer_options->Length(); ++i) {
			options[i] = TOSTR(layer_options->Get(i));
		}
	}

	OGRLayer *layer = ds->this_->CopyLayer(layer_to_copy->get(),
										   new_name.c_str(),
										   options);

	if (options) {
		delete [] options;
	}

	if (layer) {
		return scope.Close(Layer::New(layer, ds->this_));
	} else {
		return NODE_THROW("Error copying layer");
	}
}