#ifndef __NODE_OGR_FEATURE_DEFN_H__
#define __NODE_OGR_FEATURE_DEFN_H__

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

class FeatureDefn: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(OGRFeatureDefn *def);
	static Handle<Value> New(OGRFeatureDefn *def, bool owned);
	static Handle<Value> toString(const Arguments &args);
	static Handle<Value> clone(const Arguments &args);

	static Handle<Value> fieldsGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> nameGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> geomTypeGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> geomIgnoredGetter(Local<String> property, const AccessorInfo &info);
	static Handle<Value> styleIgnoredGetter(Local<String> property, const AccessorInfo &info);
	
	static void geomTypeSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void geomIgnoredSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
	static void styleIgnoredSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);

	FeatureDefn();
	FeatureDefn(OGRFeatureDefn *def);
	inline OGRFeatureDefn *get() {
		return this_;
	}

private:
	~FeatureDefn();
	OGRFeatureDefn *this_;
	bool owned_;
};

}
#endif
