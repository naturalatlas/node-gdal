#ifndef __NODE_GDAL_FIELD_COLLECTION_H__
#define __NODE_GDAL_FIELD_COLLECTION_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// gdal
#include <gdal_priv.h>

using namespace v8;
using namespace node;

class FieldCollection: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;

	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(Handle<Value> layer_obj);
	static Handle<Value> toString(const Arguments &args);
	static Handle<Value> toArray(const Arguments& args);
	static Handle<Value> toJSON(const Arguments& args);

	static Handle<Value> get(const Arguments &args);
	static Handle<Value> set(const Arguments &args);
	static Handle<Value> count(const Arguments &args);
	static Handle<Value> indexOf(const Arguments &args);

	static Handle<Value> get(OGRFeature *f, int field_index);
	static Handle<Value> getFieldAsIntegerList(OGRFeature* feature, int field_index);
	static Handle<Value> getFieldAsDoubleList(OGRFeature* feature, int field_index);
	static Handle<Value> getFieldAsStringList(OGRFeature* feature, int field_index);
	static Handle<Value> getFieldAsBinary(OGRFeature* feature, int field_index);
	static Handle<Value> getFieldAsDateTime(OGRFeature* feature, int field_index);

	FieldCollection();
private:
	~FieldCollection();
};

//determine field index based on string/numeric js argument

#define ARG_FIELD_ID(i, f, var) {                                      \
	if (args[i]->IsString()) {                                         \
		var = f->GetFieldIndex(TOSTR(args[i]));                        \
		if (field_index == -1) {                                       \
			return NODE_THROW("Specified field name does not exist");  \
		}                                                              \
	} else if (args[i]->IsInt32()) {                                   \
		var = args[i]->Int32Value();                                   \
		if (var < 0 || var >= f->GetFieldCount()) {                    \
			return NODE_THROW("Invalid field index");                  \
		}                                                              \
	} else {                                                           \
		return NODE_THROW("Field index must be integer or string");    \
	}                                                                  \
}

#endif
