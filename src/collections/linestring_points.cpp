#include "../gdal_common.hpp"
#include "../gdal_geometry.hpp"
#include "../gdal_linestring.hpp"
#include "../gdal_point.hpp"
#include "linestring_points.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> LineStringPoints::constructor;

void LineStringPoints::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LineStringPoints::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("LineStringPoints"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "set", set);
	NODE_SET_PROTOTYPE_METHOD(constructor, "add", add);
	NODE_SET_PROTOTYPE_METHOD(constructor, "reverse", reverse);
	NODE_SET_PROTOTYPE_METHOD(constructor, "resize", resize);

	target->Set(String::NewSymbol("LineStringPoints"), constructor->GetFunction());
}

LineStringPoints::LineStringPoints()
	: ObjectWrap()
{}

LineStringPoints::~LineStringPoints()
{}

Handle<Value> LineStringPoints::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		LineStringPoints *geom =  static_cast<LineStringPoints *>(ptr);
		geom->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create LineStringPoints directly");
	}
}

Handle<Value> LineStringPoints::New(Handle<Value> geom)
{
	HandleScope scope;

	LineStringPoints *wrapped = new LineStringPoints();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = LineStringPoints::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), geom);

	return scope.Close(obj);
}

Handle<Value> LineStringPoints::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("LineStringPoints"));
}

Handle<Value> LineStringPoints::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	return scope.Close(Integer::New(geom->get()->getNumPoints()));
}

Handle<Value> LineStringPoints::reverse(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	geom->get()->reversePoints();

	return Undefined();
}

Handle<Value> LineStringPoints::resize(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	int count;
	NODE_ARG_INT(0, "point count", count)
	geom->get()->setNumPoints(count);

	return Undefined();
}

Handle<Value> LineStringPoints::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	OGRPoint *pt = new OGRPoint();
	int i;

	NODE_ARG_INT(0, "index", i);
	if(i < 0 || i >= geom->get()->getNumPoints()) {
		return scope.Close(Null());
	}

	geom->get()->getPoint(i, pt);

	return scope.Close(Point::New(pt));
}

Handle<Value> LineStringPoints::set(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	int i;
	NODE_ARG_INT(0, "index", i);
	if(i < 0 || i >= geom->get()->getNumPoints()) {
		return NODE_THROW("Point index out of range");
	}

	int n = args.Length() - 1;

	if(n == 0) {
		return NODE_THROW("Point must be given");
	} else if(n == 1) {
		if(!args[1]->IsObject()) {
			return NODE_THROW("Point or object expected for second argument");
		}
		Handle<Object> obj = args[1]->ToObject();
		if(Point::constructor->HasInstance(obj)){
			//set from Point object
			Point* pt = ObjectWrap::Unwrap<Point>(obj);
			geom->get()->setPoint(i, pt->get());
		} else {
			//set from object {x: 0, y: 5}
			double x, y;
			NODE_DOUBLE_FROM_OBJ(obj, "x", x);
			NODE_DOUBLE_FROM_OBJ(obj, "y", y);

			Handle<String> z_prop_name = String::NewSymbol("z");
			if (obj->HasOwnProperty(z_prop_name)) {
				Handle<Value> z_val = obj->Get(z_prop_name);
				if (!z_val->IsNumber()) {
					return NODE_THROW("z property must be number");
				}
				geom->get()->setPoint(i, x, y, z_val->NumberValue());
			} else {
				geom->get()->setPoint(i, x, y);
			}
		}
	} else {
		//set x, y, z from numeric arguments
		if(!args[1]->IsNumber()){
			return NODE_THROW("Number expected for second argument");
		}
		if(!args[2]->IsNumber()){
			return NODE_THROW("Number expected for third argument");
		}
		if(n == 2){
			geom->get()->setPoint(i, args[1]->NumberValue(), args[2]->NumberValue());
		} else {
			if(!args[3]->IsNumber()){
				return NODE_THROW("Number expected for fourth argument");
			}

			geom->get()->setPoint(i, args[1]->NumberValue(), args[2]->NumberValue(), args[3]->NumberValue());
		}
	}

	return Undefined();
}

Handle<Value> LineStringPoints::add(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	int n = args.Length();

	if(n == 0) {
		return NODE_THROW("Point must be given");
	} else if(n == 1) {
		if(!args[0]->IsObject()) {
			return NODE_THROW("Point or object expected for second argument");
		}
		Handle<Object> obj = args[0]->ToObject();
		if(Point::constructor->HasInstance(obj)){
			//set from Point object
			Point* pt = ObjectWrap::Unwrap<Point>(obj);
			geom->get()->addPoint(pt->get());
		} else if (args[0]->IsArray()) {
			//set from array of points
			Handle<Array> array = Handle<Array>::Cast(args[0]);
			int length = array->Length();
			for (int i = 0; i < length; i++){
				Handle<Value> element = array->Get(i);
				if(!element->IsObject()) {
					return NODE_THROW("All points must be Point objects or objects");
				}
				Handle<Object> element_obj = element->ToObject();
				if(Point::constructor->HasInstance(element_obj)){
					//set from Point object
					Point* pt = ObjectWrap::Unwrap<Point>(element_obj);
					geom->get()->addPoint(pt->get());
				} else {
					//set from object {x: 0, y: 5}
					double x, y;
					NODE_DOUBLE_FROM_OBJ(element_obj, "x", x);
					NODE_DOUBLE_FROM_OBJ(element_obj, "y", y);

					Handle<String> z_prop_name = String::NewSymbol("z");
					if (element_obj->HasOwnProperty(z_prop_name)) {
						Handle<Value> z_val = element_obj->Get(z_prop_name);
						if (!z_val->IsNumber()) {
							return NODE_THROW("z property must be number");
						}
						geom->get()->addPoint(x, y, z_val->NumberValue());
					} else {
						geom->get()->addPoint(x, y);
					}
				}
			}
		} else {
			//set from object {x: 0, y: 5}
			double x, y;
			NODE_DOUBLE_FROM_OBJ(obj, "x", x);
			NODE_DOUBLE_FROM_OBJ(obj, "y", y);

			Handle<String> z_prop_name = String::NewSymbol("z");
			if (obj->HasOwnProperty(z_prop_name)) {
				Handle<Value> z_val = obj->Get(z_prop_name);
				if (!z_val->IsNumber()) {
					return NODE_THROW("z property must be number");
				}
				geom->get()->addPoint(x, y, z_val->NumberValue());
			} else {
				geom->get()->addPoint(x, y);
			}
		}
	} else {
		//set x, y, z from numeric arguments
		if(!args[0]->IsNumber()){
			return NODE_THROW("Number expected for first argument");
		}
		if(!args[1]->IsNumber()){
			return NODE_THROW("Number expected for second argument");
		}
		if(n == 2){
			geom->get()->addPoint(args[0]->NumberValue(), args[1]->NumberValue());
		} else {
			if(!args[2]->IsNumber()){
				return NODE_THROW("Number expected for third argument");
			}

			geom->get()->addPoint(args[0]->NumberValue(), args[1]->NumberValue(), args[2]->NumberValue());
		}
	}

	return Undefined();
}

} // namespace node_gdal