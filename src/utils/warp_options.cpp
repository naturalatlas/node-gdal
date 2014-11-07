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
	  dst_nodata(NULL)
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

int WarpOptions::parseResamplingAlg(Handle<Value> value){	
	if(value->IsUndefined() || value->IsNull()){
		options->eResampleAlg = GRA_NearestNeighbour;
		return 0;
	}
	if(!value->IsString()){
		NanThrowTypeError("resampling property must be a string");
		return 1;
	}
	std::string name = *NanUtf8String(value);

	if(name == "NearestNeighbor") {  options->eResampleAlg = GRA_NearestNeighbour; return 0; }
	if(name == "NearestNeighbour") { options->eResampleAlg = GRA_NearestNeighbour; return 0; }
	if(name == "Bilinear") {         options->eResampleAlg = GRA_Bilinear; return 0; }
	if(name == "Cubic") {            options->eResampleAlg = GRA_Cubic; return 0; }
	if(name == "CubicSpline") {      options->eResampleAlg = GRA_CubicSpline; return 0; }
	if(name == "Lanczos") {          options->eResampleAlg = GRA_Lanczos; return 0; }
	if(name == "Average") {          options->eResampleAlg = GRA_Average; return 0; }
	if(name == "Mode") {             options->eResampleAlg = GRA_Mode; return 0; }

	NanThrowError("Invalid resampling algorithm");
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
int WarpOptions::parse(Handle<Value> value)
{
	NanScope();

	if(!value->IsObject() || value->IsNull())
		NanThrowTypeError("Warp options must be an object");

	Handle<Object> obj = value.As<Object>();
	Handle<Value> prop;

	if(obj->HasOwnProperty(NanNew("options")) && additional_options.parse(obj->Get(NanNew("options")))){
		return 1; // error parsing string list
	}

	if(obj->HasOwnProperty(NanNew("memoryLimit"))){
		prop = obj->Get(NanNew("memoryLimit"));
		if(prop->IsNumber()){
			options->dfWarpMemoryLimit = prop->Int32Value();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("memoryLimit property must be an integer"); return 1;
		}
	}
	if(obj->HasOwnProperty(NanNew("resampling"))){
		prop = obj->Get(NanNew("resampling"));
		if(parseResamplingAlg(prop)){
			return 1; //error parsing resampling algorithm
		}
	}
	if(obj->HasOwnProperty(NanNew("src"))){
		prop = obj->Get(NanNew("src"));
		if(prop->IsObject() && !prop->IsNull() && NanHasInstance(Dataset::constructor, prop)){
			Dataset *ds = ObjectWrap::Unwrap<Dataset>(prop.As<Object>());
			options->hSrcDS = ds->getDataset();
			if(!options->hSrcDS){
				#if GDAL_VERSION_MAJOR < 2
				if(ds->getDatasource()) {
					NanThrowError("src dataset must be a raster dataset");
					return 1;
				}
				#endif
				NanThrowError("src dataset already closed");
				return 1;
			}
		} else {
			NanThrowTypeError("src property must be a Dataset object"); return 1;
		}
	} else {
		NanThrowError("Warp options must include a source dataset");
		return 1;
	}
	if(obj->HasOwnProperty(NanNew("dst"))){
		prop = obj->Get(NanNew("dst"));
		if(prop->IsObject() && !prop->IsNull() && NanHasInstance(Dataset::constructor, prop)){
			Dataset *ds = ObjectWrap::Unwrap<Dataset>(prop.As<Object>());
			options->hDstDS = ds->getDataset();
			if(!options->hDstDS){
				#if GDAL_VERSION_MAJOR < 2
				if(ds->getDatasource()) {
					NanThrowError("dst dataset must be a raster dataset");
					return 1;
				}
				#endif
				NanThrowError("dst dataset already closed");
				return 1;
			}
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("dst property must be a Dataset object"); return 1;
		}
	}
	if(obj->HasOwnProperty(NanNew("srcBands"))){
		prop = obj->Get(NanNew("srcBands"));
		if(src_bands.parse(prop)){
			return 1; //error parsing number list
		}
		options->panSrcBands = src_bands.get();
		options->nBandCount = src_bands.length();
	}
	if(obj->HasOwnProperty(NanNew("dstBands"))){
		prop = obj->Get(NanNew("dstBands"));
		if(dst_bands.parse(prop)){
			return 1; //error parsing number list
		}
		options->panDstBands = dst_bands.get();

		if (!options->panSrcBands) {
			NanThrowError("srcBands must be provided if dstBands option is used");
			return 1;
		}
		if(dst_bands.length() != options->nBandCount){
			NanThrowError("Number of dst bands must equal number of src bands");
			return 1; 
		}
	} 
	if (options->panSrcBands && !options->panDstBands) {
		NanThrowError("dstBands must be provided if srcBands option is used"); 
		return 1;
	}
	if(obj->HasOwnProperty(NanNew("srcNodata"))){
		prop = obj->Get(NanNew("srcNodata"));
		if(prop->IsNumber()){
			src_nodata = new double(prop->NumberValue());
			options->padfSrcNoDataReal = src_nodata;
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("srcNodata property must be a number"); return 1;
		}
	}
	if(obj->HasOwnProperty(NanNew("dstNodata"))){
		prop = obj->Get(NanNew("dstNodata"));
		if(prop->IsNumber()){
			dst_nodata = new double(prop->NumberValue());
			options->padfDstNoDataReal = dst_nodata;
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("dstNodata property must be a number"); return 1;
		}
	}
	if(obj->HasOwnProperty(NanNew("srcAlphaBand"))){
		prop = obj->Get(NanNew("srcAlphaBand"));
		if(prop->IsNumber()){
			options->nSrcAlphaBand = prop->Int32Value();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("srcAlphaBand property must be an integer"); return 1;
		}
	}
	if(obj->HasOwnProperty(NanNew("dstAlphaBand"))){
		prop = obj->Get(NanNew("dstAlphaBand"));
		if(prop->IsNumber()){
			options->nDstAlphaBand = prop->Int32Value();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("dstAlphaBand property must be an integer"); return 1;
		}
	}
	if(obj->HasOwnProperty(NanNew("blend"))){
		prop = obj->Get(NanNew("blend"));
		if(prop->IsNumber()){
			options->dfCutlineBlendDist = prop->NumberValue();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("cutline blend distance must be a number"); return 1;
		}
	}
	if(obj->HasOwnProperty(NanNew("cutline"))){
		prop = obj->Get(NanNew("cutline"));
		if(prop->IsObject() && !prop->IsNull() && NanHasInstance(Geometry::constructor, prop)){
			options->hCutline = ObjectWrap::Unwrap<Geometry>(prop.As<Object>())->get();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("cutline property must be a Geometry object"); return 1;
		}
	}
	return 0;
}

} //node_gdal namespace