
#include <fstream>
#include <iostream>
#include <string.h>
#include <cstring>
using namespace std;
#include "file_system.h"
#pragma warning(disable:4996)



int main()
{
	char code[30] = "", *command, *parameter = nullptr;

	FileSystem *CreateFile = new FileSystem();
	int num = 0;
	


	cout << "File system " << endl;
	cout << "Cantidad de bloque de  Datos que desea : ";
	cin >> num;
	while (strcmp(code, "exit") != 0)
	{
		cout << "C/";
		cin.getline(code, 30);

		if (strlen(code) == 0)
		{
			continue;
		}

		command = strtok(code, " ");
		parameter = strtok(nullptr, " ");
		
		if (strcmp(command, "disk") == 0)
		{
			CreateFile->createDisk(parameter, num);
		}
		else if (strcmp(command, "mkdir") == 0)
		{
			CreateFile->makeDirectory(parameter);
		}
		else if (strcmp(command, "ls") == 0)
		{
			CreateFile->list();
		}
		else if (strcmp(command, "cd") == 0 && strcmp(parameter, "..") == 0)
		{
			CreateFile->changeToPreviousDirectory();
		}
		else if (strcmp(command, "cd") == 0)
		{
			CreateFile->changeDirectory(parameter);
		}
		else if (strcmp(command, "rm") == 0)
		{
			CreateFile->DeleteNode(parameter);
		}
		else if (strcmp(command, "clear") == 0)
		{

			cout << "File system " << endl;
		
		}
		else if (strcmp(command, "exit") == 0)
		{
			break;
		}
	}

	return 0;
}

