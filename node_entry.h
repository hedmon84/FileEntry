#pragma once
#ifndef NODE_ENTRY_H

#include "DataBlock_I1.h"
#include "DataBlock_I2.h"
#include "DataBlock_I3.h"

#include "constants.h"

struct NodeEntry
{
	char name[30];
	int size;
	char type;
	char date[30];

	int parent;
	int firstChild;
	int rightBrother;
	int lastChild;
	bool isFree;

	unsigned int datablocksLocations[NODE_ENTRIES_DATA_BLOCKS];
	unsigned int indexBlockFirstLevel;
	unsigned int indexBlockSecondLevel;
	unsigned int indexBlockThirdLevel;
};

#endif