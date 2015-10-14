#ifndef __NODE_OGR_FEATURE_H__
#define __NODE_OGR_FEATURE_H__

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

class Feature: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;
	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(OGRFeature *feature);
	static Local<Value> New(OGRFeature *feature, bool owned);
	static NAN_METHOD(toString);
	static NAN_METHOD(getGeometry);
//	static NAN_METHOD(setGeometryDirectly);
	static NAN_METHOD(setGeometry);
//  static NAN_METHOD(stealGeometry);
	static NAN_METHOD(clone);
	static NAN_METHOD(equals);
	static NAN_METHOD(getFieldDefn);
	static NAN_METHOD(setFrom);
	static NAN_METHOD(destroy);

	static NAN_GETTER(fieldsGetter);
	static NAN_GETTER(fidGetter);
	static NAN_GETTER(defnGetter);

	static NAN_SETTER(fidSetter);

	Feature();
	Feature(OGRFeature *feature);
	inline OGRFeature *get() {
		return this_;
	}
	inline bool isAlive(){
		return this_;
	}
	void dispose();

private:
	~Feature();
	OGRFeature *this_;
	bool owned_;
	//int size_;
};

}
#endif
