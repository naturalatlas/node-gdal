#ifndef GDAL_SDE_INCLUDED
#define GDAL_SDE_INCLUDED

#include "gdal_pam.h"


CPL_CVSID("$Id: gdal_sde.h 32190 2015-12-16 13:50:27Z goatbar $");

CPL_C_START
void GDALRegister_SDE();
CPL_C_END

#include <sdetype.h>
#include <sdeerno.h>
#include <sderaster.h>

#include "cpl_string.h"
#include "ogr_spatialref.h"

#include "sdeerror.h"
#include "sderasterband.h"
#include "sdedataset.h"

#endif
