## Contributing

### Making a Release

```
# updates package.json, tags it, puts out new docs, publishes, pushes to git, etc
$ make release version=0.9.1

# once builds complete, run:
$ npm publish
```

Afterwards just click the "Draft a New Release" button [here](https://github.com/naturalatlas/node-gdal/releases), select the tag, and type in the version and some notes and that should do it.
