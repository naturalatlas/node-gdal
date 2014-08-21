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
	NODE_SET_PROTOTYPE_METHOD(constructor, "readBlock", readBlock);
	NODE_SET_PROTOTYPE_METHOD(constructor, "writeBlock", writeBlock);

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
	int buffer_w, buffer_h;
	int bytes_per_pixel;
	int pixel_space, line_space;
	int size, length, min_size, min_length;
	void *data;
	Handle<Value>  array;
	Handle<Object> passed_array;
	GDALDataType type;


	NODE_ARG_INT(0, "x_offset", x);
	NODE_ARG_INT(1, "y_offset", y);
	NODE_ARG_INT(2, "x_size", w);
	NODE_ARG_INT(3, "y_size", h);

	std::string type_name = "";

	buffer_w = w;
	buffer_h = h;
	type     = band->get()->GetRasterDataType();
	NODE_ARG_INT_OPT(5, "buffer_width", buffer_w);
	NODE_ARG_INT_OPT(6, "buffer_height", buffer_h);
	NODE_ARG_OPT_STR(7, "data_type", type_name);
	if(!type_name.empty()) {
		type = GDALGetDataTypeByName(type_name.c_str());
	}

	if(args.Length() >= 5 && !args[4]->IsUndefined() && !args[4]->IsNull()) {
		NODE_ARG_OBJECT(4, "data", passed_array);
		type = TypedArray::Identify(passed_array);
		if(type == GDT_Unknown) return NODE_THROW("Invalid array");
	}

	bytes_per_pixel = GDALGetDataTypeSize(type) / 8;
	pixel_space = bytes_per_pixel;
	NODE_ARG_INT_OPT(8, "pixel_space", pixel_space);
	line_space = pixel_space * buffer_w;
	NODE_ARG_INT_OPT(9, "line_space", line_space);

	if(pixel_space < bytes_per_pixel) {
		return NODE_THROW("pixel_space must be greater than or equal to size of data_type");
	}
	if(line_space < pixel_space * buffer_w) {
		return NODE_THROW("line_space must be greater than or equal to pixel_space * buffer_w");
	}

	size       = line_space * buffer_h; //bytes
	min_size   = size - (pixel_space - bytes_per_pixel); //subtract away padding on last pixel that wont be written
	length     = (size+bytes_per_pixel-1)/bytes_per_pixel;
	min_length = (min_size+bytes_per_pixel-1)/bytes_per_pixel;

	//create array if no array was passed
	if(passed_array.IsEmpty()){
		array = TypedArray::New(type, length);
		if(array.IsEmpty() || !array->IsObject()) {
			return array; //TypedArray::New threw an error
		}
		data = TypedArray::Data(array->ToObject());
	} else {
		array = passed_array;
		if(TypedArray::Length(passed_array) < min_length) {
 			return NODE_THROW("Invalid array length");
 		}
 		data = TypedArray::Data(passed_array);
	}

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

	int x, y, w, h;
	int buffer_w, buffer_h;
	int bytes_per_pixel;
	int pixel_space, line_space;
	int size, min_size, min_length;
	void *data;
	Handle<Object> passed_array;
	GDALDataType type;

	NODE_ARG_INT(0, "x_offset", x);
	NODE_ARG_INT(1, "y_offset", y);
	NODE_ARG_INT(2, "x_size", w);
	NODE_ARG_INT(3, "y_size", h);
	NODE_ARG_OBJECT(4, "data", passed_array);

	buffer_w = w;
	buffer_h = h;
	NODE_ARG_INT_OPT(5, "buffer_width", buffer_w);
	NODE_ARG_INT_OPT(6, "buffer_height", buffer_h);

	type = TypedArray::Identify(passed_array);
	if(type == GDT_Unknown) {
		return NODE_THROW("Invalid array");
	}

	bytes_per_pixel = GDALGetDataTypeSize(type) / 8;
	pixel_space = bytes_per_pixel;
	NODE_ARG_INT_OPT(7, "pixel_space", pixel_space);
	line_space = pixel_space * buffer_w;
	NODE_ARG_INT_OPT(8, "line_space", line_space);

	size       = line_space * buffer_h; //bytes
	min_size   = size - (pixel_space - bytes_per_pixel); //subtract away padding on last pixel that wont be read
	min_length = (min_size+bytes_per_pixel-1)/bytes_per_pixel;

	if(pixel_space < bytes_per_pixel) {
		return NODE_THROW("pixel_space must be greater than or equal to size of data_type");
	}
	if(line_space < pixel_space * buffer_w) {
		return NODE_THROW("line_space must be greater than or equal to pixel_space * buffer_w");
	}
	if(TypedArray::Length(passed_array) < min_length) {
		return NODE_THROW("Invalid array length");
	}

	data = TypedArray::Data(passed_array);

	CPLErr err = band->get()->RasterIO(GF_Write, x, y, w, h, data, buffer_w, buffer_h, type, pixel_space, line_space);
	if(err) return NODE_THROW_CPLERR(err);

	return scope.Close(Undefined());
}

Handle<Value> RasterBandPixels::readBlock(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int x, y, w = 0, h = 0;
	NODE_ARG_INT(0, "block_x_offset", x);
	NODE_ARG_INT(1, "block_y_offset", y);

	band->get()->GetBlockSize(&w, &h);

	GDALDataType type = band->get()->GetRasterDataType();

	Handle<Value> array;

	if(args.Length() == 3 && !args[2]->IsUndefined() && !args[2]->IsNull()) {
		Handle<Object> obj;
		NODE_ARG_OBJECT(2, "data", obj);
		if(TypedArray::Identify(obj) != type) {
			return NODE_THROW("Array type does not match band data type");
		}
		if(TypedArray::Length(obj) < w*h) {
 			return NODE_THROW("Array length must be greater than or equal to blockSize.x * blockSize.y");
 		}
 		array = obj;
	} else {
		array = TypedArray::New(type, w * h);
		if(array.IsEmpty() || !array->IsObject()) {
			return array; //TypedArray::New threw an error
		}
	}

	void* data = TypedArray::Data(array->ToObject());

	CPLErr err = band->get()->ReadBlock(x, y, data);
	if(err) return NODE_THROW_CPLERR(err);

	return scope.Close(array);
}

Handle<Value> RasterBandPixels::writeBlock(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	RasterBand *band = ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->get()) {
		return NODE_THROW("RasterBand object has already been destroyed");
	}

	int x, y, w = 0, h = 0;

	band->get()->GetBlockSize(&w, &h);

	NODE_ARG_INT(0, "block_x_offset", x);
	NODE_ARG_INT(1, "block_y_offset", y);

	Handle<Object> obj;
	NODE_ARG_OBJECT(2, "data", obj);

	GDALDataType type = TypedArray::Identify(obj);

	if(type == GDT_Unknown || type != band->get()->GetRasterDataType())
		return NODE_THROW("Array type does not match band data type");
 	if(TypedArray::Length(obj) < w*h) {
 		return NODE_THROW("Array length must be greater than or equal to blockSize.x * blockSize.y");
 	}

	void* data = TypedArray::Data(obj);

	CPLErr err = band->get()->WriteBlock(x, y, data);
	if(err) return NODE_THROW_CPLERR(err);

	return scope.Close(Undefined());
}


}