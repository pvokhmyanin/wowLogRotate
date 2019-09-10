#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include <ctime>
#include <set>
#include <sstream>
#include <map>

using namespace std;
namespace fs = std::filesystem;

typedef map<string, int> config;

class Rotator {
public:
	Rotator(const wstring& filename, const wstring& postfix, const wstring& workDir) :
			m_filename(filename), m_postfix(postfix)
	{
		m_workDir = fs::path(workDir);
		m_cfg =	{
			{"maxsizemb", 100},
			{"keepfiles", 15}
		};
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
	const wstring generateRotatedName() {
		time_t currentTime = time(0);
		tm now;
		localtime_s(&now, &currentTime);
		wstringstream ret;
		ret << setprecision(4) << setfill(L'0');
		ret << m_filename << L'-' << setw(4) << now.tm_year + 1900
			<< L'_' << setw(2) << now.tm_mon + 1
			<< L'_' << setw(2) << now.tm_mday
			<< m_postfix;

		return ret.str();
	}
	// Rename current file
	void rotateTopFile() {
		// Check if file exists first
		fs::path fileToRotate = m_workDir / fs::path(m_filename + m_postfix);
		if (!exists(fileToRotate))
		{
			cout << "File " << fileToRotate << " does not exist" << endl;
			return;
		}

		// Check if file is 0-sized, we wont rotate empty files
		fs::directory_entry srcFile(fileToRotate);
		if (srcFile.file_size() == 0) {
			cout << "File " << fileToRotate << " is 0 sized" << endl;
			return;
		}

		// Check if target file already exists
		fs::path targetFile = m_workDir / fs::path(generateRotatedName());
		if (exists(targetFile)) {
			cout << "File " << targetFile << " already exists" << endl;
			return;
		}

		try {
			rename(fileToRotate, targetFile);
		}
		catch (const exception& ex) {
			wcout << L"Unable to rename " << fileToRotate << L" to " << targetFile << endl;
			wcout << ex.what();
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
		int fileCount = 0;
		bool startRemoving = false;

		cout << "Keep " << m_cfg["keepfiles"] << " files or " << m_cfg["maxsizemb"] << " Mb" << endl;
		for (rIter = fileHeap.rbegin(); rIter != fileHeap.rend(); rIter++) {
			// First iterate over files we're going to keep,
			// iterating over reverse set should give us newest files first.
			if (!startRemoving) {
				fs::directory_entry de(*rIter);
				heapSize += de.file_size();
				fileCount++;
				if (fileCount > m_cfg["keepfiles"] or
					(heapSize >> 20) > (m_cfg["maxsizemb"] )) {
					startRemoving = true;
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
				<< "(" << fileCount << "/" << m_cfg["keepfiles"] << ") files; "
				<< "(" << (heapSize >> 20) << "/" << m_cfg["maxsizemb"] << ") Mb"
				<< endl;
		}
	}
};


