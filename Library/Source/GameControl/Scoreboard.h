/**
 CScoreboard
 By: Toh Da Jun
 Date: Jan 2024
 */
#pragma once

// Include SingletonTemplate
#include "../DesignPatterns/SingletonTemplate.h"

// #include ini.h
#include "../System/SimpleIni.h"

#include <string>
#include <utility>
#include <vector>

class CScoreboard : public CSingletonTemplate<CScoreboard>
{
	friend CSingletonTemplate<CScoreboard>;
public:
	struct Entry {
		char name[256];
		int  iScore;

		void setName(std::string str);
		void setScore(const int iScore);
		void PrintSelf(void);
	};
	typedef std::vector<Entry> Entries;
	typedef std::pair<Entries::iterator, int> EntryLocation;

	// Init this class instance
	void Init(void);

	// Load the scoreboard to a file
	bool LoadFile(std::string const& filename);

	// Save to the scoreboard to a file
	bool SaveFile(std::string const& filename) const;

	int  AddEntry(Entry const& entry);
	Entry GetEntry(const int index) const;

	void Prune(const int max_amt);
	int  Size(void) const;

	// For storing the settings read in from .ini file
	CSimpleIniA cSimpleIniA;

protected:
	// Constructor
	CScoreboard(void);

	// Destructor
	virtual ~CScoreboard(void);

	Entries entries;
	static int const SizeOfEntry = sizeof(Entry);

	EntryLocation entryWorseThan(Entry const& entry);
};

