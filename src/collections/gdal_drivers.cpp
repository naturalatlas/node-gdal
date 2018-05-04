#include "../gdal_common.hpp"
#include "../gdal_driver.hpp"
#include "gdal_drivers.hpp"

namespace node_gdal {

Nan::Persistent<FunctionTemplate> GDALDrivers::constructor;

void GDALDrivers::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(GDALDrivers::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("GDALDrivers").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "toString", toString);
	Nan::SetPrototypeMethod(lcons, "count", count);
	Nan::SetPrototypeMethod(lcons, "get", get);
	Nan::SetPrototypeMethod(lcons, "getNames", getNames);

	GDALAllRegister();
	#if GDAL_VERSION_MAJOR < 2
	OGRRegisterAll();
	#endif

	target->Set(Nan::New("GDALDrivers").ToLocalChecked(), lcons->GetFunction());
	
	constructor.Reset(lcons);
}

GDALDrivers::GDALDrivers()
	: Nan::ObjectWrap()
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
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}
	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		GDALDrivers *f =  static_cast<GDALDrivers *>(ptr);
		f->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create GDALDrivers directly");
		return;
	}
}

Local<Value> GDALDrivers::New()
{
	Nan::EscapableHandleScope scope;

	GDALDrivers *wrapped = new GDALDrivers();

	v8::Local<v8::Value> ext = Nan::New<External>(wrapped);
	v8::Local<v8::Object> obj = Nan::NewInstance(Nan::New(GDALDrivers::constructor)->GetFunction(), 1, &ext).ToLocalChecked();

	return scope.Escape(obj);
}

NAN_METHOD(GDALDrivers::toString)
{
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New("GDALDrivers").ToLocalChecked());
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
	Nan::HandleScope scope;
	
	#if GDAL_VERSION_MAJOR < 2
	OGRSFDriver *ogr_driver;
	#endif

	GDALDriver *gdal_driver;

	if (info.Length() == 0) {
		Nan::ThrowError("Either driver name or index must be provided");
		return;
	}

	if(info[0]->IsString()){
		//try getting OGR driver first, and then GDAL driver if it fails
		//A driver named "VRT" exists for both GDAL and OGR, so if building
		//with <2.0 require user to specify which driver to pick
		std::string name = *Nan::Utf8String(info[0]);

		#if GDAL_VERSION_MAJOR < 2
		if(name == "VRT") {
			Nan::ThrowError("Name \"VRT\" is ambiguous before GDAL 2.0. Use VRT:raster or VRT:vector instead");
			return;
		}
		ogr_driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(name.c_str());
		if(ogr_driver) {
			info.GetReturnValue().Set(Driver::New(ogr_driver));
			return;
		}
		#endif

		if(name == "VRT:vector") {
			#if GDAL_VERSION_MAJOR < 2
			ogr_driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("VRT");
			if(ogr_driver) {
				info.GetReturnValue().Set(Driver::New(ogr_driver));
				return;
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
			info.GetReturnValue().Set(Driver::New(gdal_driver));
			return;
		}

	} else if(info[0]->IsNumber()) {
		int i = static_cast<int>(info[0]->IntegerValue());

		gdal_driver = GetGDALDriverManager()->GetDriver(i);
		if(gdal_driver) {
			info.GetReturnValue().Set(Driver::New(gdal_driver));
			return;
		}

		#if GDAL_VERSION_MAJOR < 2
		i -= GetGDALDriverManager()->GetDriverCount();
		ogr_driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriver(i);
		if(ogr_driver) {
			info.GetReturnValue().Set(Driver::New(ogr_driver));
			return;
		}
		#endif
	} else {
		Nan::ThrowError("Argument must be string or integer");
		return;
	}

	info.GetReturnValue().Set(Nan::Null());
}

/**
 * Returns an array with the names of all the drivers registered with GDAL.
 *
 * @method getNames
 * @return {string[]}
 */
NAN_METHOD(GDALDrivers::getNames)
{
	Nan::HandleScope scope;
	int gdal_count = GetGDALDriverManager()->GetDriverCount();
	int i, ogr_count = 0;
	std::string name;

	#if GDAL_VERSION_MAJOR < 2
		ogr_count = OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount();
	#endif

	int n = gdal_count + ogr_count;

	Local<Array> driver_names = Nan::New<Array>(n);

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
	
	info.GetReturnValue().Set(driver_names);
}

/**
 * Returns the number of drivers registered with GDAL.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(GDALDrivers::count)
{
	Nan::HandleScope scope;

	int count = GetGDALDriverManager()->GetDriverCount();

	#if GDAL_VERSION_MAJOR < 2
	count += OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount();
	#endif

	info.GetReturnValue().Set(Nan::New<Integer>(count));
}

} // namespace node_gdal