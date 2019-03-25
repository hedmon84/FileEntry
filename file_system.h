

#include "data_block.h"
#include "data_file.h"
#include "node_entry.h"
#include "super_block.h"
#include "Setbits.h"

class FileSystem
{
private:
	DataFile *dataFile;
	SuperBlock *superBlock;
	int currentDirectoryInByte;
	bitsets *setupbits;;
	long nextFreepos();
	void DeleteNode(NodeEntry *, int );

public:
	FileSystem();
	~FileSystem();

	void createDisk(char *, const int );
	void makeDirectory(char *);
	void changeDirectory(char *name);
	void changeToPreviousDirectory();
	void DeleteNode(char *name);
	void list();
	void importFile(char *name);
	char *exportFile(char *name);
};

