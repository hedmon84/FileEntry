
#pragma warning(disable:4996)
#pragma warning(disable:4703)
#include <string.h>
#include "file_system.h"
#include "constants.h"
#include <iostream>
#include <fstream>
using namespace std;




FileSystem::FileSystem()
{
	this->superBlock = new SuperBlock();
	this->setupbits = new bitsets();
}

FileSystem::~FileSystem()
{
	this->dataFile->close();
	delete this->superBlock;
	delete this->dataFile;
	delete this->setupbits;
}

void FileSystem::createDisk(char *path, const int nodeEntriesQuantity)
{
	this->dataFile = new DataFile(path);
	this->dataFile->open(std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
	fstream diskfile(path, ios::out | ios::app | ios::binary);

	if (!diskfile)
	{
		cout << " Error al intentar abrir archivo ArbolAVL.dat";
	}

	const int dataBlocksQuantity = nodeEntriesQuantity * TOTAL_DATA_BLOCKS_IN_NODE_ENTRY;

	this->setupbits->dataBlockVector = new char[dataBlocksQuantity / 8];
	this->setupbits->indexBlockFirstLevelVector = new char[nodeEntriesQuantity / 8];
	this->setupbits->indexBlockSecondLevelVector = new char[nodeEntriesQuantity / 8];
	this->setupbits->indexBlockThirdLevelVector = new char[nodeEntriesQuantity / 8];

	this->currentDirectoryInByte = sizeof(SuperBlock) + sizeof(this->setupbits);

	this->superBlock->firstNodeEntry = sizeof(SuperBlock) + sizeof(this->setupbits);
	this->superBlock->nodeEntriesQuantity = nodeEntriesQuantity;
	this->superBlock->firstDataBlock = this->currentDirectoryInByte;

	this->dataFile->write(reinterpret_cast<char *>(this->superBlock),
		sizeof(SuperBlock));

	this->dataFile->write(reinterpret_cast<char *>(this->setupbits), sizeof(this->setupbits));

	NodeEntry *nodeEntry = new NodeEntry();
	for (size_t i = 0; i < nodeEntriesQuantity; i++)
	{
		nodeEntry->isFree = true;
		nodeEntry->lastChild = nodeEntry->firstChild = -1;
		nodeEntry->rightBrother = -1;
		strcpy(nodeEntry->name, (char *)"00000");

		this->dataFile->write(reinterpret_cast<char *>(nodeEntry),
			sizeof(NodeEntry));
	}

	DataBlock *dataBlock = new DataBlock();
	for (size_t i = 0; i < dataBlocksQuantity; i++)
	{
		this->dataFile->write(reinterpret_cast<char *>(dataBlock),
			sizeof(DataBlock));
	}

	IndexBlockFirstLevel *indexBlockFirstLevel = new IndexBlockFirstLevel();
	for (size_t i = 0; i < nodeEntriesQuantity; i++)
	{
		this->dataFile->write(reinterpret_cast<char *>(indexBlockFirstLevel),
			sizeof(IndexBlockFirstLevel));
	}

	IndexBlockSecondLevel *indexBlockSecondLevel = new IndexBlockSecondLevel();
	for (size_t i = 0; i < nodeEntriesQuantity; i++)
	{
		this->dataFile->write(reinterpret_cast<char *>(indexBlockSecondLevel),
			sizeof(IndexBlockFirstLevel));
	}

	IndexBlockThirdLevel *indexBlockThirdLevel = new IndexBlockThirdLevel();
	for (size_t i = 0; i < nodeEntriesQuantity; i++)
	{
		this->dataFile->write(reinterpret_cast<char *>(indexBlockThirdLevel),
			sizeof(IndexBlockThirdLevel));
	}

	this->dataFile->close();

	this->dataFile->open();
	NodeEntry *rootDirectory = new NodeEntry();
	rootDirectory->firstChild = rootDirectory->rightBrother = rootDirectory->lastChild = -1;
	strcpy(rootDirectory->name, (char *)"/");
	rootDirectory->isFree = false;
	this->dataFile->write(reinterpret_cast<char *>(rootDirectory), this->currentDirectoryInByte, sizeof(NodeEntry));

	delete nodeEntry;
	delete dataBlock;
	delete indexBlockFirstLevel;
	delete indexBlockSecondLevel;
	delete indexBlockThirdLevel;
	delete rootDirectory;
	
}


void FileSystem::makeDirectory(char *name)

{


	NodeEntry *nodeEntry;
	nodeEntry = reinterpret_cast<NodeEntry *>(
		this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));

	if (nodeEntry->isFree)
	{

		strcpy(nodeEntry->name, name);
		nodeEntry->type = 'd';
		nodeEntry->isFree = false;
		nodeEntry->rightBrother = -1;
		this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
			sizeof(NodeEntry));
	}
	else if (!nodeEntry->isFree && nodeEntry->firstChild == -1)
	{
		long newNodeEntryPosition = nextFreepos();

		if (newNodeEntryPosition == -1)
		{
			std::cout << "No space" << std::endl;
			return;
		}

		NodeEntry *newNodeEntry = new NodeEntry();
		strcpy(newNodeEntry->name, name);
		newNodeEntry->type = 'd';
		newNodeEntry->isFree = false;
		newNodeEntry->firstChild = newNodeEntry->lastChild = newNodeEntry->rightBrother = -1;
		newNodeEntry->parent = this->currentDirectoryInByte;
		this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), newNodeEntryPosition,
			sizeof(NodeEntry));

		nodeEntry->firstChild = nodeEntry->lastChild = newNodeEntryPosition;

		this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
			sizeof(NodeEntry));
	}
	else if (!nodeEntry->isFree && nodeEntry->firstChild != -1)
	{
		long newNodeEntryPosition = nextFreepos();

		if (newNodeEntryPosition == -1)
		{
			std::cout << "No space" << std::endl;
			return;
		}

		NodeEntry *newNodeEntry = new NodeEntry();
		strcpy(newNodeEntry->name, name);
		newNodeEntry->type = 'd';
		newNodeEntry->isFree = false;
		newNodeEntry->firstChild = newNodeEntry->lastChild = newNodeEntry->rightBrother = -1;
		newNodeEntry->parent = this->currentDirectoryInByte;
		this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), newNodeEntryPosition,
			sizeof(NodeEntry));

		int lastChildPreviousPosition = nodeEntry->lastChild;
		NodeEntry *lastChild = reinterpret_cast<NodeEntry *>(this->dataFile->read(nodeEntry->lastChild, sizeof(NodeEntry)));
		lastChild->rightBrother = newNodeEntryPosition;
		nodeEntry->lastChild = newNodeEntryPosition;

		this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
			sizeof(NodeEntry));

		this->dataFile->write(reinterpret_cast<char *>(lastChild), lastChildPreviousPosition, sizeof(NodeEntry));
	}

	delete nodeEntry;
}

