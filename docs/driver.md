## Driver

#### Methods

- `toString()` : string
- `open(string filename, string mode)` : object
    + *(only use to open existing files)*
- `create(string filename, int x_size, int y_size, int n_bands, int gdal_data_type = GDT_Byte, string[] co)` : [Dataset](dataset.md) *(throws)*
- `create(string filename, string[] co)` : [Dataset](dataset.md) *(throws)*
- `createCopy(string filename, Dataset src, bool strict = false, string[] options = null)` : [Dataset](dataset.md) *(throws)*
- `deleteDataset(string filename)` : void *(throws)*
- `rename(string new_name, string old_name)` : void *(throws)*
- `copyFiles(string new_name, string old_name)` : void *(throws)*
- `getMetadata(string domain = null)` : object

#### Properties

- `description` : string