#ifndef __NODE_OGR_FIELD_DEFN_H__
#define __NODE_OGR_FIELD_DEFN_H__

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

class FieldDefn: public Nan::ObjectWrap {
public:
	static Nan::Persistent<FunctionTemplate> constructor;
	static void Initialize(Local<Object> target);
	static NAN_METHOD(New);
	static Local<Value> New(OGRFieldDefn *def);
	static Local<Value> New(OGRFieldDefn *def, bool owned);
	static NAN_METHOD(toString);

	static NAN_GETTER(nameGetter);
	static NAN_GETTER(typeGetter);
	static NAN_GETTER(justificationGetter);
	static NAN_GETTER(precisionGetter);
	static NAN_GETTER(widthGetter);
	static NAN_GETTER(ignoredGetter);

	static NAN_SETTER(nameSetter);
	static NAN_SETTER(typeSetter);
	static NAN_SETTER(justificationSetter);
	static NAN_SETTER(precisionSetter);
	static NAN_SETTER(widthSetter);
	static NAN_SETTER(ignoredSetter);

	FieldDefn();
	FieldDefn(OGRFieldDefn *def);
	inline OGRFieldDefn *get() {
		return this_;
	}
	inline bool isAlive(){
		return this_;
	}

private:
	~FieldDefn();
	OGRFieldDefn *this_;
	bool owned_;
};

}
#endif
