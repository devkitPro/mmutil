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

// MAXMOD SOLUTION

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "errors.h"
#include "defs.h"
#include "files.h"
#include "mas.h"
#include "mod.h"
#include "s3m.h"
#include "xm.h"
#include "it.h"
#include "wav.h"
#include "simple.h"
#include "version.h"
#include "systems.h"

FILE*	F_SCRIPT=NULL;

FILE*	F_SAMP=NULL;
FILE*	F_SONG=NULL;

FILE*	F_HEADER=NULL;

u16		MSL_NSAMPS;
u16		MSL_NSONGS;

char	str_msl[256];

void MSL_PrintDefinition( char* filename, u16 id, char* prefix );

void MSL_Erase( void )
{
	MSL_NSAMPS = 0;
	MSL_NSONGS = 0;
	file_delete( "samp.tmp" );
	file_delete( "songs.tmp" );
}

u16 MSL_AddSample( Sample* samp )
{
	u32 sample_length;
	u32 sample_looplen;
	u32 x;
	file_open_write_end( "samp.tmp" );
	
	sample_length = samp->sample_length;
	sample_looplen = samp->loop_end-samp->loop_start;
	
	write32( ((samp->format & SAMPF_16BIT) ? sample_length*2 : sample_length ) + 12  +4); // +4 for sample padding
	if( target_system == SYSTEM_GBA )
		write8( 1 );
	else
		write8( 2 );
	write8( MAS_VERSION );
	write8( BYTESMASHER ); write8( BYTESMASHER );
	
	if( target_system == SYSTEM_GBA )
	{
		write32( sample_length );
		write32( samp->loop_type ? sample_looplen : 0xFFFFFFFF );
		write8( SAMP_FORMAT_U8 );//write8( BYTESMASHER );
		write8( BYTESMASHER );
		write16( (u16) ((samp->frequency * 1024 + (15768/2)) / 15768) );
	}
	else
	{
		if( (samp->format & SAMPF_16BIT) )
		{
			if( samp->loop_type )
			{
				write32( samp->loop_start / 2 );
				write32( (samp->loop_end-samp->loop_start) / 2 );
			}
			else
			{
				int a,b=0;
				a=sample_length/2;
				if( a > 65535 )
				{
					b = a - 65535;
					a = 65535;
				}
				write32( 0 );
				write32( sample_length/2 );
			}
		}
		else
		{
			if( samp->loop_type )
			{
				write32( samp->loop_start / 4 );
				write32( (samp->loop_end-samp->loop_start) / 4 );
			}
			else
			{
				int a,b=0;
				a=sample_length/4;
				if( a > 65535 )
				{
					b = a - 65535;
					a = 65535;
				}
				write32( 0 );
				write32( sample_length/4 );
			}
		}
		write8( sample_dsformat( samp ) );
		write8( sample_dsreptype( samp ) );
		write16( (u16) ((samp->frequency * 1024 + (32768/2)) / 32768) );
	}
	
	// write sample data
	if( samp->format & SAMPF_16BIT )
	{
		for( x = 0; x < sample_length; x++ )
			write16( ((u16*)samp->data)[x] );

		// add padding data
		if( samp->loop_type )
		{
			write16( ((u16*)samp->data)[samp->loop_start] );
			write16( ((u16*)samp->data)[samp->loop_start+1] );
		}
		else
		{
			write16( 0 );
			write16( 0 );
		}
	}
	else
	{
		for( x = 0; x < sample_length; x++ )
			write8( ((u8*)samp->data)[x] );

		// add padding data
		if( samp->loop_type )
		{
			write8( ((u8*)samp->data)[samp->loop_start] );
			write8( ((u8*)samp->data)[samp->loop_start+1] );
			write8( ((u8*)samp->data)[samp->loop_start+2] );
			write8( ((u8*)samp->data)[samp->loop_start+3] );
		}
		else
		{
			write8( 0 );
			write8( 0 );
			write8( 0 );
			write8( 0 );
		}
	}

	file_close_write();
	MSL_NSAMPS++;
	return MSL_NSAMPS-1;
}

