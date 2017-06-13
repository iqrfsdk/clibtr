/*
 * Parse file in hex format into internal representation.
 * Author: Vlastimil Kosar <kosar@rehivetrch.com>
 * License: TBD
 */
#include <vector>
#include <string>
#include <fstream>
#include <array>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <iomanip>

#include "string_operations.h"
#include "HexFmtParser.h"
#include "TrFmtException.h"

const size_t MIN_LINE_LEN = 11;
const size_t MAX_LINE_LEN = 521;
const size_t TR_LINE_LEN = 32; // Any data uploaded to TR flash and external memory must be 32B long
const size_t TR_LINE_LEN_MIN = 1; // Minimal write data length
const size_t TR_LINE_LEN_MAX = 32; // Maximal write data length

const size_t TR_MEMORY_SIZE = 65536; // Maximal memory size of TR - 16b addressing

bool verify_record_csum(const std::string& str) {
    size_t len = str.length() - 1;
    unsigned int sum = 0;
    
    std::string data = str.substr(1, len);
    for (int i = 0; i < len / 2; i++) {
        sum += std::stoul(data.substr(i * 2, 2), nullptr, 16);
    }
    
    return (sum & 0xff) == 0;
}

void generateRecordCsum(std::basic_string<unsigned char> &str) {
    unsigned int sum = 0;
    std::basic_string<unsigned char>::iterator itr;
    
    for (itr = str.begin(); itr != str.end(); itr++) {
        sum += *itr;
    }
    sum = (~sum) & 0xff;
    str.push_back(sum);
}


