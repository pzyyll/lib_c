//
// Created by czllo on 2017/4/9.
//

#include "skiplist.h"

int skRandomLevel()
{
    int lvl = 1;
    while ((rand() < (SKIPLIST_P * RAND_MAX))
            && (lvl < MAXLEVEL))
        ++lvl;
    return lvl;
}