//------------------------------------------------------------------------------------------------------------------------
// Copyright (c) 2007, Mukunda Johnson
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//     * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------------

// a little wav lib

#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "files.h"
#include "mas.h"
#include "wav.h"
#include "simple.h"
#include "samplefix.h"

int Load_WAV( Sample* samp, bool verbose )
{
	int file_size;
	int chunk_size;
	u32 a;
	int bit_depth = 8;
	bool hasformat=false;
	bool hasdata=false;
	unsigned int chunk_code;
	
	if( verbose )
		printf( "Loading WAV...\n" );

	memset( samp, 0, sizeof( Sample ) );
	read32();							// "RIFF"
	file_size = read32() + 8;
	read32();							// "WAVE"
	
	while( 1 )
	{
		if( file_tell_read() >= file_size ) break;
		chunk_code = read32();
		chunk_size = read32();
		switch( chunk_code )
		{
		case ' tmf':	/// format chunk
			a = read16(); // compression code
			if( a != 1 )
			{
				return LOADWAV_UNKNOWN_COMP;				// unknown compression
			}
			a = read16(); // #channels
			if( a != 1 )
			{
				return LOADWAV_TOOMANYCHANNELS;
			}
			samp->frequency = read32();				// sample rate
			read32();								// average something
			read16();								// wBlockAlign
			bit_depth = read16();
			if( bit_depth != 8 && bit_depth != 16 )
			{
				return LOADWAV_UNSUPPORTED_BD;
			}
			
			if( verbose )
			{
				printf( "Sample rate...%i\n", samp->frequency );
				if( bit_depth == 8 )
					printf( "Bit Depth.....8-bit\n" );
				else
					printf( "Bit Depth.....16-bit\n" );
			}
			if( (chunk_size - 0x10) > 0 )
				skip8( (chunk_size - 0x10) );
			hasformat=true;
			break;
		case 'atad':
			if( verbose )
				printf( "Loading Sampledata...\n" );
			switch( bit_depth )
			{
			case 8:
				samp->data = malloc( chunk_size );
				samp->sample_length = chunk_size;
				for( a = 0; a < samp->sample_length; a++ )
					((u8*)samp->data)[a] = read8();				// 
				hasdata=true;
				FixSample( samp );
				break;
			case 16:
				samp->data = malloc( chunk_size );
				samp->sample_length = chunk_size/2;
				for( a = 0; a < samp->sample_length; a++ )
					((u16*)samp->data)[a] = read8() + 32768;
				hasdata=true;
				FixSample( samp );
				break;
			default:
				return LOADWAV_BADDATA;
			}
		default:
			skip8( chunk_size );
		}
	}
	return (hasformat && hasdata) ? LOADWAV_OK : LOADWAV_CORRUPT;
	
}
