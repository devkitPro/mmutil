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

#ifndef FILES_H
#define FILES_H

#include <stdio.h>

int file_size( char* filename );
int file_open_read( char* filename );
int file_open_write( char* filename );
int file_open_write_end( char* filename );
void file_close_read( void );
void file_close_write( void );
u8 read8( void );
u16 read16( void );
u32 read24( void );
u32 read32( void );
void write8( u8 p_v );
void write16( u16 p_v );
void write24( u32 p_v );
void write32( u32 p_v );
void align16( void );
void align32( void );
void skip8( u32 count );
int file_seek_read( int offset, int mode );
int file_seek_write( int offset, int mode );
int file_tell_read( void );
int file_tell_write( void );

u8 read8f( FILE* p_fin );
u16 read16f( FILE* p_fin );
u32 read32f( FILE* p_fin );
void align32f( FILE* p_file );
void skip8f( u32 count, FILE* p_file );

void file_delete( char* filename );

bool file_exists( char* filename );

int file_get_byte_count();

#define FILE_OPEN_OKAY 0
#define FILE_OPEN_ERROR -1

#endif
