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
    match = ( unsigned char * )malloc( strlen( buf ) * sizeof( unsigned char ) );
    result = dre2_match( re, buf );
    if ( result.matched )
    {
      dre2_matched_substring( buf, &result, &match );
      printf( "%s\n", match );
    }
```

### Extracting the submatches.
```c
  // Initialize submatch string array.
  submatches = ( unsigned char ** )calloc( re->original->group_count, sizeof( unsigned char * ) );
  for ( i = 1; i < re->original->group_count; i++ )
    submatches[i - 1] = ( unsigned char * )calloc( 0x10000, sizeof( unsigned char ) );

  // Check if input string matches the regex.
  result = dre2_match( re, input );
  if ( result.matched )
  {
    // First, get the matched substring.
    dre2_matched_substring( input, &result, &match );
    printf( "Match: %s\n", match );

    // We only have submatch info if there is more than 1 group.
    if ( re->original->group_count > 1 )
    {
      // Run the backtracking match to set submatched strings.
      dre2_backtrack_match( re, match, &submatches );
      for ( i = 1; i < re->original->group_count; i++ )
        printf( "Submatch %d: '%s'\n", i - 1, submatches[i - 1] );
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

int main( int argc, char *argv[] )
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
  re = dre2_parse( ( unsigned char * )argv[1], 0 );

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
```

## DRE2 options:

### Thread-safe - MUST be used in any multi-threaded application.
```c
  re = dre2_parse( regex_string, DRE2_THREAD_SAFE );
```

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

### Submatch mode - Needed if you plan on using the submatches for anything.
```c
  re = dre2_parse( regex_string, DRE2_SUBMATCH );
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
