/*-----------------------------------------------------------------------------------------
Copyright (c) 2007, Mukunda Johnson

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the owners nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

-----------------------------------------------------------------------------------------*/

#ifndef SIMPLE_H
#define SIMPLE_H

#define INPUT_TYPE_MOD	0
#define INPUT_TYPE_S3M	1
#define INPUT_TYPE_XM	2
#define INPUT_TYPE_IT	3
#define INPUT_TYPE_WAV	4
#define INPUT_TYPE_TXT	5
#define INPUT_TYPE_UNK	6
#define INPUT_TYPE_H	7
#define INPUT_TYPE_MSL	8

int get_ext( char* filename );
u32 calc_samplooplen( Sample* s );
u32 calc_samplen( Sample* s );
u32 calc_samplen_ex2( Sample* s );
int clamp_s8( int value );
int clamp_u8( int value );
u32 readbits(u8* buffer, unsigned int pos, unsigned int size);

u8 sample_dsformat( Sample* samp );
u8 sample_dsreptype( Sample* samp );

#endif
