#include "../gdal_common.hpp"
#include "../gdal_field_defn.hpp"
#include "../gdal_layer.hpp"
#include "layer_fields.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> LayerFields::constructor;

void LayerFields::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(LayerFields::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("LayerFields"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);
	NODE_SET_PROTOTYPE_METHOD(lcons, "remove", remove);
	NODE_SET_PROTOTYPE_METHOD(lcons, "getNames", getNames);
	NODE_SET_PROTOTYPE_METHOD(lcons, "indexOf", indexOf);
	NODE_SET_PROTOTYPE_METHOD(lcons, "reorder", reorder);
	NODE_SET_PROTOTYPE_METHOD(lcons, "add", add);
	//NODE_SET_PROTOTYPE_METHOD(lcons, "alter", alter);

	ATTR_DONT_ENUM(lcons, "layer", layerGetter, READ_ONLY_SETTER);

	target->Set(NanNew("LayerFields"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

LayerFields::LayerFields()
	: ObjectWrap()
{}

LayerFields::~LayerFields()
{}

/**
 * @class gdal.LayerFields
 */
NAN_METHOD(LayerFields::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		LayerFields *layer =  static_cast<LayerFields *>(ptr);
		layer->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create LayerFields directly");
		NanReturnUndefined();
	}
}

Handle<Value> LayerFields::New(Handle<Value> layer_obj)
{
	NanEscapableScope();

	LayerFields *wrapped = new LayerFields();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(LayerFields::constructor)->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(NanNew("parent_"), layer_obj);

	return NanEscapeScope(obj);
}

NAN_METHOD(LayerFields::toString)
{
	NanScope();
	NanReturnValue(NanNew("LayerFields"));
}

/**
 * Returns the number of fields.
 *
 * @method count
 * @return {Integer}
 */
NAN_METHOD(LayerFields::count)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	OGRFeatureDefn *def = layer->get()->GetLayerDefn();
	if (!def) {
		NanThrowError("Layer has no layer definition set");
		NanReturnUndefined();
	}

	NanReturnValue(NanNew<Integer>(def->GetFieldCount()));
}

/**
 * Find the index of field in the layer.
 *
 * @method indexOf
 * @param {String} field
 * @return {Integer} Field index, or -1 if the field doesn't exist
 */
NAN_METHOD(LayerFields::indexOf)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	OGRFeatureDefn *def = layer->get()->GetLayerDefn();
	if (!def) {
		NanThrowError("Layer has no layer definition set");
		NanReturnUndefined();
	}

	std::string name("");
	NODE_ARG_STR(0, "field name", name);

	NanReturnValue(NanNew<Integer>(def->GetFieldIndex(name.c_str())));
}

/**
 * Returns a field definition.
 *
 * @throws Error
 * @method get
 * @param {String|integer} field Field name or index (0-based)
 * @return {gdal.FieldDefn}
 */
NAN_METHOD(LayerFields::get)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	if (args.Length() < 1) {
		NanThrowError("Field index or name must be given");
		NanReturnUndefined();
	}

	OGRFeatureDefn *def = layer->get()->GetLayerDefn();
	if (!def) {
		NanThrowError("Layer has no layer definition set");
		NanReturnUndefined();
	}

	int field_index;
	ARG_FIELD_ID(0, def, field_index);

	NanReturnValue(FieldDefn::New(def->GetFieldDefn(field_index)));
}

/**
 * Returns a list of field names.
 *
 * @throws Error
 * @method getNames
 * @return {Array} List of strings.
 */
NAN_METHOD(LayerFields::getNames)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	OGRFeatureDefn *def = layer->get()->GetLayerDefn();
	if (!def) {
		NanThrowError("Layer has no layer definition set");
		NanReturnUndefined();
	}

	int n = def->GetFieldCount();
	Handle<Array> result = NanNew<Array>(n);

	for (int i = 0; i < n;  i++) {
		OGRFieldDefn *field_def = def->GetFieldDefn(i);
		result->Set(i, SafeString::New(field_def->GetNameRef()));
	}

	NanReturnValue(result);
}

/**
 * Removes a field.
 *
 * @throws Error
 * @method remove
 * @param {String|integer} field Field name or index (0-based)
 */
