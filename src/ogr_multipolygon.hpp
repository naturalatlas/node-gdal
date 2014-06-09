#ifndef __NODE_OGR_MULTIPOLYGON_H__
#define __NODE_OGR_MULTIPOLYGON_H__

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// ogr
#include <ogrsf_frmts.h>
#include "ogr_geometry.hpp"

using namespace v8;
using namespace node;

namespace node_ogr {

	class MultiPolygon: public node::ObjectWrap {

	public:
		static Persistent<FunctionTemplate> constructor;

		static void Initialize(Handle<Object> target);
		static Handle<Value> New(const Arguments &args);
		static Handle<Value> New(OGRMultiPolygon *geom);
		static Handle<Value> New(OGRMultiPolygon *geom, bool owned);
		static Handle<Value> toString(const Arguments &args);
		static Handle<Value> unionCascaded(const Arguments &args);
		static Handle<Value> getArea(const Arguments &args);
		static Handle<Value> getGeometry(const Arguments &args);

		MultiPolygon();
		MultiPolygon(OGRMultiPolygon *geom);
		inline OGRMultiPolygon *get() {
			return this_;
		}

	private:
		~MultiPolygon();
		OGRMultiPolygon *this_;
		bool owned_;
		int size_;
	};

}

#endif
