
#include "ogr_common.hpp"
#include "ogr_layer.hpp"
#include "ogr_feature.hpp"
#include "ogr_feature_defn.hpp"
#include "ogr_field_defn.hpp"
#include "ogr_spatial_reference.hpp"
#include "ogr_datasource.hpp"

#include <stdlib.h>
#include <sstream>

using namespace node_ogr;

Persistent<FunctionTemplate> Layer::constructor;
ObjectCache<OGRLayer*> Layer::cache;

void Layer::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Layer::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Layer"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "resetReading", resetReading);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getNextFeature", getNextFeature);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getLayerDefn", getLayerDefn);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFeature", getFeature);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFeatureCount", getFeatureCount);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setFeature", setFeature);
	NODE_SET_PROTOTYPE_METHOD(constructor, "createFeature", createFeature);
	NODE_SET_PROTOTYPE_METHOD(constructor, "deleteFeature", deleteFeature);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGeomType", getGeomType);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getName", getName);
	NODE_SET_PROTOTYPE_METHOD(constructor, "testCapability", testCapability);
	NODE_SET_PROTOTYPE_METHOD(constructor, "syncToDisk", syncToDisk);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getFIDColumn", getFIDColumn);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGeometryColumn", getGeometryColumn);
	NODE_SET_PROTOTYPE_METHOD(constructor, "createField", createField);

	ATTR(constructor, "ds", dsGetter, READ_ONLY_SETTER);
	ATTR(constructor, "srs", srsGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("Layer"), constructor->GetFunction());
}

Layer::Layer(OGRLayer *layer)
	: ObjectWrap(),
	  this_(layer),
	  parent_ds(0),
	  is_result_set(false)
{}

Layer::Layer()
	: ObjectWrap(),
	  this_(0),
	  parent_ds(0),
	  is_result_set(false)
{
}

Layer::~Layer()
{
	dispose();
}

void Layer::dispose()
{
	if (this_) {
		if (is_result_set && parent_ds && this_) {

#ifdef VERBOSE_GC
			printf("Releasing result set [%p] from datasource [%p]\n", this_, parent_ds);
#endif

			parent_ds->ReleaseResultSet(this_);
		}

#ifdef VERBOSE_GC
		printf("Disposing layer [%p]\n", this_);
#endif
		this_ = NULL;
	}
};

Handle<Value> Layer::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		Layer *f = static_cast<Layer *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create layer directly. Create with datasource instead.");
	}

	return args.This();
}

Handle<Value> Layer::New(OGRLayer *raw, OGRDataSource *parent)
{
	HandleScope scope;
	return scope.Close(Layer::New(raw, parent, false));
}

Handle<Value> Layer::New(OGRLayer *raw, OGRDataSource *parent, bool result_set)
{
	HandleScope scope;

	if (!raw) {
		return v8::Null();
	}
	if (cache.has(raw)) {
		return cache.get(raw);
	}

	Layer *wrapped = new Layer(raw);
	wrapped->parent_ds = parent;
	wrapped->is_result_set = result_set;

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Layer::constructor->GetFunction()->NewInstance(1, &ext);

	cache.add(raw, obj);

	//add reference to datasource so datasource doesnt get GC'ed while layer is alive
	if (parent) {
		Handle<Value> ds;
		if (Datasource::cache.has(parent)) {
			ds = Datasource::cache.get(parent);
		} else {
			ds = Datasource::New(parent); //this should never happen
		}
		obj->SetHiddenValue(String::NewSymbol("ds_"), ds);
	}

	return scope.Close(obj);
}

Handle<Value> Layer::toString(const Arguments& args)
{
	HandleScope scope;

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		return scope.Close(String::New("Null layer"));
	}

	std::ostringstream ss;
	ss << "Layer (" << layer->this_->GetName() << ")";

	return scope.Close(SafeString::New(ss.str().c_str()));
}


NODE_WRAPPED_METHOD(Layer, resetReading, ResetReading);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT(Layer, syncToDisk, SyncToDisk);
NODE_WRAPPED_METHOD_WITH_RESULT(Layer, getGeomType, Integer, GetGeomType);
NODE_WRAPPED_METHOD_WITH_RESULT(Layer, getName, SafeString, GetName);
NODE_WRAPPED_METHOD_WITH_RESULT(Layer, getFIDColumn, SafeString, GetFIDColumn);
NODE_WRAPPED_METHOD_WITH_RESULT(Layer, getGeometryColumn, SafeString, GetGeometryColumn);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_WRAPPED_PARAM(Layer, setFeature, SetFeature, Feature, "feature");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_WRAPPED_PARAM(Layer, createFeature, CreateFeature, Feature, "feature");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_INTEGER_PARAM(Layer, deleteFeature, DeleteFeature, "feature");
NODE_WRAPPED_METHOD_WITH_RESULT_1_STRING_PARAM(Layer, testCapability, Boolean, TestCapability, "capability");
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_WRAPPED_PARAM(Layer, createField, CreateField, FieldDefn, "feature");

Handle<Value> Layer::getLayerDefn(const Arguments& args)
{
	HandleScope scope;

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());

	if (!layer->this_) {
		return NODE_THROW("Layer object already destroyed");
	}

	return scope.Close(FeatureDefn::New(layer->this_->GetLayerDefn(), false));
}

Handle<Value> Layer::getNextFeature(const Arguments& args)
{
	HandleScope scope;

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		return NODE_THROW("Layer object already destroyed");
	}

	OGRFeature *next = layer->this_->GetNextFeature();

	if (next) {
		return scope.Close(Feature::New(next));
	}

	return Undefined();
}

Handle<Value> Layer::getFeature(const Arguments& args)
{
	HandleScope scope;
	int feature_id;

	NODE_ARG_INT(0, "feature id", feature_id);

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		return NODE_THROW("Layer object already destroyed");
	}

	OGRFeature *feature = layer->this_->GetFeature(feature_id);

	if (!feature) {
		return NODE_THROW("Error getting feature");
	}

	return scope.Close(Feature::New(feature));
}


Handle<Value> Layer::getFeatureCount(const Arguments& args)
{
	HandleScope scope;
	bool force = true;

	NODE_ARG_BOOL_OPT(0, "force", force);

	Layer *layer = ObjectWrap::Unwrap<Layer>(args.This());
	if (!layer->this_) {
		return NODE_THROW("Layer object already destroyed");
	}

	int count = layer->this_->GetFeatureCount(force);

	return scope.Close(Integer::New(count));
}

Handle<Value> Layer::dsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("ds_")));
}

Handle<Value> Layer::srsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Layer *layer = ObjectWrap::Unwrap<Layer>(info.This());
	if (!layer->this_) {
		return NODE_THROW("Layer object has already been destroyed");
	}
	return scope.Close(SpatialReference::New(layer->this_->GetSpatialRef(), false));
}