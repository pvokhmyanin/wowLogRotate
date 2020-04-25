#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include <ctime>
#include <set>
#include <sstream>

#include "LogRotateConfig.h"

using namespace std;


class Rotator {
public:
	Rotator(const wstring& filename, const wstring& postfix, const wstring& workDir, const config& cfg) :
			m_filename(filename), m_postfix(postfix), m_cfg(cfg)
	{
		m_workDir = fs::path(workDir);
	}

	void doRotate() {
		rotateTopFile();
		rotateFiles();
	};

private:
	wstring m_filename;
	wstring m_postfix;
	fs::path m_workDir;
	config m_cfg;

	// Cook new name for the rotated file
	// We always want to save yesterday's log with yesterday's date,
	// hence we are going to extract 1 day from the current date
	const wstring generateRotatedName(bool shortName) {
		time_t currentTime = time(0) - (24 * 60 * 60);
		tm now;
		localtime_s(&now, &currentTime);
		wstringstream ret;
		ret << setprecision(4) << setfill(L'0');
		// Sample filename:
		// WoWCombatLog-split-2020-03-02T18-03-39.983Z.txt
		ret << m_filename << L"-split-" << setw(4) << now.tm_year + 1900
			<< L'-' << setw(2) << now.tm_mon + 1 << L'-' << setw(2) << now.tm_mday << L'T';
		// We use shortname to see if today's log already exists regardless of HH:MM:SS
		if (!shortName)
			ret << setw(2) << now.tm_hour << L'-' << setw(2) << now.tm_min << L'-' << setw(2) << now.tm_sec
			<< L".000Z" << m_postfix;
		
		return ret.str();
	}
	// Rename current file
	void rotateTopFile() {
		// Check if file exists first
		fs::path fileToRotate = m_workDir / fs::path(m_filename + m_postfix);
		if (!exists(fileToRotate))
		{
			return;
		}

		// Check if file is 0-sized, we wont rotate empty files
		fs::directory_entry srcFile(fileToRotate);
		if (srcFile.file_size() == 0) {
			return;
		}

		// Check if target file already exists
		fs::path targetFile = m_workDir / fs::path(generateRotatedName(true));
		try {
			fs::directory_iterator dir(m_workDir);
			for (const auto& e : dir) {
				fs::path p = e.path();
				wstring fn = p.filename();
				if (fn.rfind(targetFile, 0))
					return;
			}
		}
		catch (const fs::filesystem_error& fse) {
			cout << "Directory " << m_workDir << " does not exist" << endl;
			cout << fse.what() << endl;
			exit(1);
		}
		catch (const exception& exc) {
			cout << "Uncaught exception!" << endl;
			cout << exc.what() << endl;
			exit(2);
		}

		error_code ec;
		targetFile = m_workDir / fs::path(generateRotatedName(false));
		rename(fileToRotate, targetFile, ec);
		if (ec)
		{
			wcout << L"Unable to rename " << fileToRotate << L" to " << targetFile << endl;
			cout << "ec: " << ec.message() << " (" << ec.value() << ")" << endl;
			exit(1);
		}
	}
	// Rotate files
	void rotateFiles() {
		set<fs::path> fileHeap;
		try {
			fs::directory_iterator dir(m_workDir);
			for (const auto& e : dir) {
				fs::path p = e.path();
				wstring fn = p.filename();
				if (fn.find(m_filename) == 0) {
					fileHeap.insert(p);
				}
			}
		}
		catch (const fs::filesystem_error& fse) {
			cout << "Directory " << m_workDir << " does not exist" << endl;
			cout << fse.what() << endl;
			exit(1);
		}
		catch (const exception& exc) {
			cout << "Uncaught exception!" << endl;
			cout << exc.what() << endl;
			exit(2);
		}

		if (fileHeap.empty()) {
			cout << "Nothing to do, couldn't find logs to rotate" << endl;
			return;
		}

		set<fs::path>::reverse_iterator rIter;
		uintmax_t heapSize = 0;
		unsigned int fileCount = 0;
		bool startRemoving = false;

		cout << "Keep " << m_cfg.keepMaxFiles << " files or " << m_cfg.keepMaxMb << " Mb" << endl;
		for (rIter = fileHeap.rbegin(); rIter != fileHeap.rend(); rIter++) {
			// First iterate over files we're going to keep,
			// iterating over reverse set should give us newest files first.
			fs::directory_entry de(*rIter);
			heapSize += de.file_size();
			fileCount++;

			
			if (!startRemoving) {
				if (fileCount > m_cfg.keepMaxFiles or
					(heapSize >> 20) > (m_cfg.keepMaxMb )) {
					startRemoving = true;
					cout << "Limit exceeded, any remaining files are going to be removed." << endl;
				}
				else {
					cout << "[" << setw(3) << fileCount << " : " << setw(4) << (heapSize >> 20) 
						<< "Mb] Keep   " << *rIter << endl;
				}
			}
			// Now we can start removing extra files.
			// We do second "if" instead of "else" to process
			// the file which triggered removal.
			if (startRemoving) {
				cout << "[" << setw(3) << fileCount << " : " << setw(4) << (heapSize >> 20) 
					<< "Mb] Remove " << *rIter << endl;
				fs::remove(*rIter);
			}
		}
		if (!startRemoving) {
			cout << "Nothing to rotate, we're fine! "
				<< "(" << fileCount << "/" << m_cfg.keepMaxFiles << ") files; "
				<< "(" << (heapSize >> 20) << "/" << m_cfg.keepMaxMb << ") Mb"
				<< endl;
		}
	}
};