void FileSystem::changeDirectory(char *name)
{
	NodeEntry *currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));
	int position = currentDirectory->firstChild;
	while (position != -1)
	{
		NodeEntry *childNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(position, sizeof(NodeEntry)));
		if (strcmp(childNodeEntry->name, name) == 0)
		{
			this->currentDirectoryInByte = this->dataFile->readPosition() - sizeof(NodeEntry);
			break;
		}
		position = childNodeEntry->rightBrother;
	}

	delete currentDirectory;
}

void FileSystem::changeToPreviousDirectory()
{
	NodeEntry *currentDirectory;
	currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));
	this->currentDirectoryInByte = currentDirectory->parent;

	delete currentDirectory;
}

void FileSystem::list()
{
	NodeEntry *currentDirectory;
	currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));

	if (currentDirectory->isFree || currentDirectory->firstChild == -1)
	{
		std::cout << "Empty directory!" << std::endl;
		return;
	}

	currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentDirectory->firstChild, sizeof(NodeEntry)));
	std::cout << currentDirectory->name << std::endl;

	while (currentDirectory->rightBrother != -1)
	{
		currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentDirectory->rightBrother, sizeof(NodeEntry)));
		std::cout << currentDirectory->name << std::endl;
	}

	delete currentDirectory;
}

