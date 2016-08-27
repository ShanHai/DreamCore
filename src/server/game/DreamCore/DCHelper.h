#ifndef _DC_HELPER_H_
#define _DC_HELPER_H_

#include "Common.h"

namespace DC
{
    inline uint32 compressUint32(uint32 first, uint32 second)
    {
        return first & 0xFFFF | second << 16;
    }

    inline void decompressUint32(const uint32& data, uint32 &first, uint32 &second)
    {
        first  = data & 0xFFFF;
        second = data >> 16;
    }
}

#endif