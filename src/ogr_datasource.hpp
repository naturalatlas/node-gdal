#ifndef __NODE_OGR_DATASOURCE_H__
#define __NODE_OGR_DATASOURCE_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// ogr
#include <ogrsf_frmts.h>

#include "obj_cache.hpp"
#include <vector>

using namespace v8;
using namespace node;

class Datasource: public node::ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor;
	static void Initialize(Handle<Object> target);
	static Handle<Value> New(const Arguments &args);
	static Handle<Value> New(OGRDataSource *ds);
	static Handle<Value> toString(const Arguments &args);
	static Handle<Value> getName(const Arguments &args);
	static Handle<Value> getDriver(const Arguments &args);
	static Handle<Value> getLayerCount(const Arguments &args);
	static Handle<Value> getLayerByName(const Arguments &args);
	static Handle<Value> getLayer(const Arguments &args);
	static Handle<Value> deleteLayer(const Arguments &args);
	static Handle<Value> testCapability(const Arguments &args);
	static Handle<Value> executeSQL(const Arguments &args);
	static Handle<Value> syncToDisk(const Arguments &args);
	static Handle<Value> createLayer(const Arguments &args);
	static Handle<Value> copyLayer(const Arguments &args);
	//static Handle<Value> releaseResultSet(const Arguments &args);
	static Handle<Value> destroy(const Arguments& args);

	static ObjectCache<OGRDataSource*> cache;

	Datasource();
	Datasource(OGRDataSource *ds);
	inline OGRDataSource *get() {
		return this_;
	}
	void dispose();

private:
	~Datasource();
	OGRDataSource *this_;
	std::vector<OGRLayer*> result_sets;
};

#endif
