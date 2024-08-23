#ifndef JWCESSENTIALS_H

#define JWCESSENTIALS_H

// Note - this block may need to be moved into the individual headers in the future so including another header within doesn't reset it
#ifdef BUILD_JWCESSENTIALS
    #define _EXPORT_ __EXPORT__
    #define _CLASSEXPORT_ __CLASSEXPORT__
#else
    #define _EXPORT_ __IMPORT__
    #define _CLASSEXPORT_ __CLASSIMPORT__
#endif

#define P_INSTANCE(type) type*
#define P_IN_OUT(type) type*
#define P_OUT(type) type*
#define P_ELEMENTS(type) type*


#include "SingleLink_Node.h"
#include "utf8_string_struct.h"
#include "struct_array_struct.h"
#include "Paths.h"
#include "Escapes.h"
#include "feffect.h"
#include "BufferHelper.h"
#include "Random_Generator.h"
#include "Random_MT19937.h"

#include "HasherClass.h"
#include "HasherClass32.h"
#include "HasherClass64.h"
#include "Hasher_SMHasher.h"
#include "Hasher_BufferedForCallback32.h"
#include "Hasher_BufferedForCallback64.h"
#include "Hasher_PRNG32.h"
#include "Hasher_PRNG64.h"

#include "Random_Generator.h"
#include "Random_MT19937.h"

namespace JWCEssentials {
};


#undef _EXPORT_
#undef  _CLASSEXPORT_

#endif //JWCESSENTIALS_H
