#ifndef __NODE_OGR_FEATURE_H__
#define __NODE_OGR_FEATURE_H__

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

	class Feature: public node::ObjectWrap {
	public:
		static Persistent<FunctionTemplate> constructor;
		static void Initialize(Handle<Object> target);
		static Handle<Value> New(const Arguments &args);
		static Handle<Value> New(OGRFeature *feature);
		static Handle<Value> New(OGRFeature *feature, bool owned);
		static Handle<Value> toString(const Arguments &args);
		static Handle<Value> getDefn(const Arguments &args);
		static Handle<Value> getGeometry(const Arguments& args);
		static Handle<Value> setGeometryDirectly(const Arguments& args);
		static Handle<Value> setGeometry(const Arguments& args);
//  static Handle<Value> stealGeometry(const Arguments& args);
		static Handle<Value> clone(const Arguments& args);
		static Handle<Value> equal(const Arguments& args);
		static Handle<Value> getFieldDefn(const Arguments& args);
		static Handle<Value> getFID(const Arguments& args);
		static Handle<Value> setFID(const Arguments& args);
		static Handle<Value> setFrom(const Arguments& args);
		static Handle<Value> destroy(const Arguments& args);

		static Handle<Value> fieldsGetter(Local<String> property, const AccessorInfo &info);

		Feature();
		Feature(OGRFeature *geom);
		inline OGRFeature *get() {
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
