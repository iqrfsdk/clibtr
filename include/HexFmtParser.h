/*
 * Parse file in hex format into internal representation.
 * Author: Vlastimil Kosar <kosar@rehivetrch.com>
 * License: TBD
 */

#ifndef __HEXFMTPARSER_H__
#define __HEXFMTPARSER_H__

#include <vector>
#include <string>
#include <array>

#include <TrTypes.h>

struct HexDataRecord {
    unsigned int addr;
    std::basic_string<unsigned char> data;
    HexDataRecord(unsigned int a, std::basic_string<unsigned char> d) : addr(a), data(d) {}
};

class HexFmtParser {
private:
    std::string file_name;
    std::vector<HexDataRecord> blines;
    TrMemory memory;
public:
    HexFmtParser(TrMemory memory, std::string name) : memory(memory), file_name(name) {}
    void parse();
    typedef std::vector<HexDataRecord>::iterator iterator;
    typedef std::vector<HexDataRecord>::const_iterator const_iterator;
    iterator begin() { return blines.begin(); }
    iterator end() { return blines.end(); }
    void pushBack(unsigned int addr, std::basic_string<unsigned char> data);
    void save();
};

#endif // __HEXFMTPARSER_H__
