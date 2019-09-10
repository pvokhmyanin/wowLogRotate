#include <iostream>
#include <string>
#include <clocale>

#include "RegOps.h"
#include "LogRotate.h"
#include "Defines.h"
#include "ParseArgs.h"


using namespace std;

int main(int argc, char** argv)
{
	// Set locale
	setlocale(LC_ALL, "English_United States.1251");

	// Parse arguments
	int opts = parseArgs(argc, argv);

	// Retrieve path to WoW from Windows Registry
	wstring wowInstallPath = getWowLocation();
	if (wowInstallPath.empty()) {
		cout << "Unable to read WoW installation path from Registry." << endl;
		return 1;
	}
	wcout << L"WoW location: " << wowInstallPath << endl;

	// Read config file
	config cfg = readConfig(L"wowLogRotate.cfg", opts);

	// Perform log rotation
	Rotator r(L"WoWCombatLog", L".txt", wowInstallPath + L"\\Logs", cfg);
	r.doRotate();

	return 0;
}
