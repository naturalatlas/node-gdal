
#include "gdal_common.hpp"
#include "gdal_feature.hpp"
#include "gdal_feature_defn.hpp"
#include "gdal_geometry.hpp"
#include "gdal_field_defn.hpp"
#include "gdal_layer.hpp"
#include "collections/feature_fields.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> Feature::constructor;

void Feature::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Feature::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("Feature"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getGeometry", getGeometry);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "setGeometryDirectly", setGeometryDirectly);
	NODE_SET_PROTOTYPE_METHOD(constructor, "setGeometry", setGeometry);
	// NODE_SET_PROTOTYPE_METHOD(constructor, "stealGeometry", stealGeometry);
	NODE_SET_PROTOTYPE_METHOD(constructor, "clone", clone);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "equals", equals);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "getFieldDefn", getFieldDefn); (use defn.fields.get() instead)
	NODE_SET_PROTOTYPE_METHOD(constructor, "setFrom", setFrom);

	//Note: We should let node GC handle destroying features when they arent being used
	//TODO: Give node more info on the amount of memory a feature is using
	//      V8::AdjustAmountOfExternalAllocatedMemory()  
	//NODE_SET_PROTOTYPE_METHOD(constructor, "destroy", destroy); 

	ATTR(constructor, "fields", fieldsGetter, READ_ONLY_SETTER);
	ATTR(constructor, "defn", defnGetter, READ_ONLY_SETTER);
	ATTR(constructor, "fid", fidGetter, fidSetter);

	target->Set(String::NewSymbol("Feature"), constructor->GetFunction());
}

Feature::Feature(OGRFeature *feature)
	: ObjectWrap(),
	  this_(feature),
	  owned_(true)
{}

Feature::Feature()
	: ObjectWrap(),
	  this_(0),
	  owned_(true)
{
}

Feature::~Feature()
{
	dispose();
}

void Feature::dispose()
{
	if (owned_ && this_) {
		OGRFeature::DestroyFeature(this_);
	}
	this_ = NULL;
}


Handle<Value> Feature::New(const Arguments& args)
{
	HandleScope scope;
	Feature* f;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		f = static_cast<Feature *>(ptr);

	} else {

		if (args.Length() < 1) {
			return NODE_THROW("Constructor expects Layer or FeatureDefn object");
		}

		Handle<Object> obj = args[0]->ToObject();
		OGRFeatureDefn *def;

		if (Layer::constructor->HasInstance(obj)) {
			Layer *layer = ObjectWrap::Unwrap<Layer>(obj);
			if (!layer->get()) {
				return NODE_THROW("Layer object already destroyed");
			}
			def = layer->get()->GetLayerDefn();
		} else if(FeatureDefn::constructor->HasInstance(obj)) {
			FeatureDefn *feature_def = ObjectWrap::Unwrap<FeatureDefn>(obj);
			if (!feature_def->get()) {
				return NODE_THROW("FeatureDefn object already destroyed");
			}
			def = feature_def->get();
		} else {
			return NODE_THROW("Constructor expects Layer or FeatureDefn object");
		}

		OGRFeature *ogr_f = new OGRFeature(def);
		f = new Feature(ogr_f);
	}

	Handle<Value> fields = FeatureFields::New(args.This()); 
	args.This()->SetHiddenValue(String::NewSymbol("fields_"), fields); 

	f->Wrap(args.This());
	return args.This();
}

Handle<Value> Feature::New(OGRFeature *feature)
{
	HandleScope scope;
	return scope.Close(Feature::New(feature, true));
}

Handle<Value> Feature::New(OGRFeature *feature, bool owned)
{
	HandleScope scope;

	if (!feature) {
		return Null();
	}

	Feature *wrapped = new Feature(feature);
	wrapped->owned_ = owned;
	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = Feature::constructor->GetFunction()->NewInstance(1, &ext);
	return scope.Close(obj);
}

Handle<Value> Feature::toString(const Arguments& args)
{
	return HandleScope().Close(String::New("Feature"));
}

Handle<Value> Feature::getGeometry(const Arguments& args)
{
	HandleScope scope;

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	OGRGeometry* geom = feature->this_->GetGeometryRef();
	if (!geom) {
		NODE_THROW("Error getting feature geometry");
	}


	return scope.Close(Geometry::New(geom, false));
}

