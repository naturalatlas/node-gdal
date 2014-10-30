#ifndef __NODE_GDAL_FIELDTYPES_H__
#define __NODE_GDAL_FIELDTYPES_H__

static const char * const FIELD_TYPES[] = {
	"integer",
	"integer[]",
	"real",
	"real[]",
	"string",
	"string[]",
	"wideString",
	"wideString[]",
	"binary",
	"date",
	"time",
	"dateTime"
};

inline const char * getFieldTypeName(OGRFieldType type)
{
	int i = static_cast<int>(type);
	if (i < 0 || i > OFTMaxType) {
		return "invalid";
	}
	return FIELD_TYPES[i];
}

inline int getFieldTypeByName(std::string name)
{
	for (int i = 0; i <= OFTMaxType; i++) {
		if(name == FIELD_TYPES[i]) return i;
	}
	return -1; 
}

#endif