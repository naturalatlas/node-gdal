
#include "ogr_common.hpp"
#include "ogr_layer.hpp"
#include "ogr_datasource.hpp"
#include "ogr_geometry.hpp"
#include "ogr_driver.hpp"
#include "collections/dataset_layers.hpp"

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
	NODE_SET_PROTOTYPE_METHOD(constructor, "testCapability", testCapability);
	NODE_SET_PROTOTYPE_METHOD(constructor, "executeSQL", executeSQL);
	NODE_SET_PROTOTYPE_METHOD(constructor, "syncToDisk", syncToDisk);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "releaseResultSet", releaseResultSet);
	NODE_SET_PROTOTYPE_METHOD(constructor, "destroy", destroy);

	ATTR(constructor, "layers", layersGetter, READ_ONLY_SETTER);

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

		Handle<Value> layers = DatasetLayers::New(args.This()); 
		args.This()->SetHiddenValue(String::NewSymbol("layers_"), layers); 

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
NODE_WRAPPED_METHOD_WITH_RESULT(Datasource, getDriver, node_ogr::Driver, GetDriver);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(Datasource, syncToDisk, SyncToDisk);
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

Handle<Value> Datasource::layersGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("layers_")));
}