void HexFmtParser::parse() {
    std::string line;
    std::ifstream infile(file_name);
    size_t line_no = 0;
    size_t position;
    bool finished = false;
    std::vector<HexDataRecord> variableLines;
    std::array<unsigned char, TR_MEMORY_SIZE> prgData;
    std::array<bool, TR_MEMORY_SIZE> prgDataValid;
    
    
    while (std::getline(infile, line))
    {
        size_t len;
        unsigned int base = 0;
        unsigned int offset;
        unsigned int addr;
        size_t data_len;
        unsigned char type;
        std::basic_string<unsigned char> data;
        
        line_no++;
        
        // Trim whitespace
        line = trim(line);
     
        len = line.length();
        
        // Skip empty line
        if (len == 0)
            continue;
        
        // Check if file continue after End Of File record
        if (finished) {
            TR_THROW_FMT_EXCEPTION(file_name, line_no, 0, "Hex file continues after End Of File record!");
        }
        
        // Every line in hex file must be at least 11 characters long
        if (len < MIN_LINE_LEN) {
            TR_THROW_FMT_EXCEPTION(file_name, line_no, 0, "Invalid length of record in hex file - line is too short!");
        }
        
        // Every line in hex file must be at most 521 characters long
        if (len > MAX_LINE_LEN) {
            TR_THROW_FMT_EXCEPTION(file_name, line_no, 0, "Invalid length of record in hex file - line is too long!");
        }
        
        // Every line must be odd
        if (len % 2 != 1) {
            TR_THROW_FMT_EXCEPTION(file_name, line_no, 0, "Invalid length of record in hex file - line length is not odd!");
        }
        
        // Check for invalid characters
        if ((position = line.find_first_not_of(":0123456789abcdefABCDEF")) != std::string::npos) {
            TR_THROW_FMT_EXCEPTION(file_name, line_no, position, "Invalid length character in hex file!");
        }
        
        // Check for record start code
        if (line[0] != ':') {
            TR_THROW_FMT_EXCEPTION(file_name, line_no, 1, "Missing record start code : in hex file!");
        }
        
        // Check checksum
        if (!verify_record_csum(line)) {
            TR_THROW_FMT_EXCEPTION(file_name, line_no, len - 2, "Invalid checksum of record in hex file!");
        }
        
        // Get length
        data_len = std::stoul(line.substr(1, 2), nullptr, 16);
        // Get offset
        offset = std::stoul(line.substr(3, 4), nullptr, 16);
        // Get type
        type = std::stoul(line.substr(7, 2), nullptr, 16);
        
        // Check data length of record
        if (2 * data_len + 11 != len) {
            TR_THROW_FMT_EXCEPTION(file_name, line_no, 2, "Actual length of record in hex file is different from indicated length!");
        }
        
        switch(type) {
            case 0:
                // Data record
                for (size_t i = 0; i < data_len / 2; i++) {
                    data.push_back(std::stoul(line.substr(9 + i * 2, 2), nullptr, 16));
                }
                addr = base + offset;
                variableLines.push_back(HexDataRecord(addr, data));
                break;
            case 1:
                // End Of File record
                if (data_len != 0) {
                    TR_THROW_FMT_EXCEPTION(file_name, line_no, 2, "Data length of End Of File record in hex file must be 0!");
                }
                finished = true;
                break;
            case 2:
                // Extended Segment Address record
                if (data_len != 2) {
                    TR_THROW_FMT_EXCEPTION(file_name, line_no, 2, "Data length of Extended Segment Address record in hex file must be 2!");
                }
                base = std::stoul(line.substr(9, 4), nullptr, 16) * 16;
                break;
            case 3:
                // Start Segment Address record
                if (data_len != 4) {
                    TR_THROW_FMT_EXCEPTION(file_name, line_no, 2, "Data length of Start Segment Address record in hex file must be 4!");
                }
                std::cerr << "Warning: Start Segment Address record (type 03) on line " << line_no << " is ignored!\n";
                std::cerr << "         This type of record has no effect on IQRF TR device.\n";
                break;
            case 4:
                // Extended Linear Address record
                if (data_len != 2) {
                    TR_THROW_FMT_EXCEPTION(file_name, line_no, 2, "Data length of Extended Linear Address record in hex file must be 2!");
                }
                base = std::stoul(line.substr(9, 4), nullptr, 16) << 16;
                break;
            case 5:
                // Start Linear Address record
                if (data_len != 4) {
                    TR_THROW_FMT_EXCEPTION(file_name, line_no, 2, "Data length of Start Linear Address record in hex file must be 4!");
                }
                std::cerr << "Warning: Start Linear Address record (type 05) on line " << line_no << " is ignored!\n";
                std::cerr << "         This type of record has no effect on IQRF TR device.";
                break;
            default:
                TR_THROW_FMT_EXCEPTION(file_name, line_no, 8, "Unknown type of record in hex file!\n");
                break;
        }
    }
    
    if ((memory == TrMemory::FLASH) or (memory == TrMemory::EXTERNAL_EEPROM)) {
        // Convert to Tr upload data
        // Init fake data memory for programming data grouping
        prgData.fill(0);
        prgDataValid.fill(false);
        std::vector<HexDataRecord>::iterator itr;
        std::array<bool, TR_MEMORY_SIZE>::iterator itrPrgDataValid;
        
        // Programm the fake memory and note programmed positions
        for (itr = variableLines.begin(); itr != variableLines.end(); itr++) {
            std::copy((*itr).data.begin(), (*itr).data.end(), prgData.begin() + (*itr).addr);
            std::fill_n(prgDataValid.begin() + (*itr).addr, (*itr).data.size(), true);
        }
        
        // Create Tr prg data lines with width 32B
        for (itrPrgDataValid = prgDataValid.begin(); itrPrgDataValid < prgDataValid.end(); itrPrgDataValid+=TR_LINE_LEN) {
            if (std::any_of(itrPrgDataValid, itrPrgDataValid + TR_LINE_LEN, [](bool b){return b == true;})) {
                std::basic_string<unsigned char> data;
                size_t addr = std::distance(prgDataValid.begin(), itrPrgDataValid);
                data.resize(TR_LINE_LEN);
                std::copy_n(prgData.begin() + addr, TR_LINE_LEN, data.begin());
                blines.push_back(HexDataRecord(addr, data));
            }
        }
    } else if (memory == TrMemory::INTERNAL_EEPROM) {
        // Check length of programming data and splir if length exceeds limit
        std::vector<HexDataRecord>::iterator itr;
        for (itr = variableLines.begin(); itr != variableLines.end(); itr++) {
            size_t dLen = (*itr).data.size();
            if (dLen > TR_LINE_LEN_MAX) {
                // Split programming line which is too long
                int parts = static_cast<int>(ceil(dLen / static_cast<double>(TR_LINE_LEN_MAX)));
                for (int i = 0; i < parts; i++) {
                    size_t addr = (*itr).addr + i * TR_LINE_LEN_MAX;
                    size_t len = (i*TR_LINE_LEN_MAX + TR_LINE_LEN_MAX > dLen) ? (dLen - i*TR_LINE_LEN_MAX) : TR_LINE_LEN_MAX;
                    std::basic_string<unsigned char> data;
                    data.resize(len);
                    std::copy_n((*itr).data.begin() + i*TR_LINE_LEN_MAX, len, data.begin());
                    blines.push_back(HexDataRecord(addr, data));
                }
            } else if (dLen < TR_LINE_LEN_MIN) {
                TR_THROW_FMT_EXCEPTION(file_name, 0, 0, "Empty data line in hex file!\n");
            } else {
                blines.push_back(*itr);
            }
        }
    } else {
        TR_THROW_FMT_EXCEPTION(file_name, 0, 0, "Invalid TR memory type for HEX file!\n");
    }
}

void HexFmtParser::pushBack(unsigned int addr, std::basic_string<unsigned char> data) {
    blines.push_back(HexDataRecord(addr, data));
}

void HexFmtParser::save() {
    std::ofstream outfile(file_name);
    int line_no = 0;
    std::vector<HexDataRecord>::iterator itr;
    std::basic_string<unsigned char>::iterator strItr;
    
    for (itr = blines.begin(); itr != blines.end(); itr++) {
        line_no++;
        if ((*itr).addr >= TR_MEMORY_SIZE) {
            TR_THROW_FMT_EXCEPTION(file_name, line_no, 0, "Invalid address for I8HEX mode! I32HEX mode is currently unsupported for download.\n");
        }
        std::basic_string<unsigned char> data;
        data.push_back((*itr).data.size());
        data.push_back(((*itr).addr >> 8) & 0xff);
        data.push_back((*itr).addr & 0xff);
        data.push_back(0);
        data += (*itr).data;
        generateRecordCsum(data);
        outfile << ":";
        for (strItr = data.begin(); strItr != data.end(); strItr++) {
            outfile << std::setw(2) << std::setfill ('0') << std::hex << static_cast<int>(*strItr);
        }
        outfile << "\n";
    }
    outfile << ":00000001FF\n";
}
