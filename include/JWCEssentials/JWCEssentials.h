#ifndef JWCESSENTIALS_H

#define JWCESSENTIALS_H

#ifdef BUILD_JWCESSENTIALS
    #define _EXPORT_ __EXPORT__
    #define _CLASSEXPORT_ __CLASSEXPORT__
#else
    #define _EXPORT_ __IMPORT__
    #define _CLASSEXPORT_ __CLASSIMPORT__
#endif

#include "utf8_string_struct.h"
#include "struct_array_struct.h"
#include "Paths.h"
#include "Escapes.h"
#include "feffect.h"

namespace JWCEssentials {
};


#undef _EXPORT_
#undef  _CLASSEXPORT_

#endif //JWCESSENTIALS_H