u16 MSL_AddSampleC( Sample* samp )
{
	u32 st;
	u32 samp_len;
	u32 samp_llen;
	u8 sformat;

	u32 h_filesize;
	int samp_id;
	bool samp_match;
		
	int fsize=file_size( "samp.tmp" );
	if( fsize == 0 )
	{
		return MSL_AddSample( samp );
	}
	F_SAMP = fopen( "samp.tmp", "rb" );
	fseek( F_SAMP, 0, SEEK_SET );
	samp_id = 0;
	while( ftell( F_SAMP ) < fsize )
	{
		h_filesize = read32f( F_SAMP );
		read32f( F_SAMP );
		samp_len = read32f( F_SAMP );
		samp_llen = read32f( F_SAMP );
		sformat = read8f( F_SAMP );		/////// BUG! GBA DOESNLT WRITE FORMAT!?
		skip8f( 3, F_SAMP );
		samp_match=true;
		if( samp->sample_length == samp_len && ( samp->loop_type ? samp->loop_end-samp->loop_start : 0xFFFFFFFF ) == samp_llen && sformat == sample_dsformat( samp ) )
		{
			// verify sample data
			if( samp->format & SAMPF_16BIT )
			{
				for( st=0; st<samp_len; st++ )
				{
					if( read16f( F_SAMP ) != ((u16*)samp->data)[st] )
					{
						samp_match = false;
						break;
					}
				}
			}
			else
			{
				for( st=0; st<samp_len; st++ )
				{
					if( read8f( F_SAMP ) != ((u8*)samp->data)[st] )
					{
						samp_match = false;
						break;
					}
				}
			}
			if( samp_match )
			{
				fclose( F_SAMP );
				return samp_id;
			}
			else
			{
				skip8f( (h_filesize-12) - (st+1)  , F_SAMP );		// +4 to skip padding
			}
		}
		else
		{
			skip8f( h_filesize-12  , F_SAMP ); // +4 to skip padding
		}
		samp_id++;
	}
	fclose( F_SAMP );
	return MSL_AddSample( samp );
}

u16 MSL_AddModule( MAS_Module* mod )
{
	int x;
	int samp_id;
	// ADD SAMPLES
	for( x = 0; x < mod->samp_count; x++ )
	{
		samp_id = MSL_AddSampleC( &mod->samples[x] );
		MSL_PrintDefinition( mod->samples[x].filename, (u16)samp_id, "SFX_" );
		mod->samples[x].msl_index = samp_id;
	}
	
	file_open_write_end( "songs.tmp" );
	Write_MAS( mod, false, true );
	file_close_write();
	MSL_NSONGS++;
	return MSL_NSONGS-1;
}

void MSL_Export( char* filename )
{
	u32 x;
	u32 y;
	u32 file_size;

	u32* parap_samp;
	u32* parap_song;

	file_open_write( filename );
	write16( MSL_NSAMPS );
	write16( MSL_NSONGS );
	write32( 0xbabababa );
	write32( 0xbabababa );
	
	parap_samp = (u32*)malloc( MSL_NSAMPS * sizeof( u32 ) );
	parap_song = (u32*)malloc( MSL_NSONGS * sizeof( u32 ) );
	
	// reserve space for parapointers
	for( x = 0; x < MSL_NSAMPS; x++ )
		write32( 0xAAAAAAAA );
	for( x = 0; x < MSL_NSONGS; x++ )
		write32( 0xAAAAAAAA );
	// copy samples
	file_open_read( "samp.tmp" );
	for( x = 0; x < MSL_NSAMPS; x++ )
	{
		align32();
		parap_samp[x] = file_tell_write();
		file_size = read32();
		write32( file_size );
		for( y = 0; y < file_size+4; y++ )
			write8( read8() );
	}
	file_close_read();
	
	file_open_read( "songs.tmp" );
	for( x = 0; x < MSL_NSONGS; x++ )
	{
		align32();
		parap_song[x] = file_tell_write();
		file_size = read32();
		write32( file_size );
		for( y = 0; y < file_size+4; y++ )
			write8( read8() );
	}
	file_close_read();
	
	file_seek_write( 0x0C, SEEK_SET );
	for( x = 0; x < MSL_NSAMPS; x++ )
		write32( parap_samp[x] );
	for( x=  0; x < MSL_NSONGS; x++ )
		write32( parap_song[x] );

	if( parap_samp )
		free( parap_samp );
	if( parap_song )
		free( parap_song );
}

