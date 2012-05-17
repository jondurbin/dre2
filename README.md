#dre2 - An extremely fast, lightweight regex engine written in c.

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
### Combining options:
#### Entire string must match + ignore case.
```c
  re = dre2_parse( regex_string, DRE2_FULL_MATCH | DRE2_NO_CASE );
```

#### Submatch mode + ignore case + thread safe:
```c
  re = dre2_parse( regex_string, DRE2_SUBMATCH | DRE2_NO_CASE | DRE2_THREAD_SAFE );
```

## Functions:
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
### Cleanup - Frees memory used by the dre2 object:
```c
  cleanup_dre2( re );
```

### For full examples, see dre2.c and submatch.c.
