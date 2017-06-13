/*
 * Parse command line arguments for example application programtr.
 * Author: Vlastimil Kosar <kosar@rehivetrch.com>
 * License: TBD
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <iomanip>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <TrIfc.h>
#include <programtr_cmd.h>

const char * PARAMS = "i:d:c:p:t:q:";

static TrMemory parseTarget(std::string val) {
    if (val == "flash")
        return TrMemory::FLASH;
    if (val == "internal")
        return TrMemory::INTERNAL_EEPROM;
    if (val == "external")
        return TrMemory::EXTERNAL_EEPROM;
    std::cerr << "Invalid memory type " << val << "!\n";
    return TrMemory::ERROR;
}

void Commands::parse(int argc, char * argv[]) {
    int c;
    opterr = 0;
    valid = true;
    bool isInterface = false;
    bool isDev = false;
    bool isTarget = false;
    
    while ((c = getopt (argc, argv, PARAMS)) != -1)
        switch (c) {
        case 'i':
            interface = optarg;
            isInterface = true;
            break;
        case 'd':
            dev = optarg;
            isDev = true;
            break;
        case 'c':
            trconf = optarg;
            isTrconf = true;
            break;
        case 'p':
            hex = optarg;
            isHex = true;
            break;
        case 't':
            target = parseTarget(optarg);
            if (target == TrMemory::ERROR) {
                isTarget = false;
            } else {
                isTarget = true;
            }
            break;
        case 'q':
            iqrf = optarg;
            isIqrf = true;
            break;
        case '?':
            if (optopt == 'i' or optopt == 'd' or optopt == 'c' or optopt == 'p' or optopt == 't' or optopt == 'q') {
                std::cerr << "Option -" << static_cast<char>(optopt) << " requires an argument.\n";
                valid = false;
            } else if (isprint (optopt)) {
                std::cerr << "Unknown option `-" << static_cast<char>(optopt) <<"'.\n";
                valid = false;
            } else {
                std::cerr << "Unknown option character `0x" << std::setw(2) << std::setfill ('0') << std::hex << "'.\n";
                valid = false;
            }
            break;
        default:
            std::cerr << "Error while parsing commandline parameters!\n";
            valid = false;
            break;
        }
    parsed = true;
    if (!(isHex or isIqrf or isTrconf)) {
        std::cerr << "No configuration file specified!\n";
        valid = false;
    }
    
    if (isTarget and not isHex) {
        std::cerr << "Configuration option -t has no effect without command line option -p!\n";
    }
    
    if (isHex and not isTarget) {
        std::cerr << "Configuration option -p must be used togather with command line option -t!\n";
        valid = false;
    }
    
    if (!isInterface) {
        std::cerr << "Configuration option -i must be specified!\n";
        valid = false;
    }
    
    if (!isDev) {
        std::cerr << "Configuration option -d must be specified!\n";
        valid = false;
    }
}

Commands::Commands(int argc, char * argv[]) {
    isHex = false;
    isIqrf = false;
    isTrconf = false;
    valid = false;
    parsed = false;
    
    parse(argc, argv);
}

bool Commands::isValid(void) {
    if (parsed and valid) {
        return true;
    } else {
        return false;
    }
}

std::string Commands::getInterface(void) {
    if (isValid()) {
        return interface;
    } else {
        throw std::runtime_error("Can not get interface for communication with TR!");
    }
}

std::string Commands::getDev(void) {
    if (isValid()) {
        return dev;
    } else {
        throw std::runtime_error("Can not get device for TR interface!");
    }
}

bool Commands::programTrconf(void) {
    if (isValid() and isTrconf) {
        return true;
    } else {
        return false;
    }
}

std::string Commands::getTrconf(void) {
    if (isValid() and isTrconf) {
        return trconf;
    } else {
        throw std::runtime_error("Can not get nonexistent file name of TRCONF file!");
    }
}

bool Commands::programHex(void) {
    if (isValid() and isHex) {
        return true;
    } else {
        return false;
    }
}

std::string Commands::getHex(void) {
    if (isValid() and isHex) {
        return hex;
    } else {
        throw std::runtime_error("Can not get nonexistent file name of HEX file!");
    }
}

TrMemory Commands::getTarget(void) {
    if (isValid() and isHex) {
        return target;
    } else {
        throw std::runtime_error("Can not get nonexistent target memory for HEX file!");
    }
}

bool Commands::programIqrf(void) {
    if (isValid() and isIqrf) {
        return true;
    } else {
        return false;
    }
}

std::string Commands::getIqrf(void) {
    if (isValid() and isIqrf) {
        return iqrf;
    } else {
        throw std::runtime_error("Can not get nonexistent file name of IQRF file!");
    }
}
