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

#define MAX_UNROLL_THRESHOLD	1024	// will unroll upto 1kb more of data (when fixing NDS samples)
#define GBA_MIN_LOOP_SIZE 512

#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "mas.h"
#include "errors.h"
#include "systems.h"
#include "adpcm.h"

extern int ignore_sflags;

void Sample_PadStart( Sample* samp, u32 count )
{
	// Pad beginning of sample with zero
	u8* newdata8;
	u16* newdata16;
	u32 x;
	if( count == 0 )
		return;		// nothing to do
	if( samp->format & SAMPF_16BIT )
	{
		newdata16 = (u16*)malloc( (samp->sample_length+count)*2 );
		for( x = 0; x < count; x++ )
			newdata16[x]=32768;
		for( x = 0; x < samp->sample_length; x++ )
			newdata16[count+x]=((u16*)samp->data)[x];
		free( samp->data );
		samp->data = (void*)newdata16;
	}
	else
	{
		newdata8 = (u8*)malloc( (samp->sample_length+count) );
		for( x = 0; x < count; x++ )
			newdata8[x]=128;
		for( x = 0; x < samp->sample_length; x++ )
			newdata8[count+x] = ((u8*)samp->data)[x];
		free( samp->data );
		samp->data = (void*)newdata8;
	}
	samp->loop_start    += count;
	samp->loop_end      += count;
	samp->sample_length += count;
}

void Sample_PadEnd( Sample* samp, u32 count )
{
	// Pad end of sample with zero
	u8* newdata8;
	u16* newdata16;
	u32 x;
	if( count == 0 )
		return;		// nothing to do
	if( samp->format & SAMPF_16BIT )
	{
		newdata16 = malloc( (samp->sample_length+count)*2 );
		for( x = 0; x < samp->sample_length; x++ )
			newdata16[x]= ((u16*)samp->data)[x];
		for( x = 0; x < count; x++ )
			newdata16[samp->sample_length+x]=32768;
		free( samp->data );
		samp->data = (void*)newdata16;
	}
	else
	{
		newdata8 = malloc( (samp->sample_length+count) );
		for( x = 0; x < samp->sample_length; x++ )
			newdata8[x]= ((u8*)samp->data)[x];
		for( x = 0; x < count; x++ )
			newdata8[samp->sample_length+x]=128;
		free( samp->data );
		samp->data = (void*)newdata8;
	}
	samp->loop_end      += count;
	samp->sample_length += count;
}

void Unroll_Sample_Loop( Sample* samp, u32 count )
{
	// unrolls sample loop (count) times
	// loop end MUST equal sample length
	u8* newdata8;
	u16* newdata16;
	u32 newlen;
	u32 looplen;
	u32 x;
	looplen = samp->loop_end-samp->loop_start;
	newlen = samp->sample_length + looplen*count;
	if( samp->format & SAMPF_16BIT )
	{
		newdata16 = (u16*)malloc( newlen *2 );
		for( x = 0; x < samp->sample_length; x++ )
			newdata16[x] = ((u16*)samp->data)[x];
		for( x = 0; x < looplen*count; x++ )
			newdata16[samp->sample_length+x] = ((u16*)samp->data)[samp->loop_start+ (x%looplen)];
		free( samp->data );
		samp->data = (void*)newdata16;
	}
	else
	{
		newdata8 = (u8*)malloc( newlen );
		for( x = 0; x < samp->sample_length; x++ )
			newdata8[x] = ((u8*)samp->data)[x];
		for( x = 0; x < looplen*count; x++ )
			newdata8[samp->sample_length+x] = ((u8*)samp->data)[samp->loop_start+ (x%looplen)];
		free( samp->data );
		samp->data = (void*)newdata8;
	}
	samp->loop_end += looplen*count;
	samp->sample_length += looplen*count;
}

void Unroll_BIDI_Sample( Sample* samp )
{
	// sample length MUST equal sample loop end
	// sample MUST have loop type 2 (BIDI)
	u8* newdata8;
	u16* newdata16;
	u32 newlen;
	u32 looplen;
	u32 x;

	looplen = samp->loop_end-samp->loop_start;
	newlen = (samp->sample_length + looplen);
	
	if( samp->format & SAMPF_16BIT )
	{
		newdata16 = malloc( newlen *2 );
		for( x = 0; x < samp->sample_length; x++ )
			newdata16[x] = ((u16*)samp->data)[x];
		for( x = 0; x < looplen; x++ )
			newdata16[x+samp->sample_length] = ((u16*)samp->data)[samp->loop_end-1-x];
		free( samp->data );
		samp->data = (void*)newdata16;
	}
	else
	{
		newdata8 = malloc( newlen );
		for( x = 0; x < samp->sample_length; x++ )
			newdata8[x] = ((u8*)samp->data)[x];
		for( x = 0; x < looplen; x++ )
			newdata8[x+samp->sample_length] = ((u8*)samp->data)[samp->loop_end-1-x];
		free( samp->data );
		samp->data = (void*)newdata8;
	}
	samp->loop_type = 1;
	samp->sample_length += looplen;
	samp->loop_end += looplen;
}

/* NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE
  The following resample routine was stolen from CHIBITRACKER (http://chibitracker.berlios.de), thanks reduz!
 NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE */

void Resample( Sample* samp, u32 newsize )
{
	
	u8* new_data8 = NULL;
	u16* new_data16 = NULL;
	u32 oldsize = samp->sample_length;
	u32 i;
	bool bit16 = samp->format & SAMPF_16BIT;
	
	if( bit16 )
		new_data16 = (u16*)malloc(newsize*2);
	else
		new_data8 = (u8*)malloc(newsize);
	
	for( i=0; i<newsize ; i++ ) {
		
		float pos=(float)i*(float)oldsize/(float)newsize;
		u32 posi=(u32)pos;
		float mu=pos-(float)posi;
		float mu2,a0,a1,a2,a3,res;
		float y0,y1,y2,y3;
		
		if( bit16 )
		{
			y0=(posi-1)<0?0:		((float)((u16*)samp->data)[posi-1])	-32768;	// sign data
			y1=						((float)((u16*)samp->data)[posi])	-32768;
			y2=(posi+1)>=oldsize?0:	((float)((u16*)samp->data)[posi+1])	-32768;
			y3=(posi+2)>=oldsize?0:	((float)((u16*)samp->data)[posi+2])	-32768;
		}
		else
		{
			y0=(posi-1)<0?0:		((float)((u8*)samp->data)[posi-1])	-128;	// sign data
			y1=						((float)((u8*)samp->data)[posi])	-128;
			y2=(posi+1)>=oldsize?0:	((float)((u8*)samp->data)[posi+1])	-128;
			y3=(posi+2)>=oldsize?0:	((float)((u8*)samp->data)[posi+2])	-128;
		}
		
		mu2 = mu*mu;
		a0 = y3 - y2 - y0 + y1;
		a1 = y0 - y1 - a0;
		a2 = y2 - y0;
		a3 = y1;
		
		res=(a0*mu*mu2+a1*mu2+a2*mu+a3);
		if( bit16 )
		{
			if (res<-32768)
				res=-32768;
			if (res>32767)
				res=32767;
			new_data16[i] = (int)res+32768;	// unsign data
		}
		else
		{
			if (res<-128)
				res=-128;
			if (res>127)
				res=127;
			new_data8[i] = (int)res+128;	// unsign data
		}
	}
	
	free( samp->data );
	if( bit16 )
		samp->data = (void*)new_data16;
	else
		samp->data = (void*)new_data8;
	
	samp->sample_length = newsize;
	samp->loop_end = newsize;
	samp->loop_start = (int)(((double)samp->loop_start * (double)newsize+((double)oldsize/2))/(double)oldsize);
	samp->frequency = (int)(((double)samp->frequency * (double)newsize+((double)oldsize/2))/(double)oldsize);
}

void Sample_8bit( Sample* samp )
{
	if( samp->format & SAMPF_16BIT )
	{
		u8* newdata;
		u32 t;
		newdata = (u8*)malloc( samp->sample_length );
		for( t = 0; t < samp->sample_length; t++ )
			newdata[t] = ((u16*)samp->data)[t] / 256;
		free( samp->data );
		samp->data = newdata;
//		samp->bit16=false;
		samp->format &= ~SAMPF_16BIT;
	}
}

void Sample_Sign( Sample* samp )
{
	// sample must be unsigned
	u32 x;
	if( samp->format & SAMPF_16BIT )
	{
		for( x =0 ; x < samp->sample_length; x++ )
		{
			int a =  (( (int) ((u16*)samp->data)[x] ) - 32768);
			if(a < -32767) a = -32767;						// clamp LOW to -32767 (leave space for interpolation error)
//			if(a > 32765)  a = 32765;						// clamp HIGH to 32766
			((u16*)samp->data)[x] = (u16)a ;
		}
	}
	else
	{
		for( x =0 ; x < samp->sample_length; x++ )
		{
			int a =  (( (int) ((u8*)samp->data)[x] ) - 128);
			if( a == -128 ) a = -127;
			((u8*)samp->data)[x] = (u8) a;
		}

	}
	samp->format |= SAMPF_SIGNED;
}

