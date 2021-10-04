#include <stddef.h>
#include "lzss.h"

uint_fast32_t decompress( const uint8_t * input, uint8_t * output, uint_fast32_t inlength, uint_fast32_t outlength )
{
    uint_fast32_t inptr = 0;
    uint_fast32_t outptr = 0;
    while( 1 )
    {
        if( inptr >= inlength ) return outptr;
        uint_fast32_t control = (uint_fast32_t)input[ inptr++ ] << 24;
        if( inptr >= inlength ) return outptr;
        control |= (uint_fast32_t)input[ inptr++ ] << 16;
        control |= 0xffff;
        while( control & 0xffff )
        {
            if( control & 0x80000000 )
            {
                if( inptr >= inlength ) return outptr;
                uint_fast16_t run = (uint_fast16_t)input[ inptr++ ] << 8;
                if( inptr >= inlength ) return outptr;
                run |= (uint_fast16_t)input[ inptr++ ];
                ptrdiff_t runptr = (run >> 4) << 1;
                runptr = (ptrdiff_t)outptr - runptr;//runptr < outptr ? outptr - runptr : outptr;
                run = ((run & 0xf) + 1) << 1;
                for( uint_fast16_t i = 0; i < run; i++ )
                {
                    output[ outptr++ ] = output[ runptr++ ];
                    if( outptr >= outlength ) return outptr;
                }
            }
            else
            {
                if( inptr >= inlength ) return outptr;
                output[ outptr++ ] = input[ inptr++ ];
                if( outptr >= outlength ) return outptr;
                if( inptr >= inlength ) return outptr;
                output[ outptr++ ] = input[ inptr++ ];
                if( outptr >= outlength ) return outptr;
            }
            control <<= 1;
        }
    }
    return outptr;
}
