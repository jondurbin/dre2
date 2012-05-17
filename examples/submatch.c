#include "../src/dre2.h"

int main( int argc, char *argv[] )
{
  int i, j;                         // Iterators.
  struct dre2 *re;                  // Pointer to regex digraph object.
  unsigned char *buf;               // Buffer to hold input strings.
  struct dre2_match_value result;   // Structure that holds the match info.
  unsigned char *match;             // Substring which matched the regex.
  unsigned char **submatches;       // Submatch extraction strings.

  if ( argc != 2 )
  {
    printf( "Usage: cat file.txt | %s <regex>\n", argv[0] );
    return 0;
  }

  // Parse the regex string into the dre2 object.
  re = dre2_parse( ( unsigned char * )argv[1], DRE2_SUBMATCH );

  // Make sure parsing was successful.
  if ( re == NULL )
  {
    printf( "Failed to parse!\n" );
    return 0;
  }

  // Allocate some memory for input strings from stdin and matched substring.
  buf = ( unsigned char * )calloc( 0x10000, 1 );
  match = ( unsigned char * )calloc( 0x10000, 1 );
  submatches = ( unsigned char ** )calloc( re->original->group_count, sizeof( unsigned char * ) );

  for ( i = 1; i < re->original->group_count; i++ )
    submatches[i - 1] = ( unsigned char * )calloc( 0x10000, sizeof( unsigned char ) );

  // Check if input strings match the regex.
  while ( fgets( buf, 0x10000 - 1, stdin ) )
  {
    result = dre2_match( re, buf );
    if ( result.matched )
    {
      dre2_matched_substring( buf, &result, &match );
      printf( "Match: %s\n", match );
      if ( re->original->group_count > 1 )
      {
        dre2_backtrack_match( re, match, &submatches );
        for ( i = 1; i < re->original->group_count; i++ )
          printf( "Submatch %d: '%s'\n", i - 1, submatches[i - 1] );
      }
    }
  }

  // Cleanup memory.
  for ( i = 1; i < re->original->group_count; i++ )
  {
    if ( submatches[i - 1] != NULL )
    {
      free( submatches[i - 1] );
      submatches[i - 1] = NULL;
    }
  }
  free( submatches );
  submatches = NULL;
  cleanup_dre2( re );
  free( buf ); buf = NULL;
  free( match ); match = NULL;

  return 0;
}