NAN_METHOD(LayerFields::remove)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	if (args.Length() < 1) {
		NanThrowError("Field index or name must be given");
		NanReturnUndefined();
	}

	OGRFeatureDefn *def = layer->get()->GetLayerDefn();
	if (!def) {
		NanThrowError("Layer has no layer definition set");
		NanReturnUndefined();
	}

	int field_index;
	ARG_FIELD_ID(0, def, field_index);

	int err = layer->get()->DeleteField(field_index);
	if(err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

/**
 * Adds field(s).
 *
 * @throws Error
 * @method add
 * @param {gdal.FieldDefn|Array} def(s) A field definition, or array of field definitions.
 * @param {Boolean} [approx=true]
 */
NAN_METHOD(LayerFields::add)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}
	if (args.Length() < 1) {
		NanThrowError("field definition(s) must be given");
		NanReturnUndefined();
	}

	FieldDefn *field_def;
	int err;
	int approx = 1;
	NODE_ARG_BOOL_OPT(1, "approx", approx);

	if (args[0]->IsArray()) {
		Handle<Array> array = args[0].As<Array>();
		int n = array->Length();
		for (int i = 0; i < n; i++) {
			Handle<Value> element = array->Get(i);
			if (IS_WRAPPED(element, FieldDefn)) {
				field_def = ObjectWrap::Unwrap<FieldDefn>(element.As<Object>());
				err = layer->get()->CreateField(field_def->get(), approx);
				if(err) {
					NODE_THROW_OGRERR(err);
					NanReturnUndefined();
				}
			} else {
				NanThrowError("All array elements must be FieldDefn objects");
				NanReturnUndefined();
			}
		}
	} else if (IS_WRAPPED(args[0], FieldDefn)) {
		field_def = ObjectWrap::Unwrap<FieldDefn>(args[0].As<Object>());
		err = layer->get()->CreateField(field_def->get(), approx);
		if(err) {
			NODE_THROW_OGRERR(err);
			NanReturnUndefined();
		}
	} else {
		NanThrowError("field definition(s) must be a FieldDefn object or array of FieldDefn objects");
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

/**
 * Reorders fields.
 *
 * @example
 * ```
 * // reverse field order
 * layer.fields.reorder([2,1,0]);```
 *
 * @throws Error
 * @method reorder
 * @param {Array} map An array of new indexes (integers)
 */
NAN_METHOD(LayerFields::reorder)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	Layer *layer = ObjectWrap::Unwrap<Layer>(parent);
	if (!layer->get()) {
		NanThrowError("Layer object already destroyed");
		NanReturnUndefined();
	}

	OGRFeatureDefn *def = layer->get()->GetLayerDefn();
	if (!def) {
		NanThrowError("Layer has no layer definition set");
		NanReturnUndefined();
	}

	Handle<Array> field_map = NanNew<Array>(0);
	NODE_ARG_ARRAY(0, "field map", field_map);

	int n = def->GetFieldCount();
	OGRErr err = 0;

	if ((int)field_map->Length() != n) {
		NanThrowError("Array length must match field count");
		NanReturnUndefined();
	}

	int *field_map_array = new int[n];

	for (int i = 0; i < n; i++) {
		Handle<Value> val = field_map->Get(i);
		if (!val->IsNumber()) {
			delete [] field_map_array;
			NanThrowError("Array must only contain integers");
			NanReturnUndefined();
		}

		int key = val->IntegerValue();
		if (key < 0 || key >= n) {
			delete [] field_map_array;
			NanThrowError("Values must be between 0 and field count - 1");
			NanReturnUndefined();
		}

		field_map_array[i] = key;
	}

	err = layer->get()->ReorderFields(field_map_array);

	delete [] field_map_array;

	if(err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}
	NanReturnUndefined();
}

/**
 * Parent layer
 *
 * @readOnly
 * @attribute layer
 * @type {gdal.Layer}
 */
NAN_GETTER(LayerFields::layerGetter)
{
	NanScope();
	NanReturnValue(args.This()->GetHiddenValue(NanNew("parent_")));
}

} // namespace node_gdal