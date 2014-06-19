#include "../gdal_common.hpp"
#include "../gdal_dataset.hpp"
#include "../gdal_layer.hpp"
#include "dataset_layers.hpp"

Persistent<FunctionTemplate> DatasetLayers::constructor;

void DatasetLayers::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(DatasetLayers::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("DatasetLayers"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "create", create);
	NODE_SET_PROTOTYPE_METHOD(constructor, "copy", copy);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "remove", remove);

	ATTR(constructor, "ds", dsGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("DatasetLayers"), constructor->GetFunction());
}

DatasetLayers::DatasetLayers()
	: ObjectWrap()
{}

DatasetLayers::~DatasetLayers() 
{}

Handle<Value> DatasetLayers::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		DatasetLayers *f =  static_cast<DatasetLayers *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create DatasetLayers directly");
	}
}

Handle<Value> DatasetLayers::New(Handle<Value> ds_obj)
{
	HandleScope scope;

	DatasetLayers *wrapped = new DatasetLayers();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = DatasetLayers::constructor->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(String::NewSymbol("parent_"), ds_obj);

	return scope.Close(obj);
}

Handle<Value> DatasetLayers::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("DatasetLayers"));
}

Handle<Value> DatasetLayers::get(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	
	#if GDAL_MAJOR > 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr && raw) {
			return Null();
		}
	#endif

	if (!raw) {
		return NODE_THROW("Dataset object already destroyed");
	}

	if(args.Length() < 1) {
		return NODE_THROW("method must be given integer or string")
	}

	OGRLayer *lyr;
	
	if(args[0]->IsString()) {
		lyr = raw->GetLayerByName(TOSTR(args[0]));
	} else if(args[0]->IsNumber()) {
		lyr = raw->GetLayer(args[0]->IntegerValue());
	} else {
		return NODE_THROW("method must be given integer or string")
	}

	return scope.Close(Layer::New(lyr, ds));
}

Handle<Value> DatasetLayers::create(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);

	#if GDAL_MAJOR > 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			return NODE_THROW("Dataset does not support creating layers");
		}
	#endif

	if (!raw) {
		return NODE_THROW("Dataset object already destroyed");
	}

	std::string layer_name;
	std::string spatial_ref = "";
	OGRwkbGeometryType geom_type = wkbUnknown;
	Handle<Array> layer_options = Array::New(0);

	NODE_ARG_STR(0, "layer name", layer_name);
	NODE_ARG_OPT_STR(1, "spatial reference", spatial_ref);
	NODE_ARG_ENUM_OPT(2, "geometry type", OGRwkbGeometryType, geom_type);
	NODE_ARG_ARRAY_OPT(3, "layer creation options", layer_options);

	char **options = NULL;

	if (layer_options->Length() > 0) {
		options = new char* [layer_options->Length()];
		for (unsigned int i = 0; i < layer_options->Length(); ++i) {
			options[i] = TOSTR(layer_options->Get(i));
		}
	}

	OGRLayer *layer = raw->CreateLayer(layer_name.c_str(),
					  NULL,
					  geom_type,
					  options);

	if (options) {
		delete [] options;
	}

	if (layer) {
		return scope.Close(Layer::New(layer, ds, false));
	} else {
		return NODE_THROW("Error creating layer");
	}
}

Handle<Value> DatasetLayers::count(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);

	#if GDAL_MAJOR > 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr && raw) {
			return scope.Close(Integer::New(0));
		}
	#endif

	if (!raw) {
		return NODE_THROW("Dataset object already destroyed");
	}
	
	return scope.Close(Integer::New(raw->GetLayerCount()));
}

Handle<Value> DatasetLayers::copy(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);

	#if GDAL_MAJOR > 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			return NODE_THROW("Dataset does not support copying layers");
		}
	#endif

	if (!raw) {
		return NODE_THROW("Dataset object already destroyed");
	}

	Layer *layer_to_copy;
	std::string new_name = "";
	Handle<Array> layer_options = Array::New(0);

	NODE_ARG_WRAPPED(0, "layer to copy", Layer, layer_to_copy);
	NODE_ARG_STR(1, "new layer name", new_name);
	NODE_ARG_ARRAY_OPT(2, "layer creation options", layer_options);

	char **options = NULL;

	if (layer_options->Length() > 0) {
		options = new char* [layer_options->Length()];
		for (unsigned int i = 0; i < layer_options->Length(); ++i) {
			options[i] = TOSTR(layer_options->Get(i));
		}
	}

	OGRLayer *layer = raw->CopyLayer(layer_to_copy->get(),
										   new_name.c_str(),
										   options);

	if (options) {
		delete [] options;
	}

	if (layer) {
		return scope.Close(Layer::New(layer, ds));
	} else {
		return NODE_THROW("Error copying layer");
	}
}


Handle<Value> DatasetLayers::remove(const Arguments& args)
{
	HandleScope scope;

	Handle<Object> parent = args.This()->GetHiddenValue(String::NewSymbol("parent_"))->ToObject();
	Dataset *ds = ObjectWrap::Unwrap<Dataset>(parent);
	
	#if GDAL_MAJOR > 2
		GDALDataset *raw = ds->getDataset();
	#else
		OGRDataSource *raw = ds->getDatasource();
		if(!ds->uses_ogr) {
			return NODE_THROW("Dataset does not support removing layers");
		}
	#endif

	if (!raw) {
		return NODE_THROW("Dataset object already destroyed");
	}

	
	int i;
	NODE_ARG_INT(0, "layer index", i);
	OGRErr err = raw->DeleteLayer(i);
	if(err) {
		return NODE_THROW_OGRERR(err);
	}

	return Undefined();
}

Handle<Value> DatasetLayers::dsGetter(Local<String> property, const AccessorInfo &info)
{
	HandleScope scope;
	return scope.Close(info.This()->GetHiddenValue(String::NewSymbol("parent_")));
}