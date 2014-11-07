#include "../gdal_common.hpp"
#include "../gdal_dataset.hpp"
#include "../gdal_layer.hpp"
#include "../gdal_spatial_reference.hpp"
#include "dataset_layers.hpp"
#include "../utils/string_list.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> DatasetLayers::constructor;

void DatasetLayers::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(DatasetLayers::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("DatasetLayers"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "create", create);
	NODE_SET_PROTOTYPE_METHOD(lcons, "copy", copy);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);
	NODE_SET_PROTOTYPE_METHOD(lcons, "remove", remove);

	ATTR_DONT_ENUM(lcons, "ds", dsGetter, READ_ONLY_SETTER);

	target->Set(NanNew("DatasetLayers"), lcons->GetFunction());
	
	NanAssignPersistent(constructor, lcons);
}

DatasetLayers::DatasetLayers()
	: ObjectWrap()
{}

DatasetLayers::~DatasetLayers() 
{}

NAN_METHOD(DatasetLayers::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		DatasetLayers *f =  static_cast<DatasetLayers *>(ptr);
		f->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create DatasetLayers directly");
		NanReturnUndefined();
	}
}

Handle<Value> DatasetLayers::New(Handle<Value> ds_obj)
{
	NanEscapableScope();

	DatasetLayers *wrapped = new DatasetLayers();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(DatasetLayers::constructor)->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(NanNew("parent_"), ds_obj);

	return NanEscapeScope(obj);
}

NAN_METHOD(DatasetLayers::toString)
{
	NanScope();
	NanReturnValue(NanNew("DatasetLayers"));
}

NAN_METHOD(DatasetLayers::get)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	
	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr && ds->getDataset()) {
			NanReturnNull();
		}
	#endif

	if (!raw) {
		NanThrowError("Dataset object already destroyed");
		NanReturnUndefined();
	}

	if(args.Length() < 1) {
		NanThrowError("method must be given integer or string");
		NanReturnUndefined();
	}

	OGRLayer *lyr;
	
	if(args[0]->IsString()) {
		NanUtf8String layer_name = NanUtf8String(args[0]);
		lyr = raw->GetLayerByName(*layer_name);
	} else if(args[0]->IsNumber()) {
		lyr = raw->GetLayer(args[0]->IntegerValue());
	} else {
		NanThrowTypeError("method must be given integer or string");
		NanReturnUndefined();
	}

	NanReturnValue(Layer::New(lyr, raw));
}

NAN_METHOD(DatasetLayers::create)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);

	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			NanThrowError("Dataset does not support creating layers");
			NanReturnUndefined();
		}
	#endif

	if (!raw) {
		NanThrowError("Dataset object already destroyed");
		NanReturnUndefined();
	}

	std::string layer_name;
	SpatialReference *spatial_ref = NULL;
	OGRwkbGeometryType geom_type = wkbUnknown;
	StringList options;

	NODE_ARG_STR(0, "layer name", layer_name);
	NODE_ARG_WRAPPED_OPT(1, "spatial reference", SpatialReference, spatial_ref);
	NODE_ARG_ENUM_OPT(2, "geometry type", OGRwkbGeometryType, geom_type);
	if(args.Length() > 3 && options.parse(args[3])){	
		NanReturnUndefined(); //error parsing string list
	}

	OGRSpatialReference *srs = NULL;
	if(spatial_ref) srs = spatial_ref->get();

	OGRLayer *layer = raw->CreateLayer(layer_name.c_str(),
					  srs,
					  geom_type,
					  options.get());

	if (layer) {
		NanReturnValue(Layer::New(layer, raw, false));
	} else {
		NanThrowError("Error creating layer");
		NanReturnUndefined();
	}
}

NAN_METHOD(DatasetLayers::count)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);

	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr && ds->getDataset()) {
			NanReturnValue(NanNew<Integer>(0));
		}
	#endif

	if (!raw) {
		NanThrowError("Dataset object already destroyed");
		NanReturnUndefined();
	}
	
	NanReturnValue(NanNew<Integer>(raw->GetLayerCount()));
}

NAN_METHOD(DatasetLayers::copy)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);

	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			NanThrowError("Dataset does not support copying layers");
			NanReturnUndefined();
		}
	#endif

	if (!raw) {
		NanThrowError("Dataset object already destroyed");
		NanReturnUndefined();
	}

	Layer *layer_to_copy;
	std::string new_name = "";
	StringList options;

	NODE_ARG_WRAPPED(0, "layer to copy", Layer, layer_to_copy);
	NODE_ARG_STR(1, "new layer name", new_name);
	if(args.Length() > 2 && options.parse(args[2])){
		NanReturnUndefined(); //error parsing string list
	}

	OGRLayer *layer = raw->CopyLayer(layer_to_copy->get(),
										   new_name.c_str(),
										   options.get());

	if (layer) {
		NanReturnValue(Layer::New(layer, raw));
	} else {
		NanThrowError("Error copying layer");
		NanReturnUndefined();
	}
}


NAN_METHOD(DatasetLayers::remove)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	
	#if GDAL_VERSION_MAJOR >= 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			NanThrowError("Dataset does not support removing layers");
			NanReturnUndefined();
		}
	#endif

	if (!raw) {
		NanThrowError("Dataset object already destroyed");
		NanReturnUndefined();
	}

	
	int i;
	NODE_ARG_INT(0, "layer index", i);
	OGRErr err = raw->DeleteLayer(i);
	if(err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

NAN_GETTER(DatasetLayers::dsGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("parent_")));
}

} // namespace node_gdal