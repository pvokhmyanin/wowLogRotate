#include <iostream>
#include <string>
#include "RegOps.h"

using namespace std;

int main()
{
	string wowInstallPath = getWowLocation();
	if (wowInstallPath == "") {
		cout << "Unable to read WoW installation path from Registry." << endl;
		return 1;
	}

    cout << "WoW location: " << getWowLocation();
	return 0;
}