void FixSample_GBA( Sample* samp )
{
	// convert to 8-bit if neccesary
	Sample_8bit( samp );
	
	// delete data after loop_end if loop exists
	if( samp->loop_type != 0 )
		samp->sample_length = samp->loop_end;
	
	// unroll BIDI loop
	if( samp->loop_type == 2 )
		Unroll_BIDI_Sample( samp );

	if( samp->loop_type )
	{
		if( samp->loop_end-samp->loop_start < GBA_MIN_LOOP_SIZE )
		{
			Unroll_Sample_Loop( samp, (GBA_MIN_LOOP_SIZE / (samp->loop_end-samp->loop_start))+1 );
		}
	}
}

int strcmpshit( char* str1, char* str2 )
{
	int x=0;
	int f=0;
	while( str1[x] != 0 )
	{
		if( str1[x] == str2[f] )f++;
		else					f=0;
		if( str2[f] == 0 )		return 1;
		x++;
	}
	return 0;
}

void FixSample_NDS( Sample* samp )
{
	if( samp->sample_length == 0 )
	{
		// sample has no data
		samp->loop_end=samp->loop_start=0;
		return;
	}
	// delete data after loop_end if loop exists
	if( samp->loop_type != 0 )
		samp->sample_length = samp->loop_end;
	
	// unroll BIDI loop
	if( samp->loop_type == 2 )
		Unroll_BIDI_Sample( samp );

	// %o option
	if( samp->loop_type )
	{
		if( !ignore_sflags )
		{
			if( ((strcmpshit( samp->name, "%o"  )) > 0) )
			{
				Unroll_Sample_Loop( samp, 1 );
				samp->loop_start += (samp->loop_end-samp->loop_start) / 2;
			}
		}
	}

	if( !ignore_sflags )
	{
		if( ((strcmpshit( samp->name, "%c" )) > 0) )
		{
			samp->format |= SAMPF_COMP;
		}
	}
	
	// Resize loop
	if( samp->loop_type )
	{
		int looplen = samp->loop_end-samp->loop_start;
		if( !(samp->format & SAMPF_COMP) )
		{
			if( samp->format & SAMPF_16BIT )
			{
				if( looplen & 1 )
				{
					int addition = (samp->loop_end - samp->loop_start);
					if( addition > MAX_UNROLL_THRESHOLD )
						Resample( samp, samp->sample_length +1 );
					else
						Unroll_Sample_Loop( samp, 1 );
				}
			}
			else
			{
				if( looplen & 3 )
				{
					int count;
					int addition;
					count = looplen & 3;
					switch( count ) {
					case 0:
						count=0;	break;
					case 1:
						count=3;	break;
					case 2:
						count=1;	break;
					case 3:
						count=3;	break;
					}
					addition = looplen*count;
					if( addition > MAX_UNROLL_THRESHOLD )
						Resample( samp, samp->sample_length + (4-(looplen & 3)) );
					else
						Unroll_Sample_Loop( samp, count );
				}
			}
		}
		else
		{
			int a = looplen;
			int count=0, addition;
			while( looplen & 7 )
			{
				count++;
				looplen += a;
			}
			addition = looplen*count;
			if( addition > MAX_UNROLL_THRESHOLD )
				Resample( samp, samp->sample_length + (4-(looplen & 7)) );
			else
				Unroll_Sample_Loop( samp, count );
		}
	}
	
	// Align loop_start
	if( samp->loop_type )
	{
		int padsize;
		if( !(samp->format & SAMPF_COMP) )
		{
			if( samp->format & SAMPF_16BIT ) {
				padsize = ( (2 - (samp->loop_start & 1)) & 1 );
			} else {
				padsize = ( (4 - (samp->loop_start & 3)) & 3 );
			}
		}
		else
		{
			padsize = ( (8 - (samp->loop_start & 7)) & 7 );
		}
		Sample_PadStart( samp, padsize );
	}
	
	// Pad end, only happens when loop is disabled
	if( !(samp->format & SAMPF_COMP) )
	{
		if( samp->format & SAMPF_16BIT )
		{
			if( samp->sample_length & 1 )
			{
				Sample_PadEnd( samp, 2-(samp->sample_length&1) );
			}
		}
		else
		{
			if( samp->sample_length & 3 )
			{
				Sample_PadEnd( samp, 4-(samp->sample_length&3) );
			}
		}
	}
	else
	{
		if( samp->sample_length & 7 )
		{
			Sample_PadEnd( samp, 8-(samp->sample_length&7) );
		}
	}
	
	Sample_Sign( samp );	// DS hardware takes signed samples

	if( samp->format & SAMPF_COMP )
	{
		// compress with IMA-ADPCM hunger owned
		adpcm_compress_sample( samp );
	}
	else
	{
		
	}
}

void FixSample( Sample* samp )
{
	if( target_system == SYSTEM_GBA )
		FixSample_GBA( samp );
	else if( target_system == SYSTEM_NDS )
		FixSample_NDS( samp );
}

