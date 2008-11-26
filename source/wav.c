/****************************************************************************
 *                                                          __              *
 *                ____ ___  ____ __  ______ ___  ____  ____/ /              *
 *               / __ `__ \/ __ `/ |/ / __ `__ \/ __ \/ __  /               *
 *              / / / / / / /_/ />  </ / / / / / /_/ / /_/ /                *
 *             /_/ /_/ /_/\__,_/_/|_/_/ /_/ /_/\____/\__,_/                 *
 *                                                                          *
 *         Copyright (c) 2008, Mukunda Johnson (mukunda@maxmod.org)         *
 *                                                                          *
 * Permission to use, copy, modify, and/or distribute this software for any *
 * purpose with or without fee is hereby granted, provided that the above   *
 * copyright notice and this permission notice appear in all copies.        *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES *
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         *
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  *
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   *
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    *
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  *
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           *
 ****************************************************************************/

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
				samp->format |= SAMPF_16BIT;
				samp->data = malloc( chunk_size );
				samp->sample_length = chunk_size/2;
				for( a = 0; a < samp->sample_length; a++ )
					((u16*)samp->data)[a] = read16() + 32768;
				hasdata=true;
				FixSample( samp );
				break;
			default:
				return LOADWAV_BADDATA;
			}
			break;
		default:
			skip8( chunk_size );
		}
	}
	return (hasformat && hasdata) ? LOADWAV_OK : LOADWAV_CORRUPT;
	
}
