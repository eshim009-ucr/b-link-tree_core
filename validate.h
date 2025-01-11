#ifndef VALIDATE_H
#define VALIDATE_H

#include "tree.h"
#include <stdio.h>

//! @return `true` for passing, `false` for failing
bool validate(Tree const *tree, FILE *stream);

#endif
