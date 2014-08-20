
#include "gdal_common.hpp"
#include "gdal_spatial_reference.hpp"
#include "gdal_coordinate_transformation.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> CoordinateTransformation::constructor;

void CoordinateTransformation::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(CoordinateTransformation::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("CoordinateTransformation"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "transformPoint", transformPoint);

	target->Set(String::NewSymbol("CoordinateTransformation"), constructor->GetFunction());
}

CoordinateTransformation::CoordinateTransformation(OGRCoordinateTransformation *transform)
	: ObjectWrap(),
	  this_(transform)
{
	LOG("Created CoordinateTransformation [%p]", transform);
}

CoordinateTransformation::CoordinateTransformation()
	: ObjectWrap(),
	  this_(0)
{
}

CoordinateTransformation::~CoordinateTransformation()
{
	if (this_) {
		LOG("Disposing CoordinateTransformation [%p]", this_);
		OGRCoordinateTransformation::DestroyCT(this_);
		LOG("Disposed CoordinateTransformation [%p]", this_);
		this_ = NULL;
	}
}

Handle<Value> CoordinateTransformation::New(const Arguments& args)
{
	HandleScope scope;
	CoordinateTransformation *f;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		f =  static_cast<CoordinateTransformation *>(ptr);
	} else {
		SpatialReference *source, *target;
		NODE_ARG_WRAPPED(0, "source", SpatialReference, source);
		NODE_ARG_WRAPPED(1, "target", SpatialReference, target);
		OGRCoordinateTransformation * transform = OGRCreateCoordinateTransformation(source->get(), target->get());
		if (!transform) {
			return NODE_THROW_LAST_CPLERR();
		}
		f = new CoordinateTransformation(transform);
	}

	f->Wrap(args.This());
	return args.This();
}

Handle<Value> CoordinateTransformation::New(OGRCoordinateTransformation *transform)
{
	v8::HandleScope scope;

	if (!transform) {
		return Null();
	}

	CoordinateTransformation *wrapped = new CoordinateTransformation(transform);

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = CoordinateTransformation::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> CoordinateTransformation::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("CoordinateTransformation"));
}


Handle<Value> CoordinateTransformation::transformPoint(const Arguments& args)
{
	HandleScope scope;

	CoordinateTransformation *transform = ObjectWrap::Unwrap<CoordinateTransformation>(args.This());

	double x, y, z = 0;

	if (args.Length() == 1 && args[0]->IsObject()) {
		Local<Object> obj = args[0]->ToObject();
		Local<Value> arg_x = obj->Get(String::NewSymbol("x"));
		Local<Value> arg_y = obj->Get(String::NewSymbol("y"));
		Local<Value> arg_z = obj->Get(String::NewSymbol("z"));
		if (!arg_x->IsNumber() || !arg_y->IsNumber()) {
			return NODE_THROW("point must contain numerical properties x and y")
		}
		x = static_cast<double>(arg_x->NumberValue());
		y = static_cast<double>(arg_y->NumberValue());
		if (arg_z->IsNumber()) {
			z = static_cast<double>(arg_z->NumberValue());
		}
	} else {
		NODE_ARG_DOUBLE(0, "x", x);
		NODE_ARG_DOUBLE(1, "y", y);
		NODE_ARG_DOUBLE_OPT(2, "z", z);
	}

	if (!transform->this_->Transform(1, &x, &y, &z)) {
		return NODE_THROW("Error transforming point");
	}

	Local<Object> result = Object::New();
	result->Set(String::NewSymbol("x"), Number::New(x));
	result->Set(String::NewSymbol("y"), Number::New(y));
	result->Set(String::NewSymbol("z"), Number::New(z));

	return scope.Close(result);
}

} // namespace node_gdal