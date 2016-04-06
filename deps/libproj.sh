#!/bin/bash

set -eu

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR/libproj"

PROJ_VERSION=4.9.2
dir_proj=./proj

#
# download proj4 source
#

rm -rf $dir_proj
if [[ ! -f proj.tar.gz ]]; then
	curl -L http://download.osgeo.org/proj/proj-${PROJ_VERSION}.tar.gz -o proj.tar.gz
fi
tar -xzf proj.tar.gz
mv proj-${PROJ_VERSION} $dir_proj

rm -rf $dir_proj/config*
rm -rf $dir_proj/makefile*
rm -rf $dir_proj/missing
rm -rf $dir_proj/depcomp
rm -rf $dir_proj/ChangeLog
rm -rf $dir_proj/NEWS
rm -rf $dir_proj/INSTALL
rm -rf $dir_proj/README
rm -rf $dir_proj/ltconfig
rm -rf $dir_proj/install*
rm -rf $dir_proj/mkinstalldirs
rm -rf $dir_proj/*.sh
rm -rf $dir_proj/*.opt
rm -rf $dir_proj/*.in
rm -rf $dir_proj/*/*.in
rm -rf $dir_proj/*.am
rm -rf $dir_proj/*/*.am
rm -rf $dir_proj/*.m4
rm -rf $dir_proj/*/*.m4
rm -rf $dir_proj/*/*.def
rm -rf $dir_proj/man
rm -rf $dir_proj/jniwrap
rm -rf $dir_proj/nad/test*
rm -rf $dir_proj/nad/makefile*
rm -rf $dir_proj/nad/*_out*