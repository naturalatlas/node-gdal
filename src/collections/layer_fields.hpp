#ifndef __NODE_GDAL_LYR_FIELD_DEFN_COLLECTION_H__
#define __NODE_GDAL_LYR_FIELD_DEFN_COLLECTION_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// gdal
#include <gdal_priv.h>

using namespace v8;
using namespace node;

// Layer.fields : LayerFields

// Identical to FeatureDefn.fields object from the outside
// but on the inside it uses the parent layer
// to create/modify fields instead of illegally
// adding them directly to the layer definition  

class LayerFields: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;

	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(Handle<Value> layer_obj);
	static Handle<Value> toString(const Arguments &args);

	static Handle<Value> get(const Arguments &args);
	static Handle<Value> getNames(const Arguments &args);
	static Handle<Value> count(const Arguments &args);
	static Handle<Value> add(const Arguments &args);
	static Handle<Value> remove(const Arguments &args);
	static Handle<Value> indexOf(const Arguments &args);
	static Handle<Value> reorder(const Arguments &args);

	// - implement in the future -
	//static Handle<Value> alter(const Arguments &args);

	static Handle<Value> layerGetter(Local<String> property, const AccessorInfo &info);

	LayerFields();
private:
	~LayerFields();
};

#endif