void MSL_PrintDefinition( char* filename, u16 id, char* prefix )
{
	char newtitle[64];
	int x,s=0;
	if( filename[0] == 0 )	// empty string
		return;
	for( x = 0; x < (int)strlen( filename ); x++ )
	{
		if( filename[x] == '\\' || filename[x] == '/' ) s = x+1; 
	}
	for( x = s; x < (int)strlen( filename ); x++ )
	{
		if( filename[x] != '.' )
		{
			newtitle[x-s] = toupper(filename[x]);
			if( newtitle[x-s] >= ' ' && newtitle[x-s] <= '/' )
				newtitle[x-s] = '_';
			if( newtitle[x-s] >= ':' && newtitle[x-s] <= '@' )
				newtitle[x-s] = '_';
			if( newtitle[x-s] >= '[' && newtitle[x-s] <= '`' )
				newtitle[x-s] = '_';
			if( newtitle[x-s] >= '{' )
				newtitle[x-s] = '_';
		}
		else
		{
			break;
		}
	}
	newtitle[x-s] = 0;
	if( F_HEADER )
	{
		fprintf( F_HEADER, "#define %s%s	%i\r\n", prefix, newtitle, id );
	}
}

void MSL_LoadFile( char* filename, bool verbose )
{
	Sample wav;
	MAS_Module mod;
	int f_ext;
	if( file_open_read( filename ) )
	{
		printf( "Cannot open %s for reading! Skipping.\n", filename );
		return;
	}
	f_ext = get_ext( filename );
	switch( f_ext )
	{
	case INPUT_TYPE_MOD:
		Load_MOD( &mod, verbose );
		MSL_PrintDefinition( filename, MSL_AddModule( &mod ), "MOD_" );
		Delete_Module( &mod );
		break;
	case INPUT_TYPE_S3M:
		Load_S3M( &mod, verbose );
		MSL_PrintDefinition( filename, MSL_AddModule( &mod ), "MOD_" );
		Delete_Module( &mod );
		break;
	case INPUT_TYPE_XM:
		Load_XM( &mod, verbose );
		MSL_PrintDefinition( filename, MSL_AddModule( &mod ), "MOD_" );
		Delete_Module( &mod );
		break;
	case INPUT_TYPE_IT:
		Load_IT( &mod, verbose );
		MSL_PrintDefinition( filename, MSL_AddModule( &mod ), "MOD_" );
		Delete_Module( &mod );
		break;
	case INPUT_TYPE_WAV:
		Load_WAV( &wav, verbose );
		MSL_PrintDefinition( filename, MSL_AddSample( &wav ), "SFX_" );
		free( wav.data );
		break;
	default:
		// print error/warning
		printf( "Unknown file %s...\n", filename );
	}
	file_close_read();
	
}

int MSL_Create( char* argv[], int argc, char* output, char* header, bool verbose )
{
//	int str_w=0;
//	u8 pmode=0;
//	bool comment=false;

	int x;

	MSL_Erase();
	str_msl[0] = 0;
	F_HEADER=NULL;
	if( header )
	{
		F_HEADER = fopen( header, "wb" );
	}

//	if( !F_HEADER )
//		return -1;	// needs header file!
	
	file_open_write( "samp.tmp" );
	file_close_write();
	file_open_write( "songs.tmp" );
	file_close_write();
	
	for( x = 1; x < argc; x++ )
	{
		if( argv[x][0] == '-' )
		{
			
		}
		else
		{
			MSL_LoadFile( argv[x], verbose );
		}
	}

	MSL_Export( output );

	if( F_HEADER )
	{
		fprintf( F_HEADER, "#define MSL_NSONGS	%i\r\n", MSL_NSONGS );
		fprintf( F_HEADER, "#define MSL_NSAMPS	%i\r\n", MSL_NSAMPS );
		fprintf( F_HEADER, "#define MSL_BANKSIZE	%i\r\n", (MSL_NSAMPS+MSL_NSONGS) );
		fclose( F_HEADER );
		F_HEADER=NULL;
	}

	file_delete( "samp.tmp" );
	file_delete( "songs.tmp" );
	return ERR_NONE;
}
