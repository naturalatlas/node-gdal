#include "../gdal_common.hpp"
#include "../gdal_geometry.hpp"
#include "../gdal_linestring.hpp"
#include "../gdal_point.hpp"
#include "linestring_points.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> LineStringPoints::constructor;

void LineStringPoints::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(LineStringPoints::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("LineStringPoints").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "count", count);
	Nan::SetPrototypeMethod(lcons, "get", get);
	Nan::SetPrototypeMethod(lcons, "set", set);
	Nan::SetPrototypeMethod(lcons, "add", add);
	Nan::SetPrototypeMethod(lcons, "reverse", reverse);
	Nan::SetPrototypeMethod(lcons, "resize", resize);

	target->Set(Nan::New("LineStringPoints").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

LineStringPoints::LineStringPoints()
	: Nan::ObjectWrap()
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
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		LineStringPoints *geom =  static_cast<LineStringPoints *>(ptr);
		geom->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create LineStringPoints directly");
		return;
	}
}

Local<Value> LineStringPoints::New(Local<Value> geom)
{
	Nan::EscapableHandleScope scope;

	LineStringPoints *wrapped = new LineStringPoints();

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(LineStringPoints::constructor)->GetFunction(), 1, &ext).ToLocalChecked();
	Nan::SetPrivate(obj, Nan::New("parent_").ToLocalChecked(), geom);

	return scope.Escape(obj);
}

NAN_METHOD(LineStringPoints::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("LineStringPoints").ToLocalChecked());
}

/**
 * Returns the number of points that are part of the line string.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(LineStringPoints::count)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	LineString *geom = Nan::ObjectWrap::Unwrap<LineString>(parent);

	info.GetReturnValue().Set(Nan::New<Integer>(geom->get()->getNumPoints()));
}

/**
 * Reverses the order of all the points.
 *
 * @method reverse
 */
NAN_METHOD(LineStringPoints::reverse)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	LineString *geom = Nan::ObjectWrap::Unwrap<LineString>(parent);

	geom->get()->reversePoints();

	return;
}

/**
 * Adjusts the number of points that make up the line string.
 *
 * @method resize
 * @param {Integer} count
 */