void FileSystem::importFile(char * name)
{
	/*



	cout << "Nombre del documento:";
	cin >> name;
	bool tipo = false;
	double size = 0;
	int pos = 0;

	//fstream Disco(discName + ".dat", ios::in | ios::out | ios::binary);
	cout << "posicion actual es: " << posActual << "\n";
	ifstream lectura(nombre, ios::in | ios::binary);
	fstream disco("C:\Users\Erick Nisbeth\source\repos\Estructurall\FileSystem2.0" + n, ios::in | ios::out | ios::binary);
	if (lectura) {
		lectura.seekg(0, ios::end);
		size = lectura.tellg();

		CrearDirectorio(array, n, bmap, md, nombre, tipo, size);
		for (int j = 0; j < tamano; j++) {
			if (array[j].file_name == nombre) {
				pos = j;
				cout << "el nombre del actual J ES:" << array[j].file_name << "\n";
				getch();
				break;
			}
			if (j == tamano)
				cout << "No se encontro archivo a exportar\n";
		}
		if (size <= (cantidad_bloques * 4096)) {


			Datablock_direct buffer;
			Datablock_direct bufferLlenar;

			double bloqueocu = ceil(array[pos].size / 4096);
			int cont1 = 0, cont2 = 0, cont3 = 0;
			int contGeneral = 0;
			int contBloque = 0;
			int contL1aux = -1;
			int contL2aux = 0, contL2aux2 = -1;
			int posaux = 0;
			int posaux2 = 0;
			lectura.seekg(0, ios::beg);

			cout << "el nombre de este import es: " << array[pos].file_name << "\n";
			cout << "el peso de este import es: " << array[pos].size << "\n";
			cout << "tamano: " << tamano << "\n";
			/*Posiciones en Bytes en el archivo
			unsigned int posBytes = ((sizeof(superbloque) + sizeof(bitmap) + (sizeof(file_entry)*tamano)) + 1);
			unsigned int posBytesL1 = (posBytes + (((33308 * 4096)*tamano))) + 1;
			
			if (pos > 0) {
				posaux = (pos*(33308 * 4096));
				posBytes = posBytes + posaux;
				posaux2 = (pos*(2081 * sizeof(Datablock_I1)));
				posBytesL1 = posBytesL1 + posaux2;
			}

			int posBitmap = pos * 33308;
			disco.seekp(posBytes, ios::beg);
			while (!lectura.eof()) {

				lectura.read(reinterpret_cast<char *>(&buffer), sizeof(Datablock_direct));
				memcpy(&bufferLlenar, &buffer, 4096);
				if (contGeneral < 12) {
					array[pos].DB_directs[contBloque] = posBytes;
					disco.write(reinterpret_cast<char*>(&bufferLlenar), 4096);
					cout << "posicion bloque: " << contBloque << ": " << array[pos].DB_directs[contBloque] << endl;
					bmap->setOn(bmap->DBlock, posBitmap);
					posBitmap++;
					getch();
					//bd.bloque[contBloque] = buffer;

					contBloque++;
					posBytes += sizeof(Datablock_direct);
				}
				else if (contGeneral >= 12 && contGeneral < 28) {
					//d1.p[cont1] = buffer;
					if (contGeneral == 12)array[pos].DB_indirects[0] = posBytesL1;
					d1.pointers[cont1] = posBytes;
					disco.seekg(d1.pointers[cont1], ios::beg);
					disco.write(reinterpret_cast<char*>(&bufferLlenar), 4096);
					cout << "posicion bloque: " << contBloque << ": " << d1.pointers[cont1] << endl;
					bmap->setOn(bmap->DBlock, posBitmap);
					posBitmap++;
					getch();
					cont1++;
					disco.seekp(array[pos].DB_indirects[0], ios::beg);
					disco.write(reinterpret_cast<char *>(&d1), sizeof(Datablock_I1));
					if (cont1 == 15 || contGeneral == bloqueocu) {
						bmap->setOn(bmap->BlockLvl1, posBitmap);
					}
					//posBytesL1+=sizeof(Datablock_I1);
					contBloque++;
					posBytes += sizeof(Datablock_direct);
				}
				else if (contGeneral >= 28 && contGeneral < 540) {
					contL1aux++;
					//d2.p[cont2].p[contL1aux] = buffer;
					if (contL1aux == 15 || contGeneral == bloqueocu) {
						cont2++;
						contL1aux = -1;
					}
				}
				else if (contGeneral >= 540 && contGeneral < 33308) {
					contL2aux2++;
					//d3.p[cont3].p[contL2aux].p[contL2aux2]=buffer;
					if (contL2aux2 == 15) {
						contL2aux2 = -1;
						contL2aux++;
						if (contL2aux == 32) {
							contL2aux = 0;
							cont3++;
						}
					}
				}

				contGeneral++;
			}
		}

	}
	else {
		cout << "No se pudo IMPORTAR..\n";
	}

	disco.close();
	lectura.close();

	*/
}

