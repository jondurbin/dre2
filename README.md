#dre2 - An extremely fast, lightweight regex engine written in c.

## Basics

### Generating the dre2 object from a regular expresion string.
```c
  struct dre2 *re;
  re = dre2_parse( regex_string, 0 );
  if ( re == NULL )
  {
    // Parse failure.
    printf( "Failed to parse!\n" );
  }
```

### Matching an input string with the dre2 object:
```c
  struct dre2_match_value result;
  result = dre2_match( re, input_string );
  if ( result.matched )
  {
    // Match successful.
    printf( "Match.\n" );
  }
```

### Extracting the matching substring.
```c
    unsigned char *match;
    result = dre2_match( re, buf );
    if ( result.matched )
    {
      match = dre2_matched_substring( buf, &result );
      printf( "%s\n", match );
      if ( match != NULL )
      {
        free( match ); match = NULL;
      }
    }
```

### Cleanup:
```c
  cleanup_dre2( re );
```

### Full example:
```c
#include "dre2.h"

int
main( int argc, char *argv[] )
{
  struct dre2 *re;                  // Pointer to regex digraph object.
  char *buf;                        // Buffer to hold input strings.
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

  // Allocate some memory for input strings from stdin.
  buf = ( unsigned char * )calloc( 0x10000, 1 );

  // Check if input strings match the regex.
  while ( fgets( buf, 0x10000 - 1, stdin ) )
  {
    result = dre2_match( re, buf );
    if ( result.matched )
    {
      match = dre2_matched_substring( buf, &result );
      printf( "%s\n", match );
      if ( match != NULL )
      {
        free( match ); match = NULL;
      }
    }
  }

  // Cleanup memory.
  cleanup_dre2( re );
  free( buf ); buf = NULL;
  return 0;
}
```

## DRE2 options:

### Full match - Entire string must match the regex:
```c
  re = dre2_parse( regex_string, DRE2_FULL_MATCH );
```

### Case-insensitive regex:
```c
  re = dre2_parse( regex_string, DRE2_NO_CASE );
```

### Greedy mode:
```c
  re = dre2_parse( regex_string, DRE2_GREEDY );
```

### Combinations:
#### Greedy + ignore case:
```c
  re = dre2_parse( regex_string, DRE2_GREEDY | DRE2_NO_CASE );
```

#### Full match + ignore case:
```c
  re = dre2_parse( regex_string, DRE2_NO_CASE | DRE2_FULL_MATCH );
```
