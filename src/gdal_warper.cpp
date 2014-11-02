#include "gdal_warper.hpp"
#include "utils/warp_options.hpp"
#include "gdal_common.hpp"
#include "gdal_spatial_reference.hpp"

namespace node_gdal {


void Warper::Initialize(Handle<Object> target)
{
	NODE_SET_METHOD(target, "reprojectImage", reprojectImage);
}

NAN_METHOD(Warper::reprojectImage)
{
	NanScope();

	Handle<Object> obj;
	Handle<Value> prop;

	WarpOptions options;
	GDALWarpOptions* opts;
	std::string s_srs_str;
	std::string t_srs_str;
	char* s_srs = NULL;
	char* t_srs = NULL;
	double maxError = 0;

	if(args.Length() == 0) {
		NanThrowError("Warp options must be provided");
		NanReturnUndefined();
	}
	if(!args[0]->IsObject()) {
		NanThrowTypeError("Warp options must be an object");
		NanReturnUndefined();
	}
	obj = args[0].As<Object>();


	if(options.parse(args[0])){
		NanReturnUndefined(); // error parsing options object
	} else {
		opts = options.get();
	}
	if(!opts->hDstDS){
		NanThrowTypeError("dst Dataset must be provided");
		NanReturnUndefined();
	}
	if(obj->HasOwnProperty(NanNew("s_srs"))){
		prop = obj->Get(NanNew("s_srs"));
		if(prop->IsObject() && !prop->IsNull() && NanHasInstance(SpatialReference::constructor, prop)) {
			SpatialReference* srs = ObjectWrap::Unwrap<SpatialReference>(prop.As<Object>());
			
			OGRErr err = srs->get()->exportToWkt(&s_srs);
			if(err) {
				NODE_THROW_OGRERR(err);
				NanReturnUndefined();
			}
			s_srs_str = s_srs;
			CPLFree(s_srs);
			s_srs = (char*) s_srs_str.c_str();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("s_srs must be a SpatialReference object");
			NanReturnUndefined();
		}
	}
	if(obj->HasOwnProperty(NanNew("t_srs"))){
		prop = obj->Get(NanNew("t_srs"));
		if(prop->IsObject() && !prop->IsNull() && NanHasInstance(SpatialReference::constructor, prop)) {
			SpatialReference* srs = ObjectWrap::Unwrap<SpatialReference>(prop.As<Object>());
			
			OGRErr err = srs->get()->exportToWkt(&t_srs);
			if(err) {
				NODE_THROW_OGRERR(err);
				NanReturnUndefined();
			}
			t_srs_str = t_srs;
			CPLFree(t_srs);
			t_srs = (char*) t_srs_str.c_str();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("t_srs must be a SpatialReference object");
			NanReturnUndefined();
		}
	}
	if(obj->HasOwnProperty(NanNew("maxError"))){
		prop = obj->Get(NanNew("maxError"));
		if(prop->IsNumber()){
			maxError = prop->NumberValue();
		} else if (!prop->IsUndefined() && !prop->IsNull()) {
			NanThrowTypeError("maxError property must be a number");
			NanReturnUndefined();
		}
	}

	CPLErr err = GDALReprojectImage(opts->hSrcDS, s_srs, opts->hDstDS, t_srs, opts->eResampleAlg, opts->dfWarpMemoryLimit, maxError, NULL, NULL, opts);
	
	if(err) {
		NODE_THROW_CPLERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

} //node_gdal namespace