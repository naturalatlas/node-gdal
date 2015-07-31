#ifndef __NODE_GDAL_FIELDTYPES_H__
#define __NODE_GDAL_FIELDTYPES_H__

// OFTInteger = 0, OFTIntegerList = 1, OFTReal = 2, OFTRealList = 3,
// OFTString = 4, OFTStringList = 5, OFTWideString = 6, OFTWideStringList = 7,
// OFTBinary = 8, OFTDate = 9, OFTTime = 10, OFTDateTime = 11,
// OFTInteger64 = 12, OFTInteger64List = 13, OFTMaxType = 13

static const char * const FIELD_TYPES[] = {
	/* 0 */ "integer",
	/* 1 */ "integer[]",
	/* 2 */ "real",
	/* 3 */ "real[]",
	/* 4 */ "string",
	/* 5 */ "string[]",
	/* 6 */ "wideString",
	/* 7 */ "wideString[]",
	/* 8 */ "binary",
	/* 9 */ "date",
	/* 10 */ "time",
	/* 11 */ "dateTime",
	/* 12 */ "integer64",
	/* 13 */ "integer64[]"
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