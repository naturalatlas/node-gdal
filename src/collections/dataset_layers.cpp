#include "../gdal_common.hpp"
#include "../gdal_dataset.hpp"
#include "../gdal_layer.hpp"
#include "../gdal_spatial_reference.hpp"
#include "dataset_layers.hpp"
#include "../utils/string_list.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> DatasetLayers::constructor;

void DatasetLayers::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(DatasetLayers::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("DatasetLayers").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "count", count);
	Nan::SetPrototypeMethod(lcons, "create", create);
	Nan::SetPrototypeMethod(lcons, "copy", copy);
	Nan::SetPrototypeMethod(lcons, "get", get);
	Nan::SetPrototypeMethod(lcons, "remove", remove);

	ATTR_DONT_ENUM(lcons, "ds", dsGetter, READ_ONLY_SETTER);

	target->Set(Nan::New("DatasetLayers").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

DatasetLayers::DatasetLayers()
	: Nan::ObjectWrap()
{}

DatasetLayers::~DatasetLayers()
{}

/**
 * An encapsulation of a {{#crossLink "gdal.Dataset"}}Dataset{{/crossLink}}'s vector layers.
 *
 * ```
 * var layers = dataset.layers;```
 *
 * @class gdal.DatasetLayers
 */
NAN_METHOD(DatasetLayers::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		DatasetLayers *f =  static_cast<DatasetLayers *>(ptr);
		f->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create DatasetLayers directly");
		return;
	}
}

Local<Value> DatasetLayers::New(Local<Value> ds_obj)
{
	Nan::EscapableHandleScope scope;

	DatasetLayers *wrapped = new DatasetLayers();

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(DatasetLayers::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	Nan::SetPrivate(obj, Nan::New("parent_").ToLocalChecked(), ds_obj);

	return scope.Escape(obj);
}

NAN_METHOD(DatasetLayers::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("DatasetLayers").ToLocalChecked());
}

/**
 * Returns the layer with the given name or identifier.
 *
 * @method get
 * @param {String|Integer} key Layer name or ID.
 * @return {gdal.Layer}
 */
NAN_METHOD(DatasetLayers::get)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(parent);
	
	if (!ds->isAlive()) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource(); 
		if(!ds->uses_ogr) {
			info.GetReturnValue().Set(Nan::Null());
			return;
		}
	#endif

	if(info.Length() < 1) {
		Nan::ThrowError("method must be given integer or string");
		return;
	}

	OGRLayer *lyr;

	if(info[0]->IsString()) {
		std::string layer_name = *Nan::Utf8String(info[0]);
		lyr = raw->GetLayerByName(layer_name.c_str());
	} else if(info[0]->IsNumber()) {
		lyr = raw->GetLayer(info[0]->IntegerValue());
	} else {
		Nan::ThrowTypeError("method must be given integer or string");
		return;
	}

	info.GetReturnValue().Set(Layer::New(lyr, raw));
}

/**
 * Adds a new layer.
 *
 * @example
 * ```
 * dataset.layers.create('layername', null, gdal.Point);
 * ```
 * 
 * @method create
 * @throws Error
 * @param {String} name Layer name
 * @param {gdal.SpatialReference|null} srs Layer projection
 * @param {Integer|Function} geomType Geometry type or constructor ({{#crossLink "Constants (wkbGeometryType)"}}see geometry types{{/crossLink}})
 * @param {string[]|object} creation_options driver-specific layer creation options
 * @return {gdal.Layer}
 */
NAN_METHOD(DatasetLayers::create)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(parent);
	
	if (!ds->isAlive()) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			Nan::ThrowError("Dataset does not support creating layers");
			return;
		}
	#endif

	std::string layer_name;
	SpatialReference *spatial_ref = NULL;
	OGRwkbGeometryType geom_type = wkbUnknown;
	StringList options;

	NODE_ARG_STR(0, "layer name", layer_name);
	NODE_ARG_WRAPPED_OPT(1, "spatial reference", SpatialReference, spatial_ref);
	NODE_ARG_ENUM_OPT(2, "geometry type", OGRwkbGeometryType, geom_type);
	if(info.Length() > 3 && options.parse(info[3])){
		return; //error parsing string list
	}

	OGRSpatialReference *srs = NULL;
	if(spatial_ref) srs = spatial_ref->get();

	OGRLayer *layer = raw->CreateLayer(layer_name.c_str(),
					  srs,
					  geom_type,
					  options.get());

	if (layer) {
		info.GetReturnValue().Set(Layer::New(layer, raw, false));
		return;
	} else {
		Nan::ThrowError("Error creating layer");
		return;
	}
}

/**
 * Returns the number of layers.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(DatasetLayers::count)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(parent);
	
	if (!ds->isAlive()) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			info.GetReturnValue().Set(Nan::New<Integer>(0));
			return;
		}
	#endif

	info.GetReturnValue().Set(Nan::New<Integer>(raw->GetLayerCount()));
}

/**
 * Copies a layer.
 *
 * @method copy
 * @param {String} src_lyr_name
 * @param {String} dst_lyr_name
 * @param {object|string[]} [options=null] layer creation options
 * @return {gdal.Layer}
 */
NAN_METHOD(DatasetLayers::copy)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(parent);
	
	if (!ds->isAlive()) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			Nan::ThrowError("Dataset does not support copying layers");
			return;
		}
	#endif

	Layer *layer_to_copy;
	std::string new_name = "";
	StringList options;

	NODE_ARG_WRAPPED(0, "layer to copy", Layer, layer_to_copy);
	NODE_ARG_STR(1, "new layer name", new_name);
	if(info.Length() > 2 && options.parse(info[2])){
		return; //error parsing string list
	}

	OGRLayer *layer = raw->CopyLayer(layer_to_copy->get(),
										   new_name.c_str(),
										   options.get());

	if (layer) {
		info.GetReturnValue().Set(Layer::New(layer, raw));
		return;
	} else {
		Nan::ThrowError("Error copying layer");
		return;
	}
}

/**
 * Removes a layer.
 *
 * @method remove
 * @throws Error
 * @param {Integer} index
 */
NAN_METHOD(DatasetLayers::remove)
{
	Nan::HandleScope scope;

	Local<Object> parent = Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked().As<Object>();
	Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(parent);
	
	if (!ds->isAlive()) {
		Nan::ThrowError("Dataset object has already been destroyed");
		return;
	}

	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			Nan::ThrowError("Dataset does not support removing layers");
			return;
		}
	#endif

	int i;
	NODE_ARG_INT(0, "layer index", i);
	OGRErr err = raw->DeleteLayer(i);
	if(err) {
		NODE_THROW_OGRERR(err);
		return;
	}

	return;
}

/**
 * Parent dataset
 *
 * @readOnly
 * @attribute ds
 * @type {gdal.Dataset}
 */
NAN_GETTER(DatasetLayers::dsGetter)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::GetPrivate(info.This(), Nan::New("parent_").ToLocalChecked()).ToLocalChecked());
}

} // namespace node_gdal
