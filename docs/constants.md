# Constants 

All constants can be accessed directly from the module. ex: `gdal.constantName`. 

*Note: The value of constants in node-gdal do not necessarily match the values of constants in GDAL.*

### Pixel Data Types

Pixel data types. ([reference](http://www.gdal.org/gdal_8h.html#a22e22ce0a55036a96f652765793fb7a4))

```js
GDT_Unknown: undefined
GDT_Byte: 'Byte'
GDT_UInt16: 'UInt16'
GDT_Int16: 'Int16'
GDT_UInt32: 'UInt32'
GDT_Int32: 'Int32'
GDT_Float32: 'Float32'
GDT_Float64: 'Float64'
GDT_CInt16: 'CInt16'
GDT_CInt32: 'CInt32'
GDT_CFloat32: 'CFloat32'
GDT_CFloat64: 'CFloat64'
```

### Resampling

Resampling options that can be used with the `gdal.reprojectImage()` method. ([reference](http://www.gdal.org/gdalwarper_8h.html#a4775b029869df1f9270ad554c0633843))

```js
GRA_NearestNeighbor: 'NearestNeighbor'
GRA_Bilinear: 'Bilinear'
GRA_Cubic: 'Cubic'
GRA_CubicSpline: 'CubicSpline'
GRA_Lanczos: 'Lanczos'
GRA_Average: 'Average'
GRA_Mode: 'Mode'
```

### Geometry Types

Well known binary geometry types. ([reference](http://www.gdal.org/ogr__core_8h.html#a800236a0d460ef66e687b7b65610f12a))

```js
wkbUnknown: 0
wkbPoint: 1
wkbLineString: 2
wkbPolygon: 3
wkbMultiPoint: 4
wkbMultiLineString: 5
wkbMultiPolygon: 6
wkbGeometryCollection: 7
wkbNone: 100
wkbLinearRing: 101
wkbPoint25D: -2147483647
wkbLineString25D: -2147483646
wkbPolygon25D: -2147483645
wkbMultiPoint25D: -2147483644
wkbMultiLineString25D: -2147483643
wkbMultiPolygon25D: -2147483642
wkbGeometryCollection25D: -2147483641
wkbLinearRing25D: -2147483547
```

###### Conversion between 2D and 2.5D types  

The `wkb25DBit` constant can be used to convert between 2D types to 2.5D types.

```js
// 2 -> 2.5D
wkbPoint25D = gdal.wkbPoint | gdal.wkb25DBit

// 2.5D -> 2D (same as wkbFlatten())
wkbPoint = gdal.wkbPoint25D & (~gdal.wkb25DBit)
``` 

### WKB Variants

Output variants of WKB supported by GDAL. ([reference](http://www.gdal.org/ogr__core_8h.html#a6716bd3399c31e7bc8b0fd94fd7d9ba6))

```js
wkbVariantOgc: 'OGC'
wkbVariantIso: 'ISO'
```

### WKB Byte Order

Well known binary byte order.

```js
wkbXDR: 'MSB'
wkbNDR: 'LSB'
```

### Field Types

Feature Field Types. ([reference](http://www.gdal.org/ogr__core_8h.html#a787194bea637faf12d61643124a7c9fc))

```js
OFTInteger: 'integer'
OFTIntegerList: 'integer[]'
OFTReal: 'real'
OFTRealList: 'real[]'
OFTString: 'string'
OFTStringList: 'string[]'
OFTWideString: 'wideString',
OFTWideStringList: 'wideString[]'
OFTBinary: 'binary'
OFTDate: 'date'
OFTTime: 'time'
```

### Field Justification 

Display justification for field values.

```js
OJUndefined: undefined
OJLeft: 'Left'
OJRight: 'Right'
```

### Color Interpretation

Types of color interpretation for raster bands. ([reference](http://www.gdal.org/gdal_8h.html#ace76452d94514561fffa8ea1d2a5968c))

```js
GCI_Undefined: undefined
GCI_GrayIndex: 'Gray'
GCI_PaletteIndex: 'Palette'
GCI_RedBand: 'Red'
GCI_GreenBand: 'Green'
GCI_BlueBand: 'Blue'
GCI_AlphaBand: 'Alpha'
GCI_HueBand: 'Hue'
GCI_SaturationBand: 'Saturation'
GCI_LightnessBand: 'Lightness'
GCI_CyanBand: 'Cyan'
GCI_MagentaBand: 'Magenta'
GCI_YellowBand: 'Yellow'
GCI_BlackBand: 'Black'
GCI_YCbCr_YBand: 'YCbCr_Y'
GCI_YCbCr_CbBand: 'YCbCr_Cb'
GCI_YCbCr_CrBand: 'YCbCr_Cr'
```

### Driver Metadata

Metadata that can be retrieved from raster drivers. (and vector drivers once GDAL 2.0 is supported)

```js
DMD_LONGNAME: 'DMD_LONGNAME'
DMD_MIMETYPE: 'DMD_MIMETYPE'
DMD_HELPTOPIC: 'DMD_HELPTOPIC'
DMD_EXTENSION: 'DMD_EXTENSION'
DMD_CREATIONOPTIONLIST: 'DMD_CREATIONOPTIONLIST'
DMD_CREATIONDATATYPES: 'DMD_CREATIONDATATYPES'
DCAP_CREATE: 'DCAP_CREATE'
DCAP_CREATECOPY: 'DCAP_CREATECOPY'
DCAP_VIRTUALIO: 'DCAP_VIRTUALIO'
```

### Vector Layer Capabilities

Capabilities that can be tested with `Layer.testCapability()` method.

```js
OLCRandomRead: 'RandomRead'
OLCSequentialWrite: 'SequentialWrite'
OLCRandomWrite: 'RandomWrite'
OLCFastSpatialFilter: 'FastSpatialFilter'
OLCFastFeatureCount: 'FastFeatureCount'
OLCFastGetExtent: 'FastGetExtent'
OLCCreateField: 'CreateField'
OLCDeleteField: 'DeleteField'
OLCReorderFields: 'ReorderFields'
OLCAlterFieldDefn: 'AlterFieldDefn'
OLCTransactions: 'Transactions'
OLCDeleteFeature: 'DeleteFeature'
OLCFastSetNextByIndex: 'FastSetNextByIndex'
OLCStringsAsUTF8: 'StringsAsUTF8'
OLCIgnoreFields: 'IgnoreFields'
OLCCreateGeomField: 'CreateGeomField'
```

### Vector Dataset Capabilities

Capabilities that can be tested with `Dataset.testCapability()` method.

```js
ODsCCreateGeomFieldAfterCreateLayer: 'CreateGeomFieldAfterCreateLayer'
ODsCCreateLayer: 'CreateLayer'
ODsCDeleteLayer: 'DeleteLayer'
```

