#include "warputil.hpp"

namespace node_gdal
{
	namespace apps
	{
		ExtendedWarpAppOptions::ExtendedWarpAppOptions()
		{
			this->options = GDALWarpAppOptionsNew(NULL, NULL);
		}

		ExtendedWarpAppOptions::~ExtendedWarpAppOptions()
		{
			GDALWarpAppOptionsFree(this->options);
		}

		Nan::Persistent<FunctionTemplate> WarpUtil::_constructor;

		void WarpUtil::Initialize(Local<Object> target)
		{
			Nan::HandleScope scope;

			Local<FunctionTemplate> fTemplate = Nan::New<FunctionTemplate>(WarpUtil::New);
			fTemplate->SetClassName(Nan::New("WarpUtil").ToLocalChecked());
			fTemplate->InstanceTemplate()->SetInternalFieldCount(1);

			Nan::SetPrototypeMethod(fTemplate, "warp", Warp);

			target->Set(Nan::New("WarpUtil").ToLocalChecked(), fTemplate->GetFunction());

			_constructor.Reset(fTemplate);
		}

		NAN_METHOD(WarpUtil::New)
		{
			Nan::HandleScope scope;

			if (info.IsConstructCall())
			{
				info.GetReturnValue().Set(info.This());
				return;
			}
			else
			{
				Nan::ThrowError("use the new keyword to construct this object");
				return;
			}
		}

		NAN_METHOD(WarpUtil::Warp)
		{
			Nan::HandleScope scope;

			ExtendedWarpAppOptions* options = SetOptions(info[0]);
			GDALDatasetH results = Warp(options);

			//info.GetReturnValue().Set(Nan::New(result));
		}

		ExtendedWarpAppOptions* WarpUtil::SetOptions(Handle<Value> options)
		{
			Nan::HandleScope scope;

			ExtendedWarpAppOptions* warpOptions = new ExtendedWarpAppOptions();
			GDALDatasetH targetDataset = NULL;

			if (options->IsNull() || !options->IsObject())
			{
				Nan::ThrowTypeError("WarpUtil requires options object");
				return NULL;
			}

			Handle<Object> opts = options.As<Object>();
			Handle<Value> prop;

			//if (opts->HasOwnProperty(Nan::New("source").ToLocalChecked()))
			//{
			//	prop = opts->Get(Nan::New("source").ToLocalChecked());
			//	if (prop->IsArray())
			//	{
			//		Handle<Array> array = prop.As<Array>();
			//		if (array->Length() == 0)
			//		{
			//			Nan::ThrowTypeError("no input files given");
			//		}
			//		else
			//		{
			//			for (int i = 0; i < array->Length(); i++)
			//			{
			//				std::string val = *Nan::Utf8String(array->Get(i));
			//				warpOptions.inputFiles.push_back(strdup(val.c_str()));
			//			}
			//		}
			//	}
			//	else if (prop->IsString())
			//	{
			//		std::string val = *Nan::Utf8String(prop);
			//		warpOptions.inputFiles.push_back(strdup(val.c_str()));
			//		//TODO: should we check that file exists? this will add slowdown.
			//		/*FILE* f = VSIFOpen(_buildOptions.source[0], "r");
			//		if (f)
			//		{
			//		VSIFClose(f);
			//		}
			//		else
			//		{
			//		NanThrowTypeError("source file must exist");
			//		return 1;
			//		}*/
			//	}
			//	else
			//	{
			//		Nan::ThrowTypeError("no input file/s given");
			//	}
			//}
			//else
			//{
			//	Nan::ThrowTypeError("no source files given");
			//}

			//if (opts->HasOwnProperty(NanNew("target")))
			//{
			//	prop = opts->Get(NanNew("target"));
			//	if (prop->IsString())
			//	{
			//		std::string val = *NanUtf8String(prop);
			//		warpOptions.destinationFile = strdup(val.c_str());
			//	}
			//	else
			//	{
			//		NanThrowTypeError("target must be a string");
			//	}
			//}
			//else
			//{
			//	NanThrowTypeError("no target file name specified");
			//}

			//if (opts->HasOwnProperty(NanNew("width")))
			//{
			//	prop = opts->Get(NanNew("width"));
			//	warpOptions.width = prop->Int32Value();
			//}

			//if (opts->HasOwnProperty(NanNew("targetSrs")))
			//{
			//	prop = opts->Get(NanNew("targetSrs"));
			//	if (prop->IsString())
			//	{
			//		std::string val = *NanUtf8String(prop);
			//		warpOptions.targetSrs = strdup(val.c_str());
			//	}
			//}

			//if (opts->HasOwnProperty(NanNew("sourceSrs")))
			//{
			//	prop = opts->Get(NanNew("sourceSrs"));
			//	if (prop->IsString())
			//	{
			//		std::string val = *NanUtf8String(prop);
			//		warpOptions.sourceSrs = strdup(val.c_str());
			//	}
			//}

			//if (opts->HasOwnProperty(NanNew("height")))
			//{
			//	prop = opts->Get(NanNew("height"));
			//	warpOptions.height = prop->Int32Value();
			//}

			if (opts->HasOwnProperty(Nan::New("extents").ToLocalChecked()))
			{
				prop = opts->Get(Nan::New("extents").ToLocalChecked());
				if (prop->IsObject())
				{
					Handle<Object> extObj = prop.As<Object>();
					Handle<Value> minx = extObj->Get(Nan::New("minX").ToLocalChecked());
					Handle<Value> miny = extObj->Get(Nan::New("minY").ToLocalChecked());
					Handle<Value> maxx = extObj->Get(Nan::New("maxX").ToLocalChecked());
					Handle<Value> maxy = extObj->Get(Nan::New("maxY").ToLocalChecked());

					try
					{
						warpOptions->options->dfMinX = minx->NumberValue();
						warpOptions->options->dfMinY = miny->NumberValue();
						warpOptions->options->dfMaxX = maxx->NumberValue();
						warpOptions->options->dfMaxY = maxy->NumberValue();
					}
					catch (Exception e)
					{
						Nan::ThrowTypeError("bad extents object. should be of the form: {minX: 0, minY: 0, maxX: 0, maxY: 0}");
					}
				}
			}

			if (opts->HasOwnProperty(Nan::New("format").ToLocalChecked()))
			{
				prop = opts->Get(Nan::New("format").ToLocalChecked());
				if (prop->IsString())
				{
					std::string val = *Nan::Utf8String(prop);
					//warpOptions->format = strdup(val.c_str());
				}
			}

			if (opts->HasOwnProperty(Nan::New("tiled").ToLocalChecked()))
			{
				//if tiled is true we need to create the target dataset to pass in
			}

			if (opts->HasOwnProperty(Nan::New("silent").ToLocalChecked()))
			{
				prop = opts->Get(Nan::New("silent").ToLocalChecked());
				warpOptions->options->bQuiet = prop->BooleanValue();
			}

			return warpOptions;
		}

		GDALDatasetH WarpUtil::Warp(ExtendedWarpAppOptions* options)
		{
			return GDALWarp(options->targetFilename, options->targetDataset, options->sourceFilesCount, options->sourceFiles, options->options, NULL);
		}
	}
}
