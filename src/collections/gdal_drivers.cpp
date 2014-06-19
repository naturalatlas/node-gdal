#include "../gdal_common.hpp"
#include "../gdal_driver.hpp"
#include "gdal_drivers.hpp"

Persistent<FunctionTemplate> GDALDrivers::constructor;

void GDALDrivers::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(GDALDrivers::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("GDALDrivers"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(constructor, "count", count);
	NODE_SET_PROTOTYPE_METHOD(constructor, "get", get);
	NODE_SET_PROTOTYPE_METHOD(constructor, "getNames", getNames);

	GDALAllRegister();
	#if GDAL_MAJOR < 2
	OGRRegisterAll();
	#endif

	target->Set(String::NewSymbol("GDALDrivers"), constructor->GetFunction());
}

GDALDrivers::GDALDrivers()
	: ObjectWrap()
{}

GDALDrivers::~GDALDrivers() 
{}

Handle<Value> GDALDrivers::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		GDALDrivers *f =  static_cast<GDALDrivers *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create GDALDrivers directly");
	}
}

Handle<Value> GDALDrivers::New()
{
	HandleScope scope;

	GDALDrivers *wrapped = new GDALDrivers();

	v8::Handle<v8::Value> ext = v8::External::New(wrapped);
	v8::Handle<v8::Object> obj = GDALDrivers::constructor->GetFunction()->NewInstance(1, &ext);

	return scope.Close(obj);
}

Handle<Value> GDALDrivers::toString(const Arguments& args)
{
	HandleScope scope;
	return scope.Close(String::New("GDALDrivers"));
}

Handle<Value> GDALDrivers::get(const Arguments& args)
{
	HandleScope scope;
	OGRSFDriver *ogr_driver;
	GDALDriver *gdal_driver;

	if (args.Length() == 0) {
		return NODE_THROW("Either driver name or index must be provided")
	}

	if(args[0]->IsString()){
		//try getting OGR driver first, and then GDAL driver if it fails
		//A driver named "VRT" exists for both GDAL and OGR, so if building
		//with <2.0 require user to specify which driver to pick
		std::string name = TOSTR(args[0]);

		#if GDAL_MAJOR < 2
		if(name == "VRT") {
			return NODE_THROW("Name \"VRT\" is ambiguous before GDAL 2.0. Use VRT:raster or VRT:vector instead");
		}
		ogr_driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(name.c_str());
		if(ogr_driver) {
			return scope.Close(Driver::New(ogr_driver));
		}
		#endif
		
		if(name == "VRT:vector") {
			#if GDAL_MAJOR < 2
			ogr_driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("VRT");
			if(ogr_driver) {
				return scope.Close(Driver::New(ogr_driver));
			}
			#else
				name = "VRT";
			#endif
		}

		if (name == "VRT:raster") {
			name = "VRT";
		}
		gdal_driver = GetGDALDriverManager()->GetDriverByName(name.c_str());
		if(gdal_driver) {
			return scope.Close(Driver::New(gdal_driver));
		}

	} else if(args[0]->IsNumber()) {
		int i = static_cast<int>(args[0]->IntegerValue());

		gdal_driver = GetGDALDriverManager()->GetDriver(i);
		if(gdal_driver) {
			return scope.Close(Driver::New(gdal_driver));
		}

		#if GDAL_MAJOR < 2
		i -= GetGDALDriverManager()->GetDriverCount();
		ogr_driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriver(i);
		if(ogr_driver) {
			return scope.Close(Driver::New(ogr_driver));
		}
		#endif
	} else {
		return NODE_THROW("Argument must be string or integer");
	}
		
	return NODE_THROW("Error retrieving driver");
}

Handle<Value> GDALDrivers::getNames(const Arguments& args)
{	
	HandleScope scope;
	int gdal_count = GetGDALDriverManager()->GetDriverCount();
	int i, ogr_count = 0;
	std::string name;

	#if GDAL_MAJOR < 2
		ogr_count = OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount();
	#endif
	
	int n = gdal_count + ogr_count;

	Local<Array> driver_names = Array::New(n);

	for (i = 0; i < gdal_count; ++i) {
		GDALDriver *driver = GetGDALDriverManager()->GetDriver(i);
		name = driver->GetDescription();
		#if GDAL_MAJOR < 2
		if(name == "VRT") name = "VRT:raster";
		#endif
		driver_names->Set(i, SafeString::New(name.c_str()));
	}

	for (; i < n; ++i) {
		OGRSFDriver *driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriver(i - gdal_count);
		name = driver->GetName();
		#if GDAL_MAJOR < 2
		if(name == "VRT") name = "VRT:vector";
		#endif
		driver_names->Set(i, SafeString::New(name.c_str()));
	} 
	return scope.Close(driver_names);
}

Handle<Value> GDALDrivers::count(const Arguments& args)
{
	HandleScope scope;

	int gdal_count = GetGDALDriverManager()->GetDriverCount();
	int ogr_count = OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount();

	return scope.Close(Integer::New(gdal_count + ogr_count));
}