#pragma once
#ifndef INDEX_BLOCK_FIRST_LEVEL_H
#define INDEX_BLOCK_FIRST_LEVEL_H

#include "data_block.h"
#include "constants.h"

struct IndexBlockFirstLevel
{
	unsigned int dataBlockLocation[INDEX_BLOCKS_FIRST_LEVEL];
};

#endif