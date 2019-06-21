

#include "number_list.hpp"

namespace node_gdal {

IntegerList::IntegerList()
	: list(NULL), len(0), name("")
{
}

IntegerList::IntegerList(const char* name)
	: list(NULL), len(0)
{
	name = (std::string(" ")+name).c_str();
}

IntegerList::~IntegerList()
{
	if(list) delete [] list;
}

int IntegerList::parse(Local<Value> value)
{
	Nan::HandleScope scope;
	unsigned int i;
	Local<Array> arr;

	if(value->IsNull() || value->IsUndefined()) 
		return 0;

	if(value->IsArray()) {
		arr = value.As<Array>();
		len = arr->Length();
		if (len == 0) 
			return 0;

		list = new int [len];
		for (i = 0; i < len; ++i) {
			Local<Value> element = Nan::Get(arr, i).ToLocalChecked();
			if(element->IsNumber()){
				list[i] = Nan::To<int32_t>(element).ToChecked();
			} else {
				std::string err = std::string("Every element in the")+name+" array must be a number";
				Nan::ThrowTypeError(err.c_str());
				return 1;
			}
		}
	} else if (value->IsNumber()) {
		list = new int [1];
		list[0] = Nan::To<int32_t>(value).ToChecked();
		len = 1;
	} else {
		std::string err = std::string(name)+"integer list must be an array or single integer";
		Nan::ThrowTypeError(err.c_str());
		return 1;
	}
	return 0;
}

DoubleList::DoubleList()
	: list(NULL), len(0), name("")
{
}

DoubleList::DoubleList(const char* name)
	: list(NULL), len(0)
{
	name = (std::string(" ")+name).c_str();
}

DoubleList::~DoubleList()
{
	if(list) delete [] list;
}

int DoubleList::parse(Local<Value> value)
{
	Nan::HandleScope scope;
	unsigned int i;
	Local<Array> arr;

	if(value->IsNull() || value->IsUndefined()) 
		return 0;

	if(value->IsArray()) {
		arr = value.As<Array>();
		len = arr->Length();
		if (len == 0) 
			return 0;

		list = new double [len];
		for (i = 0; i < len; ++i) {
			Local<Value> element = Nan::Get(arr, i).ToLocalChecked();
			if(element->IsNumber()){
				list[i] = Nan::To<double>(element).ToChecked();
			} else {
				std::string err = std::string("Every element in the")+name+" array must be a number";
				Nan::ThrowTypeError(err.c_str());
				return 1;
			}
		}
	} else if (value->IsNumber()) {
		list = new double [1];
		list[0] = Nan::To<double>(value).ToChecked();
		len = 1;
	} else {
		std::string err = std::string(name)+"double list must be an array or single number";
		Nan::ThrowTypeError(err.c_str());
		return 1;
	}
	return 0;
}


} //node_gdal namespace
