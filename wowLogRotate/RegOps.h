#pragma once
#include <Windows.h>
#include <winreg.h>
#include <tchar.h>
#include <string>
#include <map>

using namespace std;

const map<string, LPCWSTR> registryWowPathMap = {
	{"win10x64", L"SOFTWARE\\WOW6432Node\\Blizzard Entertainment\\World of Warcraft"},
	{"win10x32", L"SOFTWARE\\Blizzard Entertainment\\World of Warcraft"}
};


// Registry path would vary depending on distro and arch, I need to detect OS type
// and return proper path.
const LPCWSTR getWowRegistryPath() {
	// TODO: Detect os properly, populate map.
	// For the time being I'll assume win10 x64.
	return registryWowPathMap.at("win10x64");
}


int getRegistryKey(HKEY &hKey, const LPCWSTR& qvalue) {
	DWORD dwRet;

	dwRet = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		qvalue,
		NULL,
		KEY_QUERY_VALUE,
		&hKey);
	if (dwRet != ERROR_SUCCESS)
	{
		clog
			<< "Error: RegOpenKeyEx:"
			<< " dwRet=" << dwRet
			<< " GetLastError=" << GetLastError() << "\n";
		return -1;
	}
	return 0;
}

int getStringValue(const HKEY& hKey, const LPCWSTR& qvalue, string& retValue) {

	DWORD dwRet;
	const DWORD SIZE = 1024;
	TCHAR szValue[SIZE] = _T("");
	DWORD dwValue = SIZE;
	DWORD dwType = 0;
	dwRet = RegQueryValueEx(
		hKey,
		qvalue,
		NULL,
		&dwType,
		(LPBYTE)& szValue,
		&dwValue);
	if (dwRet != ERROR_SUCCESS)
	{
		clog
			<< "Error: RegQueryValueEx:"
			<< " dwRet=" << dwRet
			<< " GetLastError=" << GetLastError() << "\n";
		return -1;
	}
	if (dwType != REG_SZ)
	{
		clog << "Value not a string\n";
		return -1;
	}
	
	// Convert WCHAR to string via intermittent wstring
	wstring wst(szValue);
	retValue = string(wst.begin(), wst.end());
	return 0;
}

string getWowLocation() {
	HKEY hKey;
	string ret;
	const LPCWSTR qkey = getWowRegistryPath();
	const LPCWSTR qvalue = L"InstallPath";
	
	// Retrieve Registry Key handle
	if (getRegistryKey(hKey, qkey)) {
		return "";
	}

	// Read InstallPath value from the Registry Key
	if (getStringValue(hKey, qvalue, ret)) {
		ret = "";
	}

	RegCloseKey(hKey);
	return ret;
}
