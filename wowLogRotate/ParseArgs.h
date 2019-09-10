#pragma once
#include <map>

#include "Defines.h"

struct argument {
	unsigned int flag;
	char shortName[3];
	char longName[51];
	string description;
} argsArray[argsArraySize] =
{
	{OPTS_HELP ,		"-h", "--help",			"display this massively useful help"},
	{OPTS_UNATTENDED ,	"-u", "--unattended",	"unattended mode"}
};

void printHelp() {
	cout << "wowLogRotate.exe - utility to retent(rotate) World of Warcraft combat logs." << endl << endl;
	cout << "Available arguments:" << endl;
	for (int i = argsArraySize - 1; i >= 0; i--) {
		cout << "\t" << argsArray[i].shortName << ", " << argsArray[i].longName << endl;
		cout << "\t\t" << argsArray[i].description << endl << endl;
	}
}

int parseArgs(int argc, char** argv) {
	int opts = 0;

	for (int i = 1; i < argc; i++) {
		int found = 0;
		for (int j = 0; j < argsArraySize; j++) {
			if (strcmp(argv[i], argsArray[j].shortName) == 0 ||
				strcmp(argv[i], argsArray[j].longName) == 0) {
				opts |= argsArray[j].flag;
				found = 1;
				break;
			}
		}

		if (!found) {
			cout << "Error: Unknown argument " << argv[i] << endl;
			exit(255);
		}
	}

	if (opts & OPTS_HELP)
	{
		printHelp();
		exit(0);
	}

	return opts;
}