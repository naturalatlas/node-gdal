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
#include <gdalwarp_lib.cpp>

using namespace v8;
using namespace node;

namespace node_gdal
{
	namespace apps
	{
		class WarpUtil : public node::ObjectWrap
		{
		private:
			static Nan::Persistent<FunctionTemplate> _constructor;
            static int Warp(ExtendedWarpOptions* options);
            static ExtendedWarpOptions* SetOptions(Handle<Value> options);

		public:
			static void Initialize(Handle<Object> exports);
			static NAN_METHOD(New);
			static NAN_METHOD(Warp);
		};

        class ExtendedWarpAppOptions
		{
		public:
            ExtendedWarpAppOptions();
            ~ExtendedWarpAppOptions();

            GDALWarpAppOptions* options;
			GDALDatasetH targetDataset = NULL;
            char* destinationFilename;
            int sourceFilesCount = 0;
            GDALDatasetH* sourceFiles;
		};
	}
}

#endif
