#include "quotes.h"

static char const *const QUOTES[] = {
#include "quotes.txt" /* If using Arudino on winodws, you'll need to enter full path */
};

int const NUM_QUOTES = sizeof(QUOTES) / sizeof(char const *);