char * FileSystem::exportFile(char * name)
{

	/*
	int pos = 0;

	cout << "Nombre del documento:";
	cin >> name;
	cout << "Nuevo Nombre:";
	cin >> name;

	ofstream escritura(n_nombre, ios::out | ios::binary);



	escritura.seekp(0, ios::end);
	int cont1 = 0, cont2 = 0, cont3 = 0;
	int cont2aux = -1;
	int cont3aux = -1;
	int cont2auxiliar = 0;


	fstream disco("C:/Users/WilliamPC/CLionProjects/ProyectoIParcial_ED2/cmake-build-debug/discos/" + n, ios::in | ios::out | ios::binary);
	Datablock_direct buffer;

	for (int j = 0; j < tamano; j++) {
		if (array[j].file_name == tmp) {
			pos = j;
			break;
		}
		if (j == tamano)
			cout << "No se encontro archivo a exportar\n";
	}
	double bloqueocu = ceil(array[pos].size / 4096);
	for (int i = 0; i < bloqueocu; i++) {
		if (i < 12) {
			cout << "Bloque " << i << " " << array[pos].DB_directs[i] << endl;

		}
	}
	cout << "Posicion del arreglo: " << pos << "\n";

	double faltante = array[pos].size;
	cout << "bloques " << bloqueocu << endl;
	getch();
	cout << "falta: " << faltante << "\n";
	double faux = 0;
	for (int i = 0; i < bloqueocu; i++) {
		if (i < bloqueocu) {
			faux = 4096;
		}
		if (faltante < 4096) {
			faux = faltante;
		}
		faltante = faltante - 4096;
		cout << "Cuanto falta: " << faltante << "\n";
		getch();
		if (i < 12) {
			disco.seekg(array[pos].DB_directs[i], ios::beg);
			disco.read(reinterpret_cast<char*>(&buffer), 4096);
			escritura.write(reinterpret_cast<char *>(&buffer), faux);
		}
		if (i >= 12 && i < 28) {
		
			disco.seekg(array[pos].DB_indirects[0], ios::beg);
			disco.read(reinterpret_cast<char *> (&d1), sizeof(Datablock_I1));
			disco.seekg(d1.pointers[cont1], ios::beg);
			disco.read(reinterpret_cast<char *> (&buffer), 4096);
			escritura.write(reinterpret_cast<char *>(&buffer), faux);
			cont1++;
		}
		if (i >= 28 && i < 540) {
			cont2aux++;
		
			if (cont2aux == 15 || i == bloqueocu) {
				cont2++;
				cont2aux = -1;
			}
		}
		if (i >= 540 && i < 33308) {
			cont3aux++;
			
			if (cont3aux == 15) {
				cont3aux = -1;
				cont2auxiliar++;
				if (cont2auxiliar == 32) {
					cont2auxiliar = 0;
					cont3++;
				}
			}
		}
	}

	getch();

	escritura.close();
	*/
}


long FileSystem::nextFreepos()
{
	unsigned int initialPosition = this->superBlock->firstNodeEntry;

	for (size_t i = 0; i < this->superBlock->nodeEntriesQuantity; i++)
	{
		NodeEntry *currentNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(initialPosition, sizeof(NodeEntry)));
		if (currentNodeEntry->isFree && this->dataFile->readPosition() != this->currentDirectoryInByte)
		{
			return this->dataFile->readPosition() - sizeof(NodeEntry);
		}
		initialPosition += sizeof(NodeEntry);
	}

	std::cout << "No space in disk" << std::endl;
	return -1;
}

