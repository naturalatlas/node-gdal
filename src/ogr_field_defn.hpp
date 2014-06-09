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

namespace node_ogr {

	class FieldDefn: public node::ObjectWrap {
	public:
		static Persistent<FunctionTemplate> constructor;
		static void Initialize(Handle<Object> target);
		static Handle<Value> New(const Arguments &args);
		static Handle<Value> New(OGRFieldDefn *def);
		static Handle<Value> New(OGRFieldDefn *def, bool owned);
		static Handle<Value> toString(const Arguments &args);
		static Handle<Value> setName(const Arguments &args);
		static Handle<Value> getName(const Arguments &args);
		static Handle<Value> getType(const Arguments &args);
		static Handle<Value> setType(const Arguments &args);
		static Handle<Value> getJustify(const Arguments &args);
		static Handle<Value> setJustify(const Arguments &args);
		static Handle<Value> getWidth(const Arguments &args);
		static Handle<Value> setWidth(const Arguments &args);
		static Handle<Value> getPrecision(const Arguments &args);
		static Handle<Value> setPrecision(const Arguments &args);
		static Handle<Value> set(const Arguments &args);
		static Handle<Value> isIgnored(const Arguments &args);
		static Handle<Value> setIgnored(const Arguments &args);

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
