#!/bin/bash

set -eu

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR/libgdal"

GDAL_VERSION=2.4.4
dir_gdal=./gdal
dir_formats_gyp=./gyp-formats
dir_gyp_templates=./gyp-templates

#
# download gdal source
#

rm -rf $dir_gdal
if [[ ! -f gdal-${GDAL_VERSION}.tar.gz ]]; then
	curl -L http://download.osgeo.org/gdal/${GDAL_VERSION}/gdal-${GDAL_VERSION}.tar.gz -o gdal-${GDAL_VERSION}.tar.gz
fi
tar -xzf gdal-${GDAL_VERSION}.tar.gz
mv gdal-${GDAL_VERSION} $dir_gdal

rm -rf $dir_gdal/wince
rm -rf $dir_gdal/swig
rm -rf $dir_gdal/vb6
rm -rf $dir_gdal/html
# commented since headers are now in apps/ in gdal >= 2.1.0
# https://trac.osgeo.org/gdal/ticket/6496#ticket
#rm -rf $dir_gdal/apps
rm -rf $dir_gdal/man
rm -rf $dir_gdal/m4
rm -rf $dir_gdal/doc
rm -rf $dir_gdal/scripts
rm -r $dir_gdal/install-sh
rm -rf $dir_gdal/*.in
rm -rf $dir_gdal/*.sh
rm -rf $dir_gdal/*.bat
rm -rf $dir_gdal/*.opt
rm -rf $dir_gdal/*.py
rm -rf $dir_gdal/*.dox
rm -rf $dir_gdal/*/*Makefile*
rm -rf $dir_gdal/*/GNUmakefile
rm -rf $dir_gdal/*/Doxyfile
rm -rf $dir_gdal/*/README*
rm -rf $dir_gdal/*/Makefile.dist
rm -rf $dir_gdal/*/makefile.vc
rm -rf $dir_gdal/*/*.html
rm -rf $dir_gdal/*/*.wince
rm -rf $dir_gdal/*/*.m4
rm -rf $dir_gdal/*/*.dist
rm -rf $dir_gdal/*/*/README*
rm -rf $dir_gdal/*/*/*Makefile*
rm -rf $dir_gdal/*/*/GNUmakefile
rm -rf $dir_gdal/*/*/Doxyfile
rm -rf $dir_gdal/*/*/README*
rm -rf $dir_gdal/*/*/Makefile.dist
rm -rf $dir_gdal/*/*/makefile.vc
rm -rf $dir_gdal/*/*/*.html
rm -rf $dir_gdal/*/*/*.wince
rm -rf $dir_gdal/*/*/*.m4
rm -rf $dir_gdal/*/*/*.dist
rm -rf $dir_gdal/*/*/*/*Makefile*
rm -rf $dir_gdal/*/*/*/GNUmakefile
rm -rf $dir_gdal/*/*/*/Doxyfile
rm -rf $dir_gdal/*/*/*/README*
rm -rf $dir_gdal/*/*/*/Makefile.dist
rm -rf $dir_gdal/*/*/*/makefile.vc
rm -rf $dir_gdal/*/*/*/*.html
rm -rf $dir_gdal/*/*/*/*.wince
rm -rf $dir_gdal/*/*/*/*.m4
rm -rf $dir_gdal/*/*/*/*.dist
rm -f $dir_gdal/config*
rm -f $dir_gdal/Doxyfile
rm -f $dir_gdal/GNUmakefile
rm -f $dir_gdal/makefile.vc
rm -f $dir_gdal/aclocal.m4
rm -f $dir_gdal/NEWS
rm -f $dir_gdal/COMMITERS*
rm -f $dir_gdal/COMMITTERS*
rm -f $dir_gdal/Doxygen*
rm -f $dir_gdal/PERFORMANCE*
rm -f $dir_gdal/PROVENANCE*
rm -f $dir_gdal/MIGRATION_GUIDE*
rm -f $dir_gdal/HOWTO-RELEASE*
rm -f $dir_gdal/Vagrantfile
rm -rf $dir_gdal/ci
rm -rf $dir_gdal/fuzzers

#
# apply patches
#

#patch gdal/gcore/gdal_priv.h < patches/gcore_gdal_priv.diff # clang support
#patch gdal/frmts/wms/gdalwmsdataset.cpp < patches/frmts_wms_gdalwmsdataset.diff # fixes error in wms driver
patch gdal/ogr/ogrsf_frmts/shape/shptree.c < patches/ogrsf_frmts_shape_shptree.diff # fixes INT_MAX undeclared error
#patch gdal/gcore/gdalexif.cpp < patches/gcore_gdalexif.diff # fixes MSVC++ internal compiler error (https://github.com/naturalatlas/node-gdal/issues/45)
patch gdal/ogr/ogrsf_frmts/shape/shpopen.c < patches/ogrsf_frmts_shape_shpopenc.diff # missing cpl_port.h
patch gdal/ogr/ogrsf_frmts/shape/dbfopen.c < patches/ogrsf_frmts_shape_dbfopen.diff
patch gdal/ogr/ogrsf_frmts/shape/sbnsearch.c < patches/ogrsf_frmts_shape_sbnsearch.diff
patch gdal/frmts/blx/blx.c < patches/frmts_blx_blxc.diff # missing cpl_port.h


#
# create format gyps
#

GDAL_FORMATS="gtiff hfa aigrid aaigrid ceos ceos2 iso8211 xpm
	sdts raw dted mem jdem envisat elas fit vrt usgsdem l1b
	nitf bmp pcidsk airsar rs2 ilwis rmf leveller sgi srtmhgt
	idrisi gsg ingr ers jaxapalsar dimap gff cosar pds adrg
	coasp tsx terragen blx til r northwood saga xyz hf2
	kmlsuperoverlay ctg e00grid zmap ngsgeoid iris map zlib
	jpeg png grib sigdem derived null
	${OPT_GDAL_FORMATS:-}"

OGR_FORMATS="shape vrt avc geojson mem mitab kml gpx aeronavfaa
	bna dxf csv edigeo geoconcept georss gml gmt gpsbabel gtm htf
	idrisi dgn openair openfilegdb pds pgdump rec s57 sdts segukooa
	segy sua svg sxf ntf xplane wasp mvt"

mkdir -p $dir_formats_gyp

function wrap_array() {
	local serialized=
	local indent="$2"
	local var=""
	local tabs=""
	for (( c=1; c<=$2; c++)) ; do tabs="\t$tabs"; done
	while read a; do
		if [ -n "$a" ]; then
			serialized=$(wrap_string "$a")
			var="$var$serialized,\n$tabs"
		fi
	done <<< "$1"
	var=$(trim_comma "$var")
	printf "%s" "$var"
}
function wrap_string() {
	local var=$(trim $1)
	printf "%s" "\"$var\""
}
function trim() {
	local var=$@
	var="${var#"${var%%[![:space:]]*}"}"   # remove leading whitespace characters
	var="${var%"${var##*[![:space:]]}"}"   # remove trailing whitespace characters
	printf "%s" "$var"
}
function trim_comma() {
	local var=$@
	var=`echo -e "$var" | perl -p -000 -e 's/[\s\n\,]+$//g'`
	printf "%s" "$var"
}

format_list_gyps=""
format_list_defs=""
format_gyp_template=`cat ${dir_gyp_templates}/libgdal_format.gyp`

function generate_formats() {
	local formats="$1"
	local directory="$2"
	local prefix="${3:-}"

	for fmt in $formats; do
		file_gyp="${dir_formats_gyp}/${prefix}${fmt}.gyp"
		target_name="libgdal_${prefix}${fmt}_frmt"
		format_target_name=$(wrap_string "$target_name")

		dir_format="${directory}/${fmt}"
		files_src=`find ${dir_format} | egrep '\.(c|cpp)$' | awk '{print "." $0}'`
		gyp_template="${format_gyp_template}"

		format_sources=$(wrap_array "$files_src" 4)
		format_include_dirs=$(wrap_string "$dir_format")

		if [[ ! -f "$file_gyp" ]]; then
			echo "Defining target: $format_target_name (\"$file_gyp\")"
			gyp_template=${gyp_template//'$TARGET_NAME'/$format_target_name}
			gyp_template=${gyp_template//'$INCLUDE_DIRS'/$format_include_dirs}
			gyp_template=${gyp_template//'$SOURCES'/$format_sources}
			echo -e "$gyp_template" > $file_gyp
		else
			echo "Skipping: $format_target_name (\"$file_gyp\")"
		fi

		if [[ $directory == *ogrsf* ]]; then
			frmt_upper=`echo $fmt | tr '[:lower:]' '[:upper:]'`
			if [[ ${frmt_upper} == "MITAB" ]]; then
				format_list_defs="$format_list_defs"$'\n'"TAB_ENABLED=1"
			fi
			format_list_defs="$format_list_defs"$'\n'"${frmt_upper}_ENABLED=1"
		else
			format_list_defs="$format_list_defs"$'\n'"FRMT_$fmt=1"
		fi

		format_list_gyps="$format_list_gyps"$'\n'"$file_gyp:$target_name"
	done
}

generate_formats "$GDAL_FORMATS" "${dir_gdal}/frmts"
generate_formats "$OGR_FORMATS" "${dir_gdal}/ogr/ogrsf_frmts" "ogr_"

#
# create format list
#

format_list_plain=`echo -e "$GDAL_FORMATS" | perl -p -000 -e 's/\s+/ /g'`
format_list_gyps=$(wrap_array "$format_list_gyps" 3)
format_list_defs=$(wrap_array "$format_list_defs" 3)

formats_gypi=`cat ${dir_gyp_templates}/libgdal_formats.gypi`
formats_gypi=${formats_gypi//'$GDAL_FORMATS'/$format_list_plain}
formats_gypi=${formats_gypi//'$GDAL_FORMAT_GYPS'/$format_list_gyps}
formats_gypi=${formats_gypi//'$GDAL_FORMAT_DEFS'/$format_list_defs}

echo -e "$formats_gypi" > libgdal_formats.gypi

(cd ./deps/libgdal/gdal/gcore && ./generate_gdal_version_h.sh)
