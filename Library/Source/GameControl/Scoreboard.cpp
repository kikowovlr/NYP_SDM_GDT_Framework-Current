/**
 CScoreboard
 By: Toh Da Jun
 Date: Jan 2024
 */
#include "Scoreboard.h"
#include <fstream>
#include <algorithm>
#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CScoreboard::CScoreboard()
{
}

/**
 @brief Destructor
 */
CScoreboard::~CScoreboard()
{
}

/**
 @brief Init this class instance
 */
void CScoreboard::Init(void)
{
	cSimpleIniA.SetUnicode();
}

/**
 @brief Load the scoreboard to a file
 @param filename A std::string const& variable which contains the name of the .ini file
 @return true if the file was successfully loaded, otherwise false
 */
bool CScoreboard::LoadFile(std::string const& filename)
{
	ifstream f;
	f.open(filename.c_str(), ios::binary);
	if (!f.is_open()) { return false; }

	Entry entry_buffer;
	f.read(reinterpret_cast<char*>(&entry_buffer), SizeOfEntry);
	entries.clear();

	while (!f.eof()) {
		entries.push_back(entry_buffer);
		f.read(reinterpret_cast<char*>(&entry_buffer), SizeOfEntry);
	}
	f.close();
	return true;
}

/**
 @brief Save to the scoreboard to a file
 @param filename A std::string const& variable which contains the name of the .ini file
 @return true if the file was successfully saved, otherwise false
 */
bool CScoreboard::SaveFile(std::string const& filename) const {
	ofstream f;
	f.open(filename.c_str(), ios::binary | ios::out);
	if (!f.is_open()) 
	{ 
		return false; 
	}

	f.write(reinterpret_cast<char const*>(&entries[0]), SizeOfEntry * entries.size());
	f.close();
	return true;
}

/**
 @brief 
 */
int CScoreboard::AddEntry(Entry const& entry) {
	auto loc = entryWorseThan(entry);
	entries.insert(loc.first, entry);
	return loc.second;
}

/**
 @brief
 */
CScoreboard::Entry CScoreboard::GetEntry(int index) const {
	return entries[index];
}

/**
 @brief
 */
void CScoreboard::Prune(int max_amt) {
	entries.resize(max_amt);
}

/**
 @brief
 */
int CScoreboard::Size() const 
{ 
	return entries.size(); 
}

/**
 @brief
 */
CScoreboard::EntryLocation CScoreboard::entryWorseThan(Entry const& entry) 
{
	int i = 0;
	for (auto it = entries.begin(); it != entries.end(); ++it, ++i) {
		if (it->iScore <= entry.iScore) {
			return EntryLocation(it, i);
		}
	}
	return EntryLocation(entries.end(), i);
}

/**
 @brief
 */
void CScoreboard::Entry::setName(std::string str) 
{
	memcpy(&name[0], &str.c_str()[0], sizeof(char) * (str.length() + 1));
}

/**
 @brief
 */
void CScoreboard::Entry::setScore(const int iScore)
{
	this->iScore = iScore;
}

/**
 @brief
 */
void CScoreboard::Entry::PrintSelf(void)
{
	cout << name << ", " << iScore << endl;
}
