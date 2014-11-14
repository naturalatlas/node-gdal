#include "../gdal_common.hpp"
#include "../gdal_geometry.hpp"
#include "../gdal_linestring.hpp"
#include "../gdal_point.hpp"
#include "linestring_points.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> LineStringPoints::constructor;

void LineStringPoints::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(LineStringPoints::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("LineStringPoints"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);
	NODE_SET_PROTOTYPE_METHOD(lcons, "set", set);
	NODE_SET_PROTOTYPE_METHOD(lcons, "add", add);
	NODE_SET_PROTOTYPE_METHOD(lcons, "reverse", reverse);
	NODE_SET_PROTOTYPE_METHOD(lcons, "resize", resize);

	target->Set(NanNew("LineStringPoints"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

LineStringPoints::LineStringPoints()
	: ObjectWrap()
{}

LineStringPoints::~LineStringPoints()
{}

/**
 * An encapsulation of a {{#crossLink "gdal.LineString"}}LineString{{/crossLink}}'s points.
 *
 * @class gdal.LineStringPoints
 */
NAN_METHOD(LineStringPoints::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		LineStringPoints *geom =  static_cast<LineStringPoints *>(ptr);
		geom->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create LineStringPoints directly");
		NanReturnUndefined();
	}
}

Handle<Value> LineStringPoints::New(Handle<Value> geom)
{
	NanEscapableScope();

	LineStringPoints *wrapped = new LineStringPoints();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(LineStringPoints::constructor)->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(NanNew("parent_"), geom);

	return NanEscapeScope(obj);
}

NAN_METHOD(LineStringPoints::toString)
{
	NanScope();
	NanReturnValue(NanNew("LineStringPoints"));
}

/**
 * Returns the number of points that are part of the line string.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(LineStringPoints::count)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	NanReturnValue(NanNew<Integer>(geom->get()->getNumPoints()));
}

/**
 * Reverses the order of all the points.
 *
 * @method reverse
 */
NAN_METHOD(LineStringPoints::reverse)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	geom->get()->reversePoints();

	NanReturnUndefined();
}

/**
 * Adjusts the number of points that make up the line string.
 *
 * @method resize
 * @param {Integer} count
 */
NAN_METHOD(LineStringPoints::resize)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	int count;
	NODE_ARG_INT(0, "point count", count)
	geom->get()->setNumPoints(count);

	NanReturnUndefined();
}

/**
 * Returns the point at the specified index.
 *
 * @method get
 * @param {Integer} index 0-based index
 * @return {gdal.Point}
 */
NAN_METHOD(LineStringPoints::get)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	OGRPoint *pt = new OGRPoint();
	int i;

	NODE_ARG_INT(0, "index", i);
	if(i < 0 || i >= geom->get()->getNumPoints()) {
		NanReturnValue(NanNull());
	}

	geom->get()->getPoint(i, pt);

	NanReturnValue(Point::New(pt));
}

/**
 * Sets the point at the specified index.
 *
 * @example
 * ```
 * lineString.points.set(0, new gdal.Point(1, 2));```
 *
 * @method set
 * @throws Error
 * @param {Integer} index 0-based index
 * @param {gdal.Point} point
 */
NAN_METHOD(LineStringPoints::set)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	int i;
	NODE_ARG_INT(0, "index", i);
	if(i < 0 || i >= geom->get()->getNumPoints()) {
		NanThrowError("Point index out of range");
		NanReturnUndefined();
	}

	int n = args.Length() - 1;

	if(n == 0) {
		NanThrowError("Point must be given");
		NanReturnUndefined();
	} else if(n == 1) {
		if(!args[1]->IsObject()) {
			NanThrowError("Point or object expected for second argument");
			NanReturnUndefined();
		}
		if(IS_WRAPPED(args[1], Point)){
			//set from Point object
			Point* pt = ObjectWrap::Unwrap<Point>(args[1].As<Object>());
			geom->get()->setPoint(i, pt->get());
		} else {
			Handle<Object> obj = args[1].As<Object>();
			//set from object {x: 0, y: 5}
			double x, y;
			NODE_DOUBLE_FROM_OBJ(obj, "x", x);
			NODE_DOUBLE_FROM_OBJ(obj, "y", y);

			Handle<String> z_prop_name = NanNew("z");
			if (obj->HasOwnProperty(z_prop_name)) {
				Handle<Value> z_val = obj->Get(z_prop_name);
				if (!z_val->IsNumber()) {
					NanThrowError("z property must be number");
					NanReturnUndefined();
				}
				geom->get()->setPoint(i, x, y, z_val->NumberValue());
			} else {
				geom->get()->setPoint(i, x, y);
			}
		}
	} else {
		//set x, y, z from numeric arguments
		if(!args[1]->IsNumber()){
			NanThrowError("Number expected for second argument");
			NanReturnUndefined();
		}
		if(!args[2]->IsNumber()){
			NanThrowError("Number expected for third argument");
			NanReturnUndefined();
		}
		if(n == 2){
			geom->get()->setPoint(i, args[1]->NumberValue(), args[2]->NumberValue());
		} else {
			if(!args[3]->IsNumber()){
				NanThrowError("Number expected for fourth argument");
				NanReturnUndefined();
			}

			geom->get()->setPoint(i, args[1]->NumberValue(), args[2]->NumberValue(), args[3]->NumberValue());
		}
	}

	NanReturnUndefined();
}

