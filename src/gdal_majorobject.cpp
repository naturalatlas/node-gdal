#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_driver.hpp"

#include <stdlib.h>

namespace node_gdal {

Local<Object> MajorObject::getMetadata(GDALMajorObject *obj, const char* domain)
{
	Nan::EscapableHandleScope scope;

	char **metadata = obj->GetMetadata(domain);

	Local<Object> result = Nan::New<Object>();

	if (metadata) {
		int i = 0;
		while (metadata[i]) {
			std::string pair = metadata[i];
			std::size_t i_equal = pair.find_first_of('=');
			if (i_equal != std::string::npos) {
				std::string key = pair.substr(0, i_equal);
				std::string val = pair.substr(i_equal+1);
				Nan::Set(result, Nan::New(key.c_str()).ToLocalChecked(), Nan::New(val.c_str()).ToLocalChecked());
			}
			i++;
		}
	}

	return scope.Escape(result);
}

} // namespace node_gdal