Handle<Value> Feature::getFieldDefn(const Arguments& args)
{
	HandleScope scope;
	int field_index;
	NODE_ARG_INT(0, "field index", field_index);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (field_index < 0 || field_index >= feature->this_->GetFieldCount()) {
		return NODE_THROW("Invalid field index");
	}

	return scope.Close(FieldDefn::New(feature->this_->GetFieldDefnRef(field_index), false));
}
//NODE_WRAPPED_METHOD_WITH_RESULT(Feature, stealGeometry, Geometry, StealGeometry);
NODE_WRAPPED_METHOD_WITH_RESULT(Feature, clone, Feature, Clone);
NODE_WRAPPED_METHOD_WITH_OGRERR_RESULT_1_WRAPPED_PARAM(Feature, setGeometry, SetGeometry, Geometry, "geometry");
NODE_WRAPPED_METHOD_WITH_RESULT_1_WRAPPED_PARAM(Feature, equals, Boolean, Equal, Feature, "feature");

Handle<Value> Feature::destroy(const Arguments& args)
{
	HandleScope scope;
	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}
	feature->dispose();
	return Undefined();
}

Handle<Value> Feature::setGeometryDirectly(const Arguments& args)
{
	//- currently disabled
	//- uses setGeometry instead in case future versions support unowned geometry
	//Geometry objects in V8 must be the owner of the OGRGeometry in the current implementation
	//   to avoid geometry being destroyed when a feature is destroyed

	HandleScope scope;
	return scope.Close(Feature::setGeometry(args));

	/*
	Geometry *geom;
	NODE_ARG_WRAPPED(0, "geometry", Geometry, geom);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) return NODE_THROW("Feature object already destroyed");

	OGRErr err = feature->this_->SetGeometryDirectly(geom->get());

	if (err) return NODE_THROW_OGRERR(err);

	geom->owned_ = false;
	return Undefined();
	*/
}

Handle<Value> Feature::setFrom(const Arguments& args)
{
	HandleScope scope;
	Feature *other_feature;
	bool forgiving = true;
	Handle<Array> index_map;
	OGRErr err;

	NODE_ARG_WRAPPED(0, "feature", Feature, other_feature);

	Feature *feature = ObjectWrap::Unwrap<Feature>(args.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}

	if (args.Length() <= 2) {
		NODE_ARG_BOOL_OPT(1, "forgiving", forgiving);

		err = feature->this_->SetFrom(other_feature->this_, forgiving ? TRUE : FALSE);
	} else if (args.Length() > 2) {
		NODE_ARG_ARRAY(1, "index map", index_map);
		NODE_ARG_BOOL_OPT(2, "forgiving", forgiving);

		if (index_map->Length() < 1) {
			return NODE_THROW("index map must contain at least 1 index");
		}

		int *index_map_ptr = new int[index_map->Length()];

		for (unsigned index = 0; index < index_map->Length(); index++) {
			Handle<Value> field_index(index_map->Get(Integer::New(index)));

			if (!field_index->IsUint32()) {
				delete [] index_map_ptr;
				return NODE_THROW("index map must contain only integer values");
			}

			int val = (int)field_index->Uint32Value(); //todo: validate index? perhaps ogr already does this and throws an error

			index_map_ptr[index] = val;
		}

		err = feature->this_->SetFrom(other_feature->this_, index_map_ptr, forgiving ? TRUE : FALSE);

		delete [] index_map_ptr;
	}

	if (err) {
		return NODE_THROW_OGRERR(err);
	}
	return Undefined();
}

Handle<Value> Feature::fieldsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("fields_")));
}

Handle<Value> Feature::fidGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Feature *feature = ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}
	return scope.Close(Integer::New(feature->this_->GetFID()));
}

Handle<Value> Feature::defnGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	Feature *feature = ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->this_) {
		return NODE_THROW("Feature object already destroyed");
	}
	return scope.Close(FeatureDefn::New(feature->this_->GetDefnRef(), false));
}

void Feature::fidSetter(Local<String> property, Local<Value> value, const AccessorInfo &info)
{
	HandleScope scope;
	Feature *feature = ObjectWrap::Unwrap<Feature>(info.This());
	if (!feature->this_) {
		NODE_THROW("Feature object already destroyed");
		return;
	}
	if(!value->IsInt32()){
		NODE_THROW("fid must be an integer");
		return;
	}
	feature->this_->SetFID(value->IntegerValue());
}

} // namespace node_gdal