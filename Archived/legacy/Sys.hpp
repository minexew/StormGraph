/*
    Copyright (c) 2012 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#pragma once

#include <StormGraph/Common.hpp>

namespace StormBase
{
    class ISys
    {
        public:
            virtual ~ISys() {}

            virtual double Update() = 0;

            virtual Var_t* VarCreateStr( int flags, const char* name, const char* value ) = 0;

            //virtual const char* VarStr( const char* name, int eh ) = 0;

            //virtual void VarSetStr( const char* name, const char* text ) = 0;

            /*void Var_Clear();
            const char* Var_Get( const char* name, int eh );
            const char* Var_Get2( const char* name, size_t name_length, int eh );
            int Var_GetInt( const char* name, int* value, int eh );
            int Var_SetStr( const char* name, const char* str );*/

            //virtual int VarInt( const char* name, int eh );
    };
}
