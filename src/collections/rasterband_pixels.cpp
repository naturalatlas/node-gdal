#include "../gdal_common.hpp"
#include "../gdal_rasterband.hpp"
#include "rasterband_pixels.hpp"
#include "../utils/typed_array.hpp"

#include <sstream>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> RasterBandPixels::constructor;

void RasterBandPixels::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(RasterBandPixels::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("RasterBandPixels").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "get", get);
	Nan::SetPrototypeMethod(lcons, "set", set);
	Nan::SetPrototypeMethod(lcons, "read", read);
	Nan::SetPrototypeMethod(lcons, "write", write);
	Nan::SetPrototypeMethod(lcons, "readBlock", readBlock);
	Nan::SetPrototypeMethod(lcons, "writeBlock", writeBlock);

	target->Set(Nan::New("RasterBandPixels").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

RasterBandPixels::RasterBandPixels()
	: Nan::ObjectWrap()
{}

RasterBandPixels::~RasterBandPixels()
{}

/**
 * A representation of a {{#crossLink "gdal.RasterBand"}}RasterBand{{/crossLink}}'s pixels.
 *
 * 
 * Note: Typed arrays should be created with an external ArrayBuffer for versions of node >= 0.11
 * ```
 * var n = 16*16;
 * var data = new Float32Array(new ArrayBuffer(n*4));
 * //read data into the existing array
 * band.pixels.read(0,0,16,16,data);```
 *
 * @class gdal.RasterBandPixels
 */
NAN_METHOD(RasterBandPixels::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		RasterBandPixels *f = static_cast<RasterBandPixels *>(ptr);
		f->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create RasterBandPixels directly");
		return;
	}
}

Local<Value> RasterBandPixels::New(Local<Value> band_obj)
{
	Nan::EscapableHandleScope scope;

	RasterBandPixels *wrapped = new RasterBandPixels();

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(RasterBandPixels::constructor)->GetFunction(), 1, &ext).ToLocalChecked();
	Nan::SetPrivate(obj, Nan::New("parent_").ToLocalChecked(), band_obj);

	return scope.Escape(obj);
}

NAN_METHOD(RasterBandPixels::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("RasterBandPixels").ToLocalChecked());
}

/**
 * Returns the value at the x, y coordinate.
 *
 * @method get
 * @param {Integer} x
 * @param {Integer} y
 * @return {Number}
 */
NAN_METHOD(RasterBandPixels::get)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int x, y;
	double val;

	NODE_ARG_INT(0, "x", x);
	NODE_ARG_INT(1, "y", y);

	CPLErr err = band->get()->RasterIO(GF_Read, x, y, 1, 1, &val, 1, 1, GDT_Float64, 0, 0);
	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	info.GetReturnValue().Set(Nan::New<Number>(val));
}

/**
 * Sets the value at the x, y coordinate.
 *
 * @method set
 * @param {Integer} x
 * @param {Integer} y
 * @param {Number} value
 */
NAN_METHOD(RasterBandPixels::set)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int x, y;
	double val;

	NODE_ARG_INT(0, "x", x);
	NODE_ARG_INT(1, "y", y);
	NODE_ARG_DOUBLE(2, "val", val);

	CPLErr err = band->get()->RasterIO(GF_Write, x, y, 1, 1, &val, 1, 1, GDT_Float64, 0, 0);
	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}

/**
 * Reads a region of pixels.
 *
 * @method read
 * @throws Error
 * @param {Integer} x
 * @param {Integer} y
 * @param {Integer} width
 * @param {Integer} height
 * @param {TypedArray} [data] The [TypedArray](https://developer.mozilla.org/en-US/docs/Web/API/ArrayBufferView#Typed_array_subclasses) to put the data in. A new array is created if not given.
 * @param {Object} [options]
 * @param {Integer} [options.buffer_width=x_size]
 * @param {Integer} [options.buffer_height=y_size]
 * @param {String} [options.data_type] See {{#crossLink "Constants (GDT)"}}GDT constants{{/crossLink}}.
 * @param {Integer} [options.pixel_space]
 * @param {Integer} [options.line_space]
 * @return {TypedArray} A [TypedArray](https://developer.mozilla.org/en-US/docs/Web/API/ArrayBufferView#Typed_array_subclasses) of values.
 */
NAN_METHOD(RasterBandPixels::read)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int x, y, w, h;
	int buffer_w, buffer_h;
	int bytes_per_pixel;
	int pixel_space, line_space;
	int size, length, min_size, min_length;
	void *data;
	Local<Value>  array;
	Local<Object> obj;
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

	if(info.Length() >= 5 && !info[4]->IsUndefined() && !info[4]->IsNull()) {
		NODE_ARG_OBJECT(4, "data", obj);
		type = TypedArray::Identify(obj);
		if(type == GDT_Unknown) {
			Nan::ThrowError("Invalid array");
			return;
		}
	}

	bytes_per_pixel = GDALGetDataTypeSize(type) / 8;
	pixel_space = bytes_per_pixel;
	NODE_ARG_INT_OPT(8, "pixel_space", pixel_space);
	line_space = pixel_space * buffer_w;
	NODE_ARG_INT_OPT(9, "line_space", line_space);

	if(pixel_space < bytes_per_pixel) {
		Nan::ThrowError("pixel_space must be greater than or equal to size of data_type");
		return;
	}
	if(line_space < pixel_space * buffer_w) {
		Nan::ThrowError("line_space must be greater than or equal to pixel_space * buffer_w");
		return;
	}

	size       = line_space * buffer_h; //bytes
	min_size   = size - (pixel_space - bytes_per_pixel); //subtract away padding on last pixel that wont be written
	length     = (size+bytes_per_pixel-1)/bytes_per_pixel;
	min_length = (min_size+bytes_per_pixel-1)/bytes_per_pixel;

	//create array if no array was passed
	if(obj.IsEmpty()){
		array = TypedArray::New(type, length);
		if(array.IsEmpty() || !array->IsObject()) {
			return; //TypedArray::New threw an error
		}
		obj = array.As<Object>();
	}

	data = TypedArray::Validate(obj, type, min_length);
	if(!data) {
		return; //TypedArray::Validate threw an error
	}

	CPLErr err = band->get()->RasterIO(GF_Read, x, y, w, h, data, buffer_w, buffer_h, type, pixel_space, line_space);
	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	info.GetReturnValue().Set(obj);
}

