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

namespace node_ogr {

	class FeatureDefn: public node::ObjectWrap {
	public:
		static Persistent<FunctionTemplate> constructor;
		static void Initialize(Handle<Object> target);
		static Handle<Value> New(const Arguments &args);
		static Handle<Value> New(OGRFeatureDefn *def);
		static Handle<Value> New(OGRFeatureDefn *def, bool owned);
		static Handle<Value> toString(const Arguments &args);
		static Handle<Value> getName(const Arguments &args);
		static Handle<Value> getGeomType(const Arguments &args);
		static Handle<Value> setGeomType(const Arguments &args);
		static Handle<Value> clone(const Arguments &args);
		static Handle<Value> isGeometryIgnored(const Arguments &args);
		static Handle<Value> setGeometryIgnored(const Arguments &args);
		static Handle<Value> isStyleIgnored(const Arguments &args);
		static Handle<Value> setStyleIgnored(const Arguments &args);

		static Handle<Value> fieldsGetter(Local<String> property, const AccessorInfo &info);

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
