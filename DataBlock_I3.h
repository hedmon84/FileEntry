#pragma once
#ifndef INDEX_BLOCK_THIRD_LEVEL_H
#define INDEX_BLOCK_THIRD_LEVEL_H

#include "DataBlock_I2.h"

struct IndexBlockThirdLevel
{
	unsigned int indexBlockSecondLevel[INDEX_BLOCKS_THIRD_LEVEL];
};

#endif