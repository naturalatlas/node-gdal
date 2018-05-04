#include "warp_options.hpp"
#include "../gdal_dataset.hpp"
#include "../gdal_geometry.hpp"
#include "../gdal_common.hpp"
#include <stdio.h>
namespace node_gdal {

WarpOptions::WarpOptions()
	: options(NULL), 
	  additional_options(), 
	  src_bands("src band ids"), 
	  dst_bands("dst band ids"),
	  src_nodata(NULL),
	  dst_nodata(NULL),
	  multi(false)
{
	options = GDALCreateWarpOptions();
}

WarpOptions::~WarpOptions()
{

	// Dont use: GDALDestroyWarpOptions( options ); - it assumes ownership of everything
	if(options) delete options;
	if(src_nodata) delete src_nodata;
	if(dst_nodata) delete dst_nodata;
}

int WarpOptions::parseResamplingAlg(Local<Value> value){	
	if(value->IsUndefined() || value->IsNull()){
		options->eResampleAlg = GRA_NearestNeighbour;
		return 0;
	}
	if(!value->IsString()){
		Nan::ThrowTypeError("resampling property must be a string");
		return 1;
	}
	std::string name = *Nan::Utf8String(value);

	if(name == "NearestNeighbor") {  options->eResampleAlg = GRA_NearestNeighbour; return 0; }
	if(name == "NearestNeighbour") { options->eResampleAlg = GRA_NearestNeighbour; return 0; }
	if(name == "Bilinear") {         options->eResampleAlg = GRA_Bilinear; return 0; }
	if(name == "Cubic") {            options->eResampleAlg = GRA_Cubic; return 0; }
	if(name == "CubicSpline") {      options->eResampleAlg = GRA_CubicSpline; return 0; }
	if(name == "Lanczos") {          options->eResampleAlg = GRA_Lanczos; return 0; }
	if(name == "Average") {          options->eResampleAlg = GRA_Average; return 0; }
	if(name == "Mode") {             options->eResampleAlg = GRA_Mode; return 0; }

	Nan::ThrowError("Invalid resampling algorithm");
	return 1;
}


/*
 * {
 *   options : string[] | object
 *   memoryLimit : int
 *   resampleAlg : string
 *   src: Dataset
 *   dst: Dataset
 *   srcBands: int | int[] 
 *   dstBands: int | int[]
 *   nBands: int
 *   srcAlphaBand: int
 *   dstAlphaBand: int
 *   srcNoData: double
 *   dstNoData: double
 *   cutline: geometry
 *   blend: double
 * }
 */
int WarpOptions::parse(Local<Value> value)
{
	Nan::HandleScope scope;

	if(!value->IsObject() || value->IsNull())
		Nan::ThrowTypeError("Warp options must be an object");

	Local<Object> obj = value.As<Object>();
	Local<Value> prop;

	if(Nan::HasOwnProperty(obj, Nan::New("options").ToLocalChecked()).FromMaybe(false) && additional_options.parse(obj->Get(Nan::New("options").ToLocalChecked()))){
		return 1; // error parsing string list
	}

	options->papszWarpOptions = additional_options.get();

	if(Nan::HasOwnProperty(obj, Nan::New("memoryLimit").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("memoryLimit").ToLocalChecked());
		if(prop->IsNumber()){
			options->dfWarpMemoryLimit = prop->Int32Value();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			Nan::ThrowTypeError("memoryLimit property must be an integer"); return 1;
		}
	}
	if(Nan::HasOwnProperty(obj, Nan::New("resampling").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("resampling").ToLocalChecked());
		if(parseResamplingAlg(prop)){
			return 1; //error parsing resampling algorithm
		}
	}
	if(Nan::HasOwnProperty(obj, Nan::New("src").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("src").ToLocalChecked());
		if(prop->IsObject() && !prop->IsNull() && Nan::New(Dataset::constructor)->HasInstance(prop)){
			Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(prop.As<Object>());
			options->hSrcDS = ds->getDataset();
			if(!options->hSrcDS){
				#if GDAL_VERSION_MAJOR < 2
				if(ds->getDatasource()) {
					Nan::ThrowError("src dataset must be a raster dataset");
					return 1;
				}
				#endif
				Nan::ThrowError("src dataset already closed");
				return 1;
			}
		} else {
			Nan::ThrowTypeError("src property must be a Dataset object"); return 1;
		}
	} else {
		Nan::ThrowError("Warp options must include a source dataset");
		return 1;
	}
	if(Nan::HasOwnProperty(obj, Nan::New("dst").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("dst").ToLocalChecked());
		if(prop->IsObject() && !prop->IsNull() && Nan::New(Dataset::constructor)->HasInstance(prop)){
			Dataset *ds = Nan::ObjectWrap::Unwrap<Dataset>(prop.As<Object>());
			options->hDstDS = ds->getDataset();
			if(!options->hDstDS){
				#if GDAL_VERSION_MAJOR < 2
				if(ds->getDatasource()) {
					Nan::ThrowError("dst dataset must be a raster dataset");
					return 1;
				}
				#endif
				Nan::ThrowError("dst dataset already closed");
				return 1;
			}
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			Nan::ThrowTypeError("dst property must be a Dataset object"); return 1;
		}
	}
	if(Nan::HasOwnProperty(obj, Nan::New("srcBands").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("srcBands").ToLocalChecked());
		if(src_bands.parse(prop)){
			return 1; //error parsing number list
		}
		options->panSrcBands = src_bands.get();
		options->nBandCount = src_bands.length();
	}
	if(Nan::HasOwnProperty(obj, Nan::New("dstBands").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("dstBands").ToLocalChecked());
		if(dst_bands.parse(prop)){
			return 1; //error parsing number list
		}
		options->panDstBands = dst_bands.get();

		if (!options->panSrcBands) {
			Nan::ThrowError("srcBands must be provided if dstBands option is used");
			return 1;
		}
		if(dst_bands.length() != options->nBandCount){
			Nan::ThrowError("Number of dst bands must equal number of src bands");
			return 1; 
		}
	} 
	if (options->panSrcBands && !options->panDstBands) {
		Nan::ThrowError("dstBands must be provided if srcBands option is used"); 
		return 1;
	}
	if(Nan::HasOwnProperty(obj, Nan::New("srcNodata").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("srcNodata").ToLocalChecked());
		if(prop->IsNumber()){
			src_nodata = new double(prop->NumberValue());
			options->padfSrcNoDataReal = src_nodata;
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			Nan::ThrowTypeError("srcNodata property must be a number"); return 1;
		}
	}
	if(Nan::HasOwnProperty(obj, Nan::New("dstNodata").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("dstNodata").ToLocalChecked());
		if(prop->IsNumber()){
			dst_nodata = new double(prop->NumberValue());
			options->padfDstNoDataReal = dst_nodata;
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			Nan::ThrowTypeError("dstNodata property must be a number"); return 1;
		}
	}
	if(Nan::HasOwnProperty(obj, Nan::New("srcAlphaBand").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("srcAlphaBand").ToLocalChecked());
		if(prop->IsNumber()){
			options->nSrcAlphaBand = prop->Int32Value();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			Nan::ThrowTypeError("srcAlphaBand property must be an integer"); return 1;
		}
	}
	if(Nan::HasOwnProperty(obj, Nan::New("dstAlphaBand").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("dstAlphaBand").ToLocalChecked());
		if(prop->IsNumber()){
			options->nDstAlphaBand = prop->Int32Value();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			Nan::ThrowTypeError("dstAlphaBand property must be an integer"); return 1;
		}
	}
	if(Nan::HasOwnProperty(obj, Nan::New("blend").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("blend").ToLocalChecked());
		if(prop->IsNumber()){
			options->dfCutlineBlendDist = prop->NumberValue();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			Nan::ThrowTypeError("cutline blend distance must be a number"); return 1;
		}
	}
	if(Nan::HasOwnProperty(obj, Nan::New("cutline").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("cutline").ToLocalChecked());
		if(prop->IsObject() && !prop->IsNull() && Nan::New(Geometry::constructor)->HasInstance(prop)){
			options->hCutline = Nan::ObjectWrap::Unwrap<Geometry>(prop.As<Object>())->get();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			Nan::ThrowTypeError("cutline property must be a Geometry object"); return 1;
		}
	}
	if(Nan::HasOwnProperty(obj, Nan::New("multi").ToLocalChecked()).FromMaybe(false)){
		prop = obj->Get(Nan::New("multi").ToLocalChecked());
		if(prop->IsTrue()){
			multi = true;
		}
	}
	return 0;
}

} //node_gdal namespace