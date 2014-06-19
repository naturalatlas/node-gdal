#include "gdal_common.hpp"
#include "gdal_majorobject.hpp"
#include "gdal_driver.hpp"

Persistent<FunctionTemplate> MajorObject::constructor;

void MajorObject::Initialize(Handle<Object> target)
{
	HandleScope scope;

	constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(MajorObject::New));
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(String::NewSymbol("MajorObject"));

	NODE_SET_PROTOTYPE_METHOD(constructor, "getMetadata", getMetadata);

	ATTR(constructor, "description", descriptionGetter, READ_ONLY_SETTER);

	target->Set(String::NewSymbol("MajorObject"), constructor->GetFunction());
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


Handle<Value> MajorObject::New(const Arguments& args)
{
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return NODE_THROW("Cannot call constructor as function, you need to use 'new' keyword");
	}

	if (args[0]->IsExternal()) {
		Local<External> ext = Local<External>::Cast(args[0]);
		void* ptr = ext->Value();
		MajorObject *f = static_cast<MajorObject *>(ptr);
		f->Wrap(args.This());
		return args.This();
	} else {
		return NODE_THROW("Cannot create MajorObject directly");
	}
}

Handle<Value> MajorObject::New(GDALMajorObject *obj)
{
	return ClosedPtr<MajorObject, GDALMajorObject>::Closed(obj);
}


Handle<Value> MajorObject::getMetadata(const Arguments& args)
{
	HandleScope scope;

	std::string domain("");
	NODE_ARG_OPT_STR(0, "domain", domain);

	MajorObject *obj = ObjectWrap::Unwrap<MajorObject>(args.This());
	if (!obj->this_) {
		return NODE_THROW("MajorObject object has already been destroyed");
	}

	return scope.Close(getMetadata(obj->this_, domain.empty() ? NULL : domain.c_str()));
}

Handle<Value> MajorObject::getMetadata(GDALMajorObject *obj, const char* domain)
{
	HandleScope scope;

	char **metadata = obj->GetMetadata(domain);

	Local<Object> result = Object::New();

	if (metadata) {
		int i = 0;
		while (metadata[i]) {
			char* val = strchr(metadata[i], '=');
			if (val) {
				*val = 0; //split string into key and value
				result->Set(String::NewSymbol(metadata[i]), String::New(val+1));
			}
			i++;
		}
	}

	return scope.Close(result);
}


Handle<Value> MajorObject::descriptionGetter(Local<String> property, const AccessorInfo &info) {
	HandleScope scope;

	MajorObject *obj = ObjectWrap::Unwrap<MajorObject>(info.This());
	if (!obj->this_) {
		return NODE_THROW("MajorObject object has already been destroyed");
	}

	return scope.Close(SafeString::New(obj->this_->GetDescription()));
}