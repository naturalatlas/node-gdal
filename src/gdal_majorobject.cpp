#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_driver.hpp"

#include <stdlib.h>

namespace node_gdal {

Nan::Persistent<FunctionTemplate> MajorObject::constructor;

void MajorObject::Initialize(Local<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(MajorObject::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(Nan::New("MajorObject").ToLocalChecked());

	Nan::SetPrototypeMethod(lcons, "getMetadata", getMetadata);

	ATTR(lcons, "description", descriptionGetter, READ_ONLY_SETTER);

	target->Set(Nan::New("MajorObject").ToLocalChecked(), lcons->GetFunction());

	constructor.Reset(lcons);
}

MajorObject::MajorObject(GDALMajorObject *obj)
	: Nan::ObjectWrap(), this_(obj)
{
}

MajorObject::MajorObject()
	: Nan::ObjectWrap(), this_(0)
{
}

MajorObject::~MajorObject()
{
}


NAN_METHOD(MajorObject::New)
{
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		return;
	}

	if (info[0]->IsExternal()) {
		Local<External> ext = info[0].As<External>();
		void* ptr = ext->Value();
		MajorObject *f = static_cast<MajorObject *>(ptr);
		f->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
		return;
	} else {
		Nan::ThrowError("Cannot create MajorObject directly");
		return;
	}
}

Local<Value> MajorObject::New(GDALMajorObject *raw)
{
	Nan::EscapableHandleScope scope;
	if (!raw) {
		return scope.Escape(Nan::Null());
	}
	MajorObject *wrapped = new MajorObject(raw);
	Local<Value> ext = Nan::New<External>(wrapped);
	Local<Object> obj = Nan::New(MajorObject::constructor)->GetFunction()->NewInstance(1, &ext);
	return scope.Escape(obj);
}


NAN_METHOD(MajorObject::getMetadata)
{
	Nan::HandleScope scope;

	std::string domain("");
	NODE_ARG_OPT_STR(0, "domain", domain);

	MajorObject *obj = Nan::ObjectWrap::Unwrap<MajorObject>(info.This());
	if (!obj->this_) {
		Nan::ThrowError("MajorObject object has already been destroyed");
		return;
	}

	info.GetReturnValue().Set(getMetadata(obj->this_, domain.empty() ? NULL : domain.c_str()));
}

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
				result->Set(Nan::New(key.c_str()).ToLocalChecked(), Nan::New(val.c_str()).ToLocalChecked());
			}
			i++;
		}
	}

	return scope.Escape(result);
}


NAN_GETTER(MajorObject::descriptionGetter) {
	Nan::HandleScope scope;

	MajorObject *obj = Nan::ObjectWrap::Unwrap<MajorObject>(info.This());
	if (!obj->this_) {
		Nan::ThrowError("MajorObject object has already been destroyed");
		return;
	}

	info.GetReturnValue().Set(SafeString::New(obj->this_->GetDescription()));
}

} // namespace node_gdal