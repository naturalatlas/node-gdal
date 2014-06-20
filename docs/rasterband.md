## RasterBand

Note: all methods throw errors if the band has been destroyed by the dataset

#### Methods

- `toString()` : string
- `flush()` : void
- `fill(number real_value, number imag_value = 0)` : void *(throws)*
- `getStatistics(bool allow_approx, bool force)` : object *(throws)*
- `computeStatistics(bool allow_approx)` : object *(throws)*
- `setStatistics(Number min, Number max, Number mean, Number std_dev)` : void *(throws)*
- `createMaskBand(int num_samples)` : void *(throws)*
- `getMaskBand()` : [RasterBand](rasterband.md)
- `getMaskFlags()` : int
- `getMetadata(string domain = null)` : object

#### Properties

- `description` : string
- `ds` : [Dataset](dataset.md)
- `id` : int
- `size` : object
- `blockSize` : object
- `unitType` : string *(settable)*
- `dataType` : int //GDALDataType
- `minimum` : Number
- `maximum` : Number
- `offset` : Number *(settable)*
- `scale` : Number *(settable)*
- `noDataValue` : Number *(settable)*
- `readOnly` : bool
- `hasArbitraryOverviews` : bool
- `categoryNames` : string[] *(settable)*
- `overviews` : RasterBandOverviews
- `colorInterpretation` : string
    + `"Undefined"`
    + `"Gray"`
    + `"Palette"`
    + `"Red"`
    + `"Green"`
    + `"Blue"`
    + `"Alpha"`
    + `"Hue"`
    + `"Saturation"`
    + `"Lightness"`
    + `"Cyan"`
    + `"Magenta"`
    + `"Yellow"`
    + `"Black"`

#### Overviews

- `overviews.get(int id)` : [RasterBand](rasterband.md)
- `overviews.getBySampleCount(int min_samples)` : [RasterBand](rasterband.md)
- `overviews.count()` : int