#include "../gdal_common.hpp"
#include "../gdal_rasterband.hpp"
#include "rasterband_pixels.hpp"
#include "../typed_array.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> RasterBandPixels::constructor;

void RasterBandPixels::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(RasterBandPixels::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("RasterBandPixels"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "set", set);
	NODE_SET_PROTOTYPE_METHOD(constructor, "read", read);
	NODE_SET_PROTOTYPE_METHOD(constructor, "write", write);
	
	target->Set(String::NewSymbol("RasterBandPixels"), constructor->GetFunction());
}

RasterBandPixels::RasterBandPixels()
	: ObjectWrap()
{}

RasterBandPixels::~RasterBandPixels() 
{}

Handle<Value> RasterBandPixels::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		RasterBandPixels *f = static_cast<RasterBandPixels *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create RasterBandPixels directly");
	}
}

Handle<Value> RasterBandPixels::New(Handle<Value> band_obj)
{
	HandleScope scope;

	RasterBandPixels *wrapped = new RasterBandPixels();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = RasterBandPixels::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), band_obj);

	return scope.Close(obj);
}

Handle<Value> RasterBandPixels::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("RasterBandPixels"));
}

Handle<Value> RasterBandPixels::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int x, y;
	double val;

	NODE_ARG_INT(0, "x", x);
	NODE_ARG_INT(1, "y", y);

	CPLErr err = band->get()->RasterIO(GF_Read, x, y, 1, 1, &val, 1, 1, GDT_Float64, 0, 0);
	if(err) return NODE_THROW_CPLERR(err);

	return scope.Close(Number::New(val));
}

Handle<Value> RasterBandPixels::set(const Arguments& args)
{
	HandleScope scope;
	
	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int x, y;
	double val;

	NODE_ARG_INT(0, "x", x);
	NODE_ARG_INT(1, "y", y);
	NODE_ARG_DOUBLE(2, "val", val);

	CPLErr err = band->get()->RasterIO(GF_Write, x, y, 1, 1, &val, 1, 1, GDT_Float64, 0, 0);
	if(err) return NODE_THROW_CPLERR(err);

	return scope.Close(Undefined());
}

Handle<Value> RasterBandPixels::read(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int x, y, w, h;
	NODE_ARG_INT(0, "x_offset", x);
	NODE_ARG_INT(1, "y_offset", y);
	NODE_ARG_INT(2, "x_size", w);
	NODE_ARG_INT(3, "y_size", h);

	std::string type_name = "";
	GDALDataType type = band->get()->GetRasterDataType();
	int buffer_w = w, buffer_h = h;

	NODE_ARG_INT_OPT(4, "buffer_x_size", buffer_w);
	NODE_ARG_INT_OPT(5, "buffer_y_size", buffer_h);
	NODE_ARG_OPT_STR(6, "data_type", type_name);
	if(!type_name.empty()) {
		type = GDALGetDataTypeByName(type_name.c_str());
	}

	int bytes_per_pixel = GDALGetDataTypeSize(type) / 8;
	int pixel_space = bytes_per_pixel;
	int line_space  = bytes_per_pixel * buffer_w;
	NODE_ARG_INT_OPT(7, "pixel_space", pixel_space);
	NODE_ARG_INT_OPT(8, "line_space", line_space);

	//allocate data
	int length = buffer_w * buffer_h;
	Handle<Value> array = TypedArray::New(type, length);
	if(array.IsEmpty() || !array->IsObject()) 
		return array; //TypedArray::New threw an error
	void* data = TypedArray::Data(array->ToObject());

	CPLErr err = band->get()->RasterIO(GF_Read, x, y, w, h, data, buffer_w, buffer_h, type, pixel_space, line_space);
	if(err) return NODE_THROW_CPLERR(err);

	return scope.Close(array);
}

Handle<Value> RasterBandPixels::write(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}


	Handle<Object> obj;
	NODE_ARG_OBJECT(0, "data", obj);

	GDALDataType type = TypedArray::Identify(obj);
	if(type == GDT_Unknown) return NODE_THROW("Invalid array");

	int length = TypedArray::Length(obj);

	int x, y, w, h;
	NODE_ARG_INT(1, "x_offset", x);
	NODE_ARG_INT(2, "y_offset", y);
	NODE_ARG_INT(3, "x_size", w);
	NODE_ARG_INT(4, "y_size", h);

	int buffer_w = w, buffer_h = h;
	NODE_ARG_INT_OPT(6, "buffer_x_size", buffer_w);
	NODE_ARG_INT_OPT(7, "buffer_y_size", buffer_h);

	int bytes_per_pixel = GDALGetDataTypeSize(type) / 8;
	int pixel_space = bytes_per_pixel;
	int line_space  = bytes_per_pixel * buffer_w;
	NODE_ARG_INT_OPT(8, "pixel_space", pixel_space);
	NODE_ARG_INT_OPT(9, "line_space", line_space);

	if(length != buffer_w*buffer_h) {
		return NODE_THROW("Mismatch between array length and buffer_x_size, buffer_y_size arguments");
	}

	void* data = TypedArray::Data(obj);

	CPLErr err = band->get()->RasterIO(GF_Write, x, y, w, h, data, buffer_w, buffer_h, type, pixel_space, line_space);
	if(err) return NODE_THROW_CPLERR(err);

	return scope.Close(Undefined());
}

}