void FileSystem::DeleteNode(char *name)
{
	unsigned int initialPosition = this->superBlock->firstNodeEntry;
	NodeEntry *currentNodeEntry;

	for (size_t i = 0; i < this->superBlock->nodeEntriesQuantity; i++)
	{
		currentNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(initialPosition, sizeof(NodeEntry)));
		if (strcmp(currentNodeEntry->name, name) == 0)
		{
			break;
		}
		initialPosition += sizeof(NodeEntry);
	}

	if (strcmp(currentNodeEntry->name, name) != 0)
	{
		std::cout << "Not found!" << std::endl;
	}

	NodeEntry *parent = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentNodeEntry->parent, sizeof(NodeEntry)));

	int parentPosition = currentNodeEntry->parent;
	int brotherPosition = currentNodeEntry->rightBrother;
	int firstChildPosition = currentNodeEntry->firstChild;

	if (currentNodeEntry->firstChild != -1)
	{
		NodeEntry *child = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentNodeEntry->firstChild, sizeof(NodeEntry)));
		DeleteNode(child, currentNodeEntry->firstChild);
	}

	if (initialPosition == parent->firstChild)
	{
		parent->firstChild = brotherPosition;
		this->dataFile->write(reinterpret_cast<char *>(parent), parentPosition, sizeof(NodeEntry));
	}
	else if (initialPosition == parent->lastChild)
	{
		NodeEntry *nodeEntry;
		int position = parent->firstChild;
		int location = 0;
		do
		{
			nodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(position, sizeof(NodeEntry)));
			location = this->dataFile->readPosition() - sizeof(NodeEntry);
		} while (nodeEntry->rightBrother != initialPosition);

		nodeEntry->rightBrother = -1;

		NodeEntry *newNodeEntry = new NodeEntry();
		strcpy(newNodeEntry->name, (char *)"00000");
		newNodeEntry->isFree = true;
		newNodeEntry->firstChild = newNodeEntry->lastChild = newNodeEntry->rightBrother = -1;
		newNodeEntry->parent = -1;
		this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), initialPosition,
			sizeof(NodeEntry));

		this->dataFile->write(reinterpret_cast<char *>(nodeEntry), location,
			sizeof(NodeEntry));
	}
	else
	{
		NodeEntry *nodeEntry;
		int position = parent->firstChild;
		int location = 0;
		do
		{
			nodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(position, sizeof(NodeEntry)));
			position = nodeEntry->rightBrother;
			location = this->dataFile->readPosition() - sizeof(NodeEntry);
		} while (nodeEntry->rightBrother != initialPosition);

		nodeEntry->rightBrother = brotherPosition;

		NodeEntry *newNodeEntry = new NodeEntry();
		strcpy(newNodeEntry->name, (char *)"00000");
		newNodeEntry->isFree = true;
		newNodeEntry->firstChild = newNodeEntry->lastChild = newNodeEntry->rightBrother = -1;
		newNodeEntry->parent = -1;
		this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), initialPosition,
			sizeof(NodeEntry));

		this->dataFile->write(reinterpret_cast<char *>(nodeEntry), location,
			sizeof(NodeEntry));
	}
}

void FileSystem::DeleteNode(NodeEntry *nodeEntry, int position)
{
	if (position == -1)
	{
		return;
	}

	int childPosition = nodeEntry->firstChild;
	int brotherPosition = nodeEntry->rightBrother;

	NodeEntry *newNodeEntry = new NodeEntry();
	strcpy(newNodeEntry->name, (char *)"00000");
	newNodeEntry->isFree = true;
	newNodeEntry->firstChild = newNodeEntry->lastChild = newNodeEntry->rightBrother = -1;
	newNodeEntry->parent = -1;
	this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), position,
		sizeof(NodeEntry));

	if (brotherPosition != -1)
	{
		NodeEntry *brother = reinterpret_cast<NodeEntry *>(this->dataFile->read(brotherPosition, sizeof(NodeEntry)));
		DeleteNode(brother, brotherPosition);
	}

	if (childPosition != -1)
	{
		NodeEntry *child = reinterpret_cast<NodeEntry *>(this->dataFile->read(childPosition, sizeof(NodeEntry)));
		DeleteNode(child, childPosition);
	}
}