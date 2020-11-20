#ifndef _types_h
#define _types_h

#include <string>
#include <optional>

using std::optional;
using std::nullopt;

#ifndef uint
    typedef unsigned int uint;
#endif
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef int64_t uint64;

using AccessWidth = uint16;
const AccessWidth maxAccessWidth = UINT16_MAX;

#include <llvm/ADT/APInt.h>

#endif // _types_h