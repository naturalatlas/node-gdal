#include "../gdal_common.hpp"
#include "../gdal_driver.hpp"
#include "gdal_drivers.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> GDALDrivers::constructor;

void GDALDrivers::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(GDALDrivers::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("GDALDrivers"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getNames", getNames);

	GDALAllRegister();
	#if GDAL_VERSION_MAJOR < 2
	OGRRegisterAll();
	#endif

	target->Set(NanNew("GDALDrivers"), lcons->GetFunction());
	
	NanAssignPersistent(constructor, lcons);
}

GDALDrivers::GDALDrivers()
	: ObjectWrap()
{}

GDALDrivers::~GDALDrivers()
{}

/**
 * An collection of all {{#crossLink "gdal.Driver"}}drivers{{/crossLink}} registered with GDAL.
 *
 * @class gdal.GDALDrivers
 */
NAN_METHOD(GDALDrivers::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		GDALDrivers *f =  static_cast<GDALDrivers *>(ptr);
		f->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create GDALDrivers directly");
		NanReturnUndefined();
	}
}

Handle<Value> GDALDrivers::New()
{
	NanEscapableScope();

	GDALDrivers *wrapped = new GDALDrivers();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(GDALDrivers::constructor)->GetFunction()->NewInstance(1, &ext);

	return NanEscapeScope(obj);
}

NAN_METHOD(GDALDrivers::toString)
{
	NanScope();
	NanReturnValue(NanNew("GDALDrivers"));
}

/**
 * Returns a driver with the specified name.
 *
 * Note: Prior to GDAL2.x there is a separate driver for vector VRTs and raster VRTs.
 *       Use `"VRT:vector"` to fetch the vector VRT driver and `"VRT:raster"` to fetch the raster VRT driver.
 * 
 * @method get
 * @param {Integer|string} index 0-based index or driver name
 * @return {gdal.Driver}
 */
NAN_METHOD(GDALDrivers::get)
{
	NanScope();
	
	#if GDAL_VERSION_MAJOR < 2
	OGRSFDriver *ogr_driver;
	#endif

	GDALDriver *gdal_driver;

	if (args.Length() == 0) {
		NanThrowError("Either driver name or index must be provided");
		NanReturnUndefined();
	}

	if(args[0]->IsString()){
		//try getting OGR driver first, and then GDAL driver if it fails
		//A driver named "VRT" exists for both GDAL and OGR, so if building
		//with <2.0 require user to specify which driver to pick
		std::string name = *NanUtf8String(args[0]);

		#if GDAL_VERSION_MAJOR < 2
		if(name == "VRT") {
			NanThrowError("Name \"VRT\" is ambiguous before GDAL 2.0. Use VRT:raster or VRT:vector instead");
			NanReturnUndefined();
		}
		ogr_driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(name.c_str());
		if(ogr_driver) {
			NanReturnValue(Driver::New(ogr_driver));
		}
		#endif

		if(name == "VRT:vector") {
			#if GDAL_VERSION_MAJOR < 2
			ogr_driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("VRT");
			if(ogr_driver) {
				NanReturnValue(Driver::New(ogr_driver));
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
			NanReturnValue(Driver::New(gdal_driver));
		}

	} else if(args[0]->IsNumber()) {
		int i = static_cast<int>(args[0]->IntegerValue());

		gdal_driver = GetGDALDriverManager()->GetDriver(i);
		if(gdal_driver) {
			NanReturnValue(Driver::New(gdal_driver));
		}

		#if GDAL_VERSION_MAJOR < 2
		i -= GetGDALDriverManager()->GetDriverCount();
		ogr_driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriver(i);
		if(ogr_driver) {
			NanReturnValue(Driver::New(ogr_driver));
		}
		#endif
	} else {
		NanThrowError("Argument must be string or integer");
		NanReturnUndefined();
	}

	NanReturnValue(NanNull());
}

/**
 * Returns an array with the names of all the drivers registered with GDAL.
 *
 * @method getNames
 * @return {string[]}
 */
NAN_METHOD(GDALDrivers::getNames)
{
	NanScope();
	int gdal_count = GetGDALDriverManager()->GetDriverCount();
	int i, ogr_count = 0;
	std::string name;

	#if GDAL_VERSION_MAJOR < 2
		ogr_count = OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount();
	#endif

	int n = gdal_count + ogr_count;

	Local<Array> driver_names = NanNew<Array>(n);

	for (i = 0; i < gdal_count; ++i) {
		GDALDriver *driver = GetGDALDriverManager()->GetDriver(i);
		name = driver->GetDescription();
		#if GDAL_VERSION_MAJOR < 2
		if(name == "VRT") name = "VRT:raster";
		#endif
		driver_names->Set(i, SafeString::New(name.c_str()));
	}


	#if GDAL_VERSION_MAJOR < 2
	for (; i < n; ++i) {
		OGRSFDriver *driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriver(i - gdal_count);
		name = driver->GetName();
		if(name == "VRT") name = "VRT:vector";
		driver_names->Set(i, SafeString::New(name.c_str()));
	}
	#endif
	
	NanReturnValue(driver_names);
}

/**
 * Returns the number of drivers registered with GDAL.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(GDALDrivers::count)
{
	NanScope();

	int count = GetGDALDriverManager()->GetDriverCount();

	#if GDAL_VERSION_MAJOR < 2
	count += OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount();
	#endif

	NanReturnValue(NanNew<Integer>(count));
}

} // namespace node_gdal