NAN_METHOD(LineStringPoints::resize)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	LineString *geom = Nan::ObjectWrap::Unwrap<LineString>(parent);

	int count;
	NODE_ARG_INT(0, "point count", count)
	geom->get()->setNumPoints(count);

	return;
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
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	LineString *geom = Nan::ObjectWrap::Unwrap<LineString>(parent);

	OGRPoint *pt = new OGRPoint();
	int i;

	NODE_ARG_INT(0, "index", i);
	if(i < 0 || i >= geom->get()->getNumPoints()) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	geom->get()->getPoint(i, pt);

	info.GetReturnValue().Set(Point::New(pt));
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
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	LineString *geom = Nan::ObjectWrap::Unwrap<LineString>(parent);

	int i;
	NODE_ARG_INT(0, "index", i);
	if(i < 0 || i >= geom->get()->getNumPoints()) {
		Nan::ThrowError("Point index out of range");
		return;
	}

	int n = info.Length() - 1;

	if(n == 0) {
		Nan::ThrowError("Point must be given");
		return;
	} else if(n == 1) {
		if(!info[1]->IsObject()) {
			Nan::ThrowError("Point or object expected for second argument");
			return;
		}
		if(IS_WRAPPED(info[1], Point)){
			//set from Point object
			Point* pt = Nan::ObjectWrap::Unwrap<Point>(info[1].As<Object>());
			geom->get()->setPoint(i, pt->get());
		} else {
			Local<Object> obj = info[1].As<Object>();
			//set from object {x: 0, y: 5}
			double x, y;
			NODE_DOUBLE_FROM_OBJ(obj, "x", x);
			NODE_DOUBLE_FROM_OBJ(obj, "y", y);

			Local<String> z_prop_name = Nan::New("z").ToLocalChecked();
			if (Nan::HasOwnProperty(obj, z_prop_name).FromMaybe(false)) {
				Local<Value> z_val = obj->Get(z_prop_name);
				if (!z_val->IsNumber()) {
					Nan::ThrowError("z property must be number");
					return;
				}
				geom->get()->setPoint(i, x, y, z_val->NumberValue());
			} else {
				geom->get()->setPoint(i, x, y);
			}
		}
	} else {
		//set x, y, z from numeric arguments
		if(!info[1]->IsNumber()){
			Nan::ThrowError("Number expected for second argument");
			return;
		}
		if(!info[2]->IsNumber()){
			Nan::ThrowError("Number expected for third argument");
			return;
		}
		if(n == 2){
			geom->get()->setPoint(i, info[1]->NumberValue(), info[2]->NumberValue());
		} else {
			if(!info[3]->IsNumber()){
				Nan::ThrowError("Number expected for fourth argument");
				return;
			}

			geom->get()->setPoint(i, info[1]->NumberValue(), info[2]->NumberValue(), info[3]->NumberValue());
		}
	}

	return;
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
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	LineString *geom = Nan::ObjectWrap::Unwrap<LineString>(parent);

	int n = info.Length();

	if(n == 0) {
		Nan::ThrowError("Point must be given");
		return;
	} else if(n == 1) {
		if(!info[0]->IsObject()) {
			Nan::ThrowError("Point, object, or array of points expected");
			return;
		}
		if(IS_WRAPPED(info[0], Point)){
			//set from Point object
			Point* pt = Nan::ObjectWrap::Unwrap<Point>(info[0].As<Object>());
			geom->get()->addPoint(pt->get());
		} else if (info[0]->IsArray()) {
			//set from array of points
			Local<Array> array = info[0].As<Array>();
			int length = array->Length();
			for (int i = 0; i < length; i++){
				Local<Value> element = array->Get(i);
				if(!element->IsObject()) {
					Nan::ThrowError("All points must be Point objects or objects");
					return;
				}
				Local<Object> element_obj = element.As<Object>();
				if(IS_WRAPPED(element_obj, Point)){
					//set from Point object
					Point* pt = Nan::ObjectWrap::Unwrap<Point>(element_obj);
					geom->get()->addPoint(pt->get());
				} else {
					//set from object {x: 0, y: 5}
					double x, y;
					NODE_DOUBLE_FROM_OBJ(element_obj, "x", x);
					NODE_DOUBLE_FROM_OBJ(element_obj, "y", y);

					Local<String> z_prop_name = Nan::New("z").ToLocalChecked();
					if (Nan::HasOwnProperty(element_obj, z_prop_name).FromMaybe(false)) {
						Local<Value> z_val = element_obj->Get(z_prop_name);
						if (!z_val->IsNumber()) {
							Nan::ThrowError("z property must be number");
							return;
						}
						geom->get()->addPoint(x, y, z_val->NumberValue());
					} else {
						geom->get()->addPoint(x, y);
					}
				}
			}
		} else {
			//set from object {x: 0, y: 5}
			Local<Object> obj = info[0].As<Object>();
			double x, y;
			NODE_DOUBLE_FROM_OBJ(obj, "x", x);
			NODE_DOUBLE_FROM_OBJ(obj, "y", y);

			Local<String> z_prop_name = Nan::New("z").ToLocalChecked();
			if (Nan::HasOwnProperty(obj, z_prop_name).FromMaybe(false)) {
				Local<Value> z_val = obj->Get(z_prop_name);
				if (!z_val->IsNumber()) {
					Nan::ThrowError("z property must be number");
					return;
				}
				geom->get()->addPoint(x, y, z_val->NumberValue());
			} else {
				geom->get()->addPoint(x, y);
			}
		}
	} else {
		//set x, y, z from numeric arguments
		if(!info[0]->IsNumber()){
			Nan::ThrowError("Number expected for first argument");
			return;
		}
		if(!info[1]->IsNumber()){
			Nan::ThrowError("Number expected for second argument");
			return;
		}
		if(n == 2){
			geom->get()->addPoint(info[0]->NumberValue(), info[1]->NumberValue());
		} else {
			if(!info[2]->IsNumber()){
				Nan::ThrowError("Number expected for third argument");
				return;
			}

			geom->get()->addPoint(info[0]->NumberValue(), info[1]->NumberValue(), info[2]->NumberValue());
		}
	}

	return;
}

} // namespace node_gdal