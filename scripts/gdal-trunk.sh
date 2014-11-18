#!/bin/bash

# configure environment
if [ "$(uname)" == "Darwin" ]; then
	brew uninstall gdal
else
	sudo apt-get remove libgdal-dev
	sudo apt-get remove libgdal1-dev
	sudo apt-get remove libgdal1h
	sudo apt-get remove gdal-bin
	sudo apt-get install unzip
fi

# set up directory
mkdir gdal2
cd gdal2

# download trunk (http://trac.osgeo.org/gdal/browser/trunk)
svn checkout https://svn.osgeo.org/gdal/trunk/gdal gdal --non-interactive --trust-server-cert
cd gdal

# compile and install
./configure --with-png=internal --with-jpeg=internal --with-geotiff=internal --with-libtiff=internal
make
sudo make install