/**
 * Writes a region of pixels.
 *
 * @method write
 * @throws Error
 * @param {Integer} x
 * @param {Integer} y
 * @param {Integer} width
 * @param {Integer} height
 * @param {TypedArray} data The [TypedArray](https://developer.mozilla.org/en-US/docs/Web/API/ArrayBufferView#Typed_array_subclasses) to write to the band.
 * @param {Object} [options]
 * @param {Integer} [options.buffer_width=x_size]
 * @param {Integer} [options.buffer_height=y_size]
 * @param {Integer} [options.pixel_space]
 * @param {Integer} [options.line_space]
 */
NAN_METHOD(RasterBandPixels::write)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int x, y, w, h;
	int buffer_w, buffer_h;
	int bytes_per_pixel;
	int pixel_space, line_space;
	int size, min_size, min_length;
	void *data;
	Local<Object> passed_array;
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
		Nan::ThrowError("Invalid array");
		return;
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
		Nan::ThrowError("pixel_space must be greater than or equal to size of data_type");
		return;
	}
	if(line_space < pixel_space * buffer_w) {
		Nan::ThrowError("line_space must be greater than or equal to pixel_space * buffer_w");
		return;
	}

	data = TypedArray::Validate(passed_array, type, min_length);
	if(!data){
		return; //TypedArray::Validate threw an error
	}

	CPLErr err = band->get()->RasterIO(GF_Write, x, y, w, h, data, buffer_w, buffer_h, type, pixel_space, line_space);
	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}

/**
 * Reads a block of pixels.
 *
 * @method readBlock
 * @throws Error
 * @param {Integer} x
 * @param {Integer} y
 * @param {TypedArray} [data] The [TypedArray](https://developer.mozilla.org/en-US/docs/Web/API/ArrayBufferView#Typed_array_subclasses) to put the data in. A new array is created if not given.
 * @return {TypedArray} A [TypedArray](https://developer.mozilla.org/en-US/docs/Web/API/ArrayBufferView#Typed_array_subclasses) of values.
 */
NAN_METHOD(RasterBandPixels::readBlock)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int x, y, w = 0, h = 0;
	NODE_ARG_INT(0, "block_x_offset", x);
	NODE_ARG_INT(1, "block_y_offset", y);

	band->get()->GetBlockSize(&w, &h);

	GDALDataType type = band->get()->GetRasterDataType();

	Local<Value> array;
	Local<Object> obj;

	if(info.Length() == 3 && !info[2]->IsUndefined() && !info[2]->IsNull()) {
		NODE_ARG_OBJECT(2, "data", obj);
 		array = obj;
	} else {
		array = TypedArray::New(type, w * h);
		if(array.IsEmpty() || !array->IsObject()) {
			return; //TypedArray::New threw an error
		}
		obj = array.As<Object>();
	}


	void* data = TypedArray::Validate(obj, type, w*h);
	if(!data){
		return; //TypedArray::Validate threw an error
	}

	CPLErr err = band->get()->ReadBlock(x, y, data);
	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	info.GetReturnValue().Set(array);
}

/**
 * Writes a block of pixels.
 *
 * @method writeBlock
 * @throws Error
 * @param {Integer} x
 * @param {Integer} y
 * @param {TypedArray} data The [TypedArray](https://developer.mozilla.org/en-US/docs/Web/API/ArrayBufferView#Typed_array_subclasses) of values to write to the band.
 */
NAN_METHOD(RasterBandPixels::writeBlock)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	RasterBand *band = Nan::ObjectWrap::Unwrap<RasterBand>(parent);
	if (!band->isAlive()) {
		Nan::ThrowError("RasterBand object has already been destroyed");
		return;
	}

	int x, y, w = 0, h = 0;

	band->get()->GetBlockSize(&w, &h);

	NODE_ARG_INT(0, "block_x_offset", x);
	NODE_ARG_INT(1, "block_y_offset", y);

	Local<Object> obj;
	NODE_ARG_OBJECT(2, "data", obj);

	// validate array
	void* data = TypedArray::Validate(obj, band->get()->GetRasterDataType(), w*h);
	if(!data){
		return; //TypedArray::Validate threw an error
	}

	CPLErr err = band->get()->WriteBlock(x, y, data);

	if(err) {
		NODE_THROW_CPLERR(err);
		return;
	}

	return;
}


}