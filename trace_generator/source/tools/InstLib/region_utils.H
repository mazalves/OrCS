/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2019 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
#ifndef REGION_UTILS_H
#define REGION_UTILS_H

#include <sstream> 
#include <string.h>

using namespace std;
namespace CONTROLLER{
class REGION_UTILS {
public:
    static UINT32 StringToUINT32(string &s, const char * name, UINT32 base=10)
    {
        char* end = 0 ;
        INT32 retval = strtoul(s.c_str(), &end, base); 
        ASSERT((*end == 0), "ERROR reading " + name + " from " + s);
        ASSERT((retval >=0 ), name + " (" + s  + ") must be positive " );
        return (UINT32)retval;
    }

    static UINT64 StringToUINT64(string &s, const char * name, UINT32 base=10)
    {
        char* end = 0 ;
        INT64 retval = strtoull(s.c_str(), &end, base); 
        ASSERT((*end == 0), "ERROR reading " + name + " from " + s);
        ASSERT((retval >=0 ), name + " (" + s  + ") must be positive " );
        return (UINT64)retval;
    }

    static ADDRINT StringToADDRINT(string &s, const char * name)
    {
#if defined(TARGET_IA32)
        return StringToUINT32(s, name, 16);
#else
        return StringToUINT64(s, name, 16);
#endif
    }

    static double StringToDouble(string &s, const char * name )
    {
        char* end = 0 ;
        double retval = strtod(s.c_str(), &end); 
        ASSERT((*end == 0), "ERROR reading " + name + " from " + s);
        return retval;
    }

    // Convert weight to string for setting region name
    static string WeightToString(UINT32 weightX100000)
    {
        CHAR weight_array[8]; // xxx.xxx
        string prefix = (weightX100000 == 100000) ? "1" : "0";

        // WeightTimesHundredThousand naming
        ASSERTX(weightX100000<10000000);
        sprintf(weight_array, "%07d", weightX100000);

        for (UINT32 d = 1; d < 7; d++)
        {
            weight_array[d] = weight_array[d + 1];
        }
        weight_array[0] = '-';
        weight_array[7] = 0;
        return prefix + string(weight_array);
    }
};
}
#endif
