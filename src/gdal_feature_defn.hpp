#ifndef __NODE_OGR_FEATURE_DEFN_H__
#define __NODE_OGR_FEATURE_DEFN_H__

// node
#include <node.h>
#include <node_object_wrap.h>

// nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

// ogr
#include <ogrsf_frmts.h>

using namespace v8;
using namespace node;

namespace node_gdal {

class FeatureDefn: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;
	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(OGRFeatureDefn *def);
	static Local<Value> New(OGRFeatureDefn *def, bool owned);
	static NAN_METHOD(toString);
	static NAN_METHOD(clone);

	static NAN_GETTER(fieldsGetter);
	static NAN_GETTER(nameGetter);
	static NAN_GETTER(geomTypeGetter);
	static NAN_GETTER(geomIgnoredGetter);
	static NAN_GETTER(styleIgnoredGetter);
	
	static NAN_SETTER(geomTypeSetter);
	static NAN_SETTER(geomIgnoredSetter);
	static NAN_SETTER(styleIgnoredSetter);

	FeatureDefn();
	FeatureDefn(OGRFeatureDefn *def);
	inline OGRFeatureDefn *get() {
		return this_;
	}
	inline bool isAlive(){
		return this_;
	}

private:
	~FeatureDefn();
	OGRFeatureDefn *this_;
	bool owned_;
};

}
#endif
