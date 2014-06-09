#ifndef __NODE_OGR_LINEARRING_H__
#define __NODE_OGR_LINEARRING_H__

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

	class LinearRing: public node::ObjectWrap {

	public:
		static Persistent<FunctionTemplate> constructor;

		static void Initialize(Handle<Object> target);
		static Handle<Value> New(const Arguments &args);
		static Handle<Value> New(OGRLinearRing *geom);
		static Handle<Value> New(OGRLinearRing *geom, bool owned);
		static Handle<Value> toString(const Arguments &args);
		static Handle<Value> getArea(const Arguments &args);

		LinearRing();
		LinearRing(OGRLinearRing *geom);
		inline OGRLinearRing *get() {
			return this_;
		}

	private:
		~LinearRing();
		OGRLinearRing *this_;
		bool owned_;
		int size_;
	};

}

#endif
