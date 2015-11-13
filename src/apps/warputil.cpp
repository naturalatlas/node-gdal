#include "warputil.hpp"
#include <gdalwarp_lib.cpp>
#include "../gdal_dataset.hpp"

namespace node_gdal
{
	namespace apps
	{
		ExtendedWarpAppOptions::ExtendedWarpAppOptions()
		{
			this->options = GDALWarpAppOptionsNew(NULL, NULL);
			this->options->bTargetAlignedPixels = FALSE;
			this->options->bEnableDstAlpha = TRUE;
			this->options->bEnableSrcAlpha = TRUE;
			this->options->bCreateOutput = TRUE;
			this->options->bMulti = TRUE;
			this->options->bCopyMetadata = TRUE;
			this->options->bCopyBandInfo = TRUE;
			this->options->eOutputType = GDT_Unknown;
			this->options->eWorkingType = GDT_Unknown;

			//we will always create an in memory format and then save after the warp lib is done,
			//this gives us the ability to support more output types
			this->options->pszFormat = strdup("MEM");
			this->targetDataset = NULL;
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

			//now take this in memory dataset and save to disk as the desired format
			GDALDriverH driver = GDALGetDriverByName(options->format);
			if (driver != NULL)
			{
				GDALDatasetH output = GDALCreateCopy(driver, options->targetFilename, results, false, NULL, NULL, NULL);
				if (output != NULL)
				{
					GDALClose(output);
				}
			}

			//clean up
			for (int i = 0; i < options->sourceFilesCount; i++)
			{
				GDALClose(options->sourceDatasets[i]);				
			}
			GDALClose(results);
			delete options;
			
			info.GetReturnValue().Set(0);
		}

		ExtendedWarpAppOptions* WarpUtil::SetOptions(Handle<Value> options)
		{
			Nan::HandleScope scope;			

			if (options->IsNull() || !options->IsObject())
			{
				Nan::ThrowTypeError("WarpUtil requires options object");
				return NULL;
			}

			ExtendedWarpAppOptions* warpOptions = new ExtendedWarpAppOptions();			

			Handle<Object> opts = options.As<Object>();
			Handle<Value> prop;

			if (opts->HasOwnProperty(Nan::New("source").ToLocalChecked()))
			{
				prop = opts->Get(Nan::New("source").ToLocalChecked());
				if (prop->IsArray())
				{
					Handle<Array> array = prop.As<Array>();
					if (array->Length() == 0)
					{
						Nan::ThrowTypeError("no input files given");
					}
					else
					{
						try
						{
							int count = array->Length();
							GDALDatasetH* sourceDatasets = new GDALDatasetH[count];
							warpOptions->sourceDatasets = sourceDatasets;
							warpOptions->sourceFilesCount = count;

							for (int i = 0; i < count; i++)
							{
								std::string val = *Nan::Utf8String(array->Get(i));							
								GDALDatasetH ds = GDALOpen(strdup(val.c_str()), GA_ReadOnly);
								sourceDatasets[i] = ds;														
							}							
						}
						catch (Exception e)
						{
							Nan::ThrowError("Invalid source file");
						}
					}
				}
				else if (prop->IsString())
				{
					
					try
					{
						int count = 1;
						GDALDatasetH* sourceDatasets = new GDALDatasetH[count];
						warpOptions->sourceDatasets = sourceDatasets;
						warpOptions->sourceFilesCount = count;

						std::string val = *Nan::Utf8String(prop);
						GDALDatasetH ds = GDALOpen(strdup(val.c_str()), GA_ReadOnly);
						sourceDatasets[0] = ds;						
					}
					catch (Exception e)
					{
						Nan::ThrowError("Invalid source file");
					}
				}
				else
				{
					Nan::ThrowTypeError("no input file/s given");
				}
			}
			else
			{
				Nan::ThrowTypeError("no source files given");
			}

			if (opts->HasOwnProperty(Nan::New("target").ToLocalChecked()))
			{
				prop = opts->Get(Nan::New("target").ToLocalChecked());
				if (prop->IsString())
				{
					std::string val = *Nan::Utf8String(prop);
					warpOptions->targetFilename = strdup(val.c_str());
				}
				else
				{
					Nan::ThrowTypeError("target must be a string");
				}
			}
			else
			{
				Nan::ThrowTypeError("no target file name specified");
			}

			if (opts->HasOwnProperty(Nan::New("width").ToLocalChecked()))
			{
				prop = opts->Get(Nan::New("width").ToLocalChecked());
				warpOptions->options->nForcePixels = prop->Int32Value();				
			}

			if (opts->HasOwnProperty(Nan::New("targetSrs").ToLocalChecked()))
			{
				prop = opts->Get(Nan::New("targetSrs").ToLocalChecked());
				if (prop->IsString())
				{
					std::string val = *Nan::Utf8String(prop);
					warpOptions->options->pszTE_SRS = strdup(val.c_str());
				}
			}			

			if (opts->HasOwnProperty(Nan::New("height").ToLocalChecked()))
			{
				prop = opts->Get(Nan::New("height").ToLocalChecked());
				warpOptions->options->nForceLines = prop->Int32Value();
			}

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
					warpOptions->format = strdup(val.c_str());
				}
			}

			if (opts->HasOwnProperty(Nan::New("tiled").ToLocalChecked()))
			{
				prop = opts->Get(Nan::New("tiled").ToLocalChecked());
				if (prop->BooleanValue() == true)
				{
					warpOptions->options->papszCreateOptions[0] = "TILED=YES";
				}
			}

			if (opts->HasOwnProperty(Nan::New("persistMetadata").ToLocalChecked()))
			{
				prop = opts->Get(Nan::New("persistMetadata").ToLocalChecked());
				CPLSetConfigOption("GDAL_PAM_ENABLED", prop->BooleanValue() ? "YES" : "NO");				
			}
			else
			{
				CPLSetConfigOption("GDAL_PAM_ENABLED", "NO");
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
			return GDALWarp(options->targetFilename, options->targetDataset, options->sourceFilesCount, options->sourceDatasets, options->options, NULL);
		}
	}
}
