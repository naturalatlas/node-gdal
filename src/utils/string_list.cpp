#include "string_list.hpp"

namespace node_gdal {

StringList::StringList()
	: list(NULL), strlist(NULL)
{
}

StringList::~StringList()
{
	if(list) delete [] list;
	if(strlist) delete [] strlist;
}

int StringList::parse(Local<Value> value)
{
	unsigned int i;

	if(value->IsNull() || value->IsUndefined()) 
		return 0;

	if(value->IsArray()) {
		Local<Array> array = value.As<Array>();
		if (array->Length() == 0) 
			return 0;

		list = new char* [array->Length() + 1];
		strlist = new std::string [array->Length()];
		for (i = 0; i < array->Length(); ++i) {
			strlist[i] = *Nan::Utf8String(Nan::Get(array, i).ToLocalChecked());
			list[i] = (char*) strlist[i].c_str();
		}
		list[i] = NULL;
	} else if (value->IsObject()) {
		Local<Object> obj = value.As<Object>();
		Local<Array> keys = Nan::GetOwnPropertyNames(obj).ToLocalChecked();
		if(keys->Length() == 0)
			return 0;

		list = new char* [keys->Length() + 1];
		strlist = new std::string [keys->Length()];
		for (i = 0; i < keys->Length(); ++i) {
			std::string key = *Nan::Utf8String(Nan::Get(keys, i).ToLocalChecked());
			std::string val = *Nan::Utf8String(Nan::Get(obj, Nan::Get(keys, i).ToLocalChecked()).ToLocalChecked());
			strlist[i] = key+"="+val;
			list[i] = (char*) strlist[i].c_str();
		}
		list[i] = NULL;
	} else {
		Nan::ThrowTypeError("String list must be an array or object");
		return 1;
	}
	return 0;
}

} //node_gdal namespace
