#include "dre2.h"

int
main( int argc, char *argv[] )
{
  struct dre2 *re;                  // Pointer to regex digraph object.
  unsigned char *buf;               // Buffer to hold input strings.
  struct dre2_match_value result;   // Structure that holds the match info.
  unsigned char *match;             // Substring which matched the regex.

  if ( argc != 2 )
  {
    printf( "Usage: cat file.txt | %s <regex>\n", argv[0] );
    return 0;
  }

  // Parse the regex string into the dre2 object.
  re = dre2_parse( ( unsigned char * )argv[1], DRE2_GREEDY );

  // Make sure parsing was successful.
  if ( re == NULL )
  {
    printf( "Failed to parse!\n" );
    return 0;
  }

  // Allocate some memory for input strings from stdin and matched substring.
  buf = ( unsigned char * )calloc( 0x10000, 1 );
  match = ( unsigned char * )calloc( 0x10000, 1 );

  // Check if input strings match the regex.
  while ( fgets( buf, 0x10000 - 1, stdin ) )
  {
    result = dre2_match( re, buf );
    if ( result.matched )
    {
      dre2_matched_substring( buf, &result, &match );
      printf( "%s\n", match );
    }
  }

  // Cleanup memory.
  cleanup_dre2( re );
  free( buf ); buf = NULL;
  free( match ); match = NULL;
  return 0;
}
