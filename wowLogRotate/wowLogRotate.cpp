#include <iostream>
#include <string>
#include <filesystem>

#include "RegOps.h"
#include "LogRotate.h"

using namespace std;

int main()
{
	wstring wowInstallPath = getWowLocation();
	if (wowInstallPath.empty()) {
		cout << "Unable to read WoW installation path from Registry." << endl;
		return 1;
	}

	wcout << L"WoW location: " << wowInstallPath << endl;

	Rotator r(L"WoWCombatLog", L".txt", wowInstallPath + L"\\Logs");
	r.doRotate();

	/*	cout << "Press any key to exit" << endl;
	(void)getchar();*/

	return 0;
}
