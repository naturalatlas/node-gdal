#ifndef __NODE_OGR_FIELD_DEFN_H__
#define __NODE_OGR_FIELD_DEFN_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// ogr
#include <ogrsf_frmts.h>

using namespace v8;
using namespace node;

namespace node_gdal {

class FieldDefn: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(OGRFieldDefn *def);
	static Handle<Value> New(OGRFieldDefn *def, bool owned);
	static Handle<Value> toString(const Arguments &args);

	static Handle<Value> nameGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> typeGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> justificationGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> precisionGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> widthGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> ignoredGetter(Local<String> property, const AccessorInfo &info);

	static void nameSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void typeSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void justificationSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void precisionSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void widthSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void ignoredSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);

	FieldDefn();
	FieldDefn(OGRFieldDefn *def);
	inline OGRFieldDefn *get() {
		return this_;
	}

private:
	~FieldDefn();
	OGRFieldDefn *this_;
	bool owned_;
};

}
#endif
