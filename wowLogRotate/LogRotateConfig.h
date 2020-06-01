#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <windows.h>

#include "Defines.h"

using namespace std;
namespace fs = std::filesystem;

struct config {
	uintmax_t keepMaxMb = 2048;		// Default size to keep is 2Gb
	unsigned int keepMaxFiles = 14;	// Default amount of files to keep
									// is 2 weeks worth of logs
};

wstring getConfigPath() {
	WCHAR ownPth[MAX_PATH];
	GetModuleFileNameW(NULL, ownPth, MAX_PATH);
	wstring ret(ownPth);
	int pos = ret.rfind('\\');
	return ret.substr(0, pos + 1) + L"wowLogRotate.cfg";
}

ostream& operator<<(ostream& os, const config& cfg) {
	os << "KeepMaxMb " << cfg.keepMaxMb << endl;
	os << "KeepMaxFiles " << cfg.keepMaxFiles << endl;
	return os;
}

void writeConfig(const wstring& cfgName, const config& cfg) {
	ofstream cfgFile(cfgName);
	if (!cfgFile) {
		wcout << "Unable to write config file " << cfgName << endl;
		exit(3);
	}
	cfgFile << cfg;
	cfgFile.close();
}

void parseError(const string& line) {
	cout << "Invalid config! Couldn't parse line:" << endl << line << endl;
	exit(4);
}

config readConfig(const wstring& cfgName, const int opts) {
	config cfg;

	// If config file is missing - generate default config
	if (!exists(fs::path(cfgName))) {
		wcout << "Config file " << cfgName << " not found, generating default config..." << endl;
		writeConfig(cfgName, cfg);

		// Generating default config and running logrotation right away
		// could be too intrusive. Lets ask user if he wants to run it as is
		// or would rather cancel operation and edit config first.
		cout << "New config generated:" << endl
			<< cfg << endl;
		
		cout << "Do you want to run Log Rotation(y), or would rather edit config first(n)? (y/n) ";

		char c = '\0';
		// If launched in unatended mode - interrupt execution
		if (opts & OPTS_UNATTENDED) {
			exit(0);
		}

		while (c != 'y' and c != 'n') {
			c = getchar();
		}
		if (c == 'y')
			return cfg;
		else
			exit(0);
	}

	ifstream cfgFile(cfgName);

	if (!cfgFile) {
		wcout << L"Cannot open config file " << cfgName << endl;
		exit(2);
	}

	string buf;
	while (getline(cfgFile, buf)) {
		stringstream inss(buf);

		// Skip empty lines
		if (!inss)
			continue;

		string label;
		inss >> label;
		if (label == "KeepMaxFiles") {
			if (inss) {
				inss >> cfg.keepMaxFiles;
			}
			else {
				parseError(buf);
			}
		}
		else if (label == "KeepMaxMb") {
			if (inss) {
				inss >> cfg.keepMaxMb;
			}
			else {
				parseError(buf);
			}
		}
		else {
			parseError(buf);
		}
	}
	return cfg;
}