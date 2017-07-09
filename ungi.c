#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzss.h"

static inline void * checked_malloc( size_t size )
{
    void * x = malloc( size );
    if( !x )
    {
        fprintf( stderr, "Allocation of 0x%zX bytes failed\n", size );
        exit( EXIT_FAILURE );
    }
    return memset( x, 0, size );
}

int main( const int argc, const char * const * const argv )
{
    if( argc < 2 || argc > 3 )
    {
        fprintf( stderr, "Usage: ungi compressed.gi [uncompressed.bin]\n" );
        return argv ? EXIT_FAILURE : EXIT_SUCCESS;
    }
    FILE * infile = fopen( argv[1], "rb" );
    if( !infile )
    {
        fprintf( stderr, "Can't open file: %s\n", argv[1] );
        return EXIT_FAILURE;
    }
    FILE * outfile = NULL;
    if( argc == 3 )
    {
        outfile = fopen( argv[2], "wb" );
        if( !outfile )
        {
            fprintf( stderr, "Can't open file: %s\n", argv[2] );
            return EXIT_FAILURE;
        }
    }
    else
    {
        size_t i = strlen( argv[1] );
        char * filename = checked_malloc( i + 5 );
        memcpy( filename, argv[1], i );
        filename[i] = '.';
        filename[i + 1] = 'b';
        filename[i + 2] = 'i';
        filename[i + 3] = 'n';
        outfile = fopen( filename, "wb" );
        if( !outfile )
        {
            fprintf( stderr, "Can't open file: %s\n", filename );
            return EXIT_FAILURE;
        }
        free( filename );
    }
    
    uint8_t * input = checked_malloc( 0x20 );
    if( !fread( input, 0x20, 1, infile ) )
    {
        fprintf( stderr, "Can't read file: %s\n", argv[1] );
        return EXIT_FAILURE;
    }
    uint_fast32_t inlength = (uint_fast32_t)input[2] |
                             (uint_fast32_t)input[3] << 8 |
                             (uint_fast32_t)input[4] << 16 |
                             (uint_fast32_t)input[5] << 24;
    //fprintf( stdout, "Input length: %08" PRIXFAST32 "\n", inlength );
    uint_fast32_t outlength = (uint_fast32_t)input[6] |
                              (uint_fast32_t)input[7] << 8 |
                              (uint_fast32_t)input[8] << 16 |
                              (uint_fast32_t)input[9] << 24;
    //fprintf( stdout, "Output length: %08" PRIXFAST32 "\n", outlength );
    free( input );
    input = checked_malloc( inlength );
    if( !fread( input, inlength, 1, infile ) )
    {
        fprintf( stderr, "Can't read file: %s\n", argv[1] );
        return EXIT_FAILURE;
    }
    uint_fast32_t offset = (uint_fast32_t)(input[0] & ~0xc0) << 24 |
                           (uint_fast32_t)input[1] << 16 |
                           (uint_fast32_t)input[2] << 8 |
                           (uint_fast32_t)input[3];
    if( offset != 0x100000 )
    {
        fprintf( stderr, "Unexpected offset! (0x%08" PRIXFAST32 ")\n", offset );
    }
    uint8_t * output = checked_malloc( outlength );
    for( uint_fast32_t i = 0; i < inlength; i += 0x13c8 )
    {
        uint_fast32_t start = ((uint_fast32_t)(input[i] & ~0xc0) << 24 |
                               (uint_fast32_t)input[i + 1] << 16 |
                               (uint_fast32_t)input[i + 2] << 8 |
                               (uint_fast32_t)input[i + 3]) - offset;
        uint_fast32_t length = (uint_fast32_t)input[i + 4] << 9 |
                               (uint_fast32_t)input[i + 5] << 1;
        fprintf( stdout, "Debug: %08" PRIXFAST32 " %02" PRIX8 " %08" PRIXFAST32 " %05" PRIXFAST32 "\n", i+0x20, input[i], start, length );
        if( input[i] & 0x80 )
        {
            memcpy( output + start, input + i + 6, length );
        }
        else
        {
            if( decompress( input + i + 6, output + start, 0x13c2, length ) != length )
            {
                fprintf( stderr, "Invalid compressed data!\n" );
                return EXIT_FAILURE;
            }
        }
    }
    fwrite( output, outlength, 1, outfile );
    return EXIT_SUCCESS;
}
