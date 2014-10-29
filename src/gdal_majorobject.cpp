#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_driver.hpp"

#include <stdlib.h>

namespace node_gdal {

Persistent<FunctionTemplate> MajorObject::constructor;

void MajorObject::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(MajorObject::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("MajorObject"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "getMetadata", getMetadata);

	ATTR(lcons, "description", descriptionGetter, READ_ONLY_SETTER);

	target->Set(NanNew("MajorObject"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

MajorObject::MajorObject(GDALMajorObject *obj)
	: ObjectWrap(), this_(obj)
{
}

MajorObject::MajorObject()
	: ObjectWrap(), this_(0)
{
}

MajorObject::~MajorObject()
{
}


NAN_METHOD(MajorObject::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		MajorObject *f = static_cast<MajorObject *>(ptr);
		f->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create MajorObject directly");
		NanReturnUndefined();
	}
}

Handle<Value> MajorObject::New(GDALMajorObject *raw)
{
	NanEscapableScope();
	if (!raw) {
		return NanEscapeScope(NanNull());
	}
	MajorObject *wrapped = new MajorObject(raw);
	Handle<Value> ext = NanNew<External>(wrapped);
	Handle<Object> obj = NanNew(MajorObject::constructor)->GetFunction()->NewInstance(1, &ext);
	return NanEscapeScope(obj);
}


NAN_METHOD(MajorObject::getMetadata)
{
	NanScope();

	std::string domain("");
	NODE_ARG_OPT_STR(0, "domain", domain);

	MajorObject *obj = ObjectWrap::Unwrap<MajorObject>(args.This());
	if (!obj->this_) {
		NanThrowError("MajorObject object has already been destroyed");
		NanReturnUndefined();
	}

	NanReturnValue(getMetadata(obj->this_, domain.empty() ? NULL : domain.c_str()));
}

Handle<Object> MajorObject::getMetadata(GDALMajorObject *obj, const char* domain)
{
	NanEscapableScope();

	char **metadata = obj->GetMetadata(domain);

	Local<Object> result = NanNew<Object>();

	if (metadata) {
		int i = 0;
		while (metadata[i]) {
			std::string pair = metadata[i];
			std::size_t i_equal = pair.find_first_of('=');
			if (i_equal != std::string::npos) {
				std::string key = pair.substr(0, i_equal);
				std::string val = pair.substr(i_equal+1);
				result->Set(NanNew(key.c_str()), NanNew(val.c_str()));
			}
			i++;
		}
	}

	return NanEscapeScope(result);
}


NAN_GETTER(MajorObject::descriptionGetter) {
	NanScope();

	MajorObject *obj = ObjectWrap::Unwrap<MajorObject>(args.This());
	if (!obj->this_) {
		NanThrowError("MajorObject object has already been destroyed");
		NanReturnUndefined();
	}

	NanReturnValue(SafeString::New(obj->this_->GetDescription()));
}

} // namespace node_gdal