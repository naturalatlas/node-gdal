#ifndef __NODE_GDAL_WARP_H__
#define __NODE_GDAL_WARP_H__

//node
#include <node.h>
#include <node_object_wrap.h>

//nan
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nan.h>
#pragma GCC diagnostic pop

//gdal
#include <gdal.h>

using namespace v8;
using namespace node;

extern struct GDALWarpAppOptions;

namespace node_gdal
{
	namespace apps
	{
		class ExtendedWarpAppOptions
		{
		public:
			ExtendedWarpAppOptions();
			~ExtendedWarpAppOptions();

			GDALWarpAppOptions* options;
			GDALDatasetH targetDataset = NULL;
			char* targetFilename;
			int sourceFilesCount = 0;
			GDALDatasetH* sourceFiles;
		};

		class WarpUtil : public node::ObjectWrap
		{
		private:
			static Nan::Persistent<FunctionTemplate> _constructor;
			static GDALDatasetH Warp(ExtendedWarpAppOptions* options);
            static ExtendedWarpAppOptions* SetOptions(Handle<Value> options);

		public:
			static void Initialize(Handle<Object> exports);
			static NAN_METHOD(New);
			static NAN_METHOD(Warp);
		};        
	}
}

#endif
