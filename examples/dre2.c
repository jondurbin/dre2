#include <dre2.h>

int main( int argc, char *argv[] )
{
  struct dre2 *re;                  // Pointer to regex digraph object.
  unsigned char *buf;               // Buffer to hold input strings.
  unsigned char *match;             // Matched substring.
  struct dre2_match_value result;   // Structure that holds the match info.
  int i;                            // Iterator.
  int options;                      // RE options.
  int re_arg;                       // Argument corresponding to regex.

  // Parse command line options.
  options = 0;
  re_arg = 0;
  for ( i = 1; i < argc; i++ )
  {
    if ( strcmp( argv[i], "-i" ) == 0 )
    {
      options = options | DRE2_NO_CASE;
    } else if ( strcmp( argv[i], "-o" ) == 0 )
    {
      options = options | DRE2_GREEDY;
    } else if ( strcmp( argv[i], "--help" ) == 0 )
    {
      re_arg = 0;
      break;
    } else
    {
      if ( re_arg != 0 )
      {
        printf( "Unknown option: %s\n", argv[1] );
        re_arg = 0;
        break;
      } else
      {
        re_arg = i;
      }
    }
  }

  if ( re_arg == 0 )
  {
    printf( "Usage: cat file.txt | %s <options> <regex>\n", argv[0] );
    printf( "     -i: Case insensitive matching.\n" );
    printf( "     -o: Display the matched substring.\n" );
    printf( " --help: Display this message\n" );
    return 0;
  }

  // Parse the regex string into the dre2 object.
  re = dre2_parse( ( unsigned char * )argv[1], options );

  // Make sure parsing was successful.
  if ( re == NULL )
  {
    printf( "Failed to parse the regular expression!\n" );
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
      if ( options & DRE2_GREEDY )
      {
        dre2_matched_substring( buf, &result, &match );
        printf( "%s\n", match );
      } else
      {
        printf( "%s", buf );
      }
    }
  }

  // Cleanup memory.
  cleanup_dre2( re );
  free( buf ); buf = NULL;
  return 0;
}
