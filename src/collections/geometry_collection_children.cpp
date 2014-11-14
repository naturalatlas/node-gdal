#include "../gdal_common.hpp"
#include "../gdal_geometry.hpp"
#include "../gdal_geometrycollection.hpp"
#include "geometry_collection_children.hpp"

namespace node_gdal {

Persistent<FunctionTemplate> GeometryCollectionChildren::constructor;

void GeometryCollectionChildren::Initialize(Handle<Object> target)
{
	NanScope();

	Local<FunctionTemplate> lcons = NanNew<FunctionTemplate>(GeometryCollectionChildren::New);
	lcons->InstanceTemplate()->SetInternalFieldCount(1);
	lcons->SetClassName(NanNew("GeometryCollectionChildren"));

	NODE_SET_PROTOTYPE_METHOD(lcons, "toString", toString);
	NODE_SET_PROTOTYPE_METHOD(lcons, "count", count);
	NODE_SET_PROTOTYPE_METHOD(lcons, "get", get);
	NODE_SET_PROTOTYPE_METHOD(lcons, "remove", remove);
	NODE_SET_PROTOTYPE_METHOD(lcons, "add", add);

	target->Set(NanNew("GeometryCollectionChildren"), lcons->GetFunction());

	NanAssignPersistent(constructor, lcons);
}

GeometryCollectionChildren::GeometryCollectionChildren()
	: ObjectWrap()
{}

GeometryCollectionChildren::~GeometryCollectionChildren()
{}

/**
 * A collection of Geometries, used by {{#crossLink "gdal.GeometryCollection"}}gdal.GeometryCollection{{/crossLink}}.
 *
 * @class gdal.GeometryCollectionChildren
 */
NAN_METHOD(GeometryCollectionChildren::New)
{
	NanScope();

	if (!args.IsConstructCall()) {
		NanThrowError("Cannot call constructor as function, you need to use 'new' keyword");
		NanReturnUndefined();
	}
	if (args[0]->IsExternal()) {
		Local<External> ext = args[0].As<External>();
		void* ptr = ext->Value();
		GeometryCollectionChildren *geom =  static_cast<GeometryCollectionChildren *>(ptr);
		geom->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		NanThrowError("Cannot create GeometryCollectionChildren directly");
		NanReturnUndefined();
	}
}

Handle<Value> GeometryCollectionChildren::New(Handle<Value> geom)
{
	NanEscapableScope();

	GeometryCollectionChildren *wrapped = new GeometryCollectionChildren();

	v8::Handle<v8::Value> ext = NanNew<External>(wrapped);
	v8::Handle<v8::Object> obj = NanNew(GeometryCollectionChildren::constructor)->GetFunction()->NewInstance(1, &ext);
	obj->SetHiddenValue(NanNew("parent_"), geom);

	return NanEscapeScope(obj);
}

NAN_METHOD(GeometryCollectionChildren::toString)
{
	NanScope();
	NanReturnValue(NanNew("GeometryCollectionChildren"));
}

/**
 * Returns the number of items.
 *
 * @method count
 * @return Integer
 */
NAN_METHOD(GeometryCollectionChildren::count)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	GeometryCollection *geom = ObjectWrap::Unwrap<GeometryCollection>(parent);

	NanReturnValue(NanNew<Integer>(geom->get()->getNumGeometries()));
}

/**
 * Returns the geometry at the specified index.
 *
 * @method get
 * @param {Integer} index 0-based index
 * @return {gdal.Geometry}
 */
NAN_METHOD(GeometryCollectionChildren::get)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	GeometryCollection *geom = ObjectWrap::Unwrap<GeometryCollection>(parent);

	int i;
	NODE_ARG_INT(0, "index", i);

	NanReturnValue(Geometry::New(geom->get()->getGeometryRef(i), false));
}

/**
 * Removes the geometry at the specified index.
 *
 * @method remove
 * @param {Integer} index 0-based index
 */
NAN_METHOD(GeometryCollectionChildren::remove)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	GeometryCollection *geom = ObjectWrap::Unwrap<GeometryCollection>(parent);

	int i;
	NODE_ARG_INT(0, "index", i);

	OGRErr err = geom->get()->removeGeometry(i);
	if(err) {
		NODE_THROW_OGRERR(err);
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

/**
 * Adds geometry(s) to the collection.
 *
 * @example
 * ```
 * // one at a time:
 * geometryCollection.children.add(new Point(0,0,0));
 *
 * // add many at once:
 * geometryCollection.children.add([
 *     new Point(1,0,0),
 *     new Point(1,0,0)
 * ]);```
 *
 * @method add
 * @param {mixed} geometry
 */
NAN_METHOD(GeometryCollectionChildren::add)
{
	NanScope();

	Handle<Object> parent = args.This()->GetHiddenValue(NanNew("parent_")).As<Object>();
	GeometryCollection *geom = ObjectWrap::Unwrap<GeometryCollection>(parent);

	Geometry *child;

	if (args.Length() < 1) {
		NanThrowError("child(ren) must be given");
		NanReturnUndefined();
	}
	if (args[0]->IsArray()){
		//set from array of geometry objects
		Handle<Array> array = args[0].As<Array>();
		int length = array->Length();
		for (int i = 0; i < length; i++){
			Handle<Value> element = array->Get(i);
			if(IS_WRAPPED(element, Geometry)){
				child = ObjectWrap::Unwrap<Geometry>(element.As<Object>());
				OGRErr err = geom->get()->addGeometry(child->get());
				if(err) {
					NODE_THROW_OGRERR(err);
					NanReturnUndefined();
				}
			} else {
				NanThrowError("All array elements must be geometry objects");
				NanReturnUndefined();
			}
		}
	} else if (IS_WRAPPED(args[0], Geometry)){
		child = ObjectWrap::Unwrap<Geometry>(args[0].As<Object>());
		OGRErr err = geom->get()->addGeometry(child->get());
		if(err) {
			NODE_THROW_OGRERR(err);
			NanReturnUndefined();
		}
	} else {
		NanThrowError("child must be a geometry object or array of geometry objects");
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

} // namespace node_gdal