/**
 * Adds point(s) to the line string. Also accepts any object with an x and y property.
 *
 * @example
 * ```
 * lineString.points.add(new gdal.Point(1, 2));
 * lineString.points.add([
 *     new gdal.Point(1, 2)
 *     new gdal.Point(3, 4)
 * ]);```
 *
 * @method add
 * @throws Error
 * @param {gdal.Point|object|Array} point(s)
 */
NAN_METHOD(LineStringPoints::add)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	LineString *geom = ObjectWrap::Unwrap<LineString>(parent);

	int n = args.Length();

	if(n == 0) {
		NanThrowError("Point must be given");
		NanReturnUndefined();
	} else if(n == 1) {
		if(!args[0]->IsObject()) {
			NanThrowError("Point, object, or array of points expected");
			NanReturnUndefined();
		}
		if(IS_WRAPPED(args[0], Point)){
			//set from Point object
			Point* pt = ObjectWrap::Unwrap<Point>(args[0].As<Object>());
			geom->get()->addPoint(pt->get());
		} else if (args[0]->IsArray()) {
			//set from array of points
			Handle<Array> array = args[0].As<Array>();
			int length = array->Length();
			for (int i = 0; i < length; i++){
				Handle<Value> element = array->Get(i);
				if(!element->IsObject()) {
					NanThrowError("All points must be Point objects or objects");
					NanReturnUndefined();
				}
				Handle<Object> element_obj = element.As<Object>();
				if(IS_WRAPPED(element_obj, Point)){
					//set from Point object
					Point* pt = ObjectWrap::Unwrap<Point>(element_obj);
					geom->get()->addPoint(pt->get());
				} else {
					//set from object {x: 0, y: 5}
					double x, y;
					NODE_DOUBLE_FROM_OBJ(element_obj, "x", x);
					NODE_DOUBLE_FROM_OBJ(element_obj, "y", y);

					Handle<String> z_prop_name = NanNew("z");
					if (element_obj->HasOwnProperty(z_prop_name)) {
						Handle<Value> z_val = element_obj->Get(z_prop_name);
						if (!z_val->IsNumber()) {
							NanThrowError("z property must be number");
							NanReturnUndefined();
						}
						geom->get()->addPoint(x, y, z_val->NumberValue());
					} else {
						geom->get()->addPoint(x, y);
					}
				}
			}
		} else {
			//set from object {x: 0, y: 5}
			Handle<Object> obj = args[0].As<Object>();
			double x, y;
			NODE_DOUBLE_FROM_OBJ(obj, "x", x);
			NODE_DOUBLE_FROM_OBJ(obj, "y", y);

			Handle<String> z_prop_name = NanNew("z");
			if (obj->HasOwnProperty(z_prop_name)) {
				Handle<Value> z_val = obj->Get(z_prop_name);
				if (!z_val->IsNumber()) {
					NanThrowError("z property must be number");
					NanReturnUndefined();
				}
				geom->get()->addPoint(x, y, z_val->NumberValue());
			} else {
				geom->get()->addPoint(x, y);
			}
		}
	} else {
		//set x, y, z from numeric arguments
		if(!args[0]->IsNumber()){
			NanThrowError("Number expected for first argument");
			NanReturnUndefined();
		}
		if(!args[1]->IsNumber()){
			NanThrowError("Number expected for second argument");
			NanReturnUndefined();
		}
		if(n == 2){
			geom->get()->addPoint(args[0]->NumberValue(), args[1]->NumberValue());
		} else {
			if(!args[2]->IsNumber()){
				NanThrowError("Number expected for third argument");
				NanReturnUndefined();
			}

			geom->get()->addPoint(args[0]->NumberValue(), args[1]->NumberValue(), args[2]->NumberValue());
		}
	}

	NanReturnUndefined();
}

} // namespace node_gdal