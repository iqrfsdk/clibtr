/*
 * Example application for clibtr - download TR memory.
 * Author: Vlastimil Kosar <kosar@rehivetech.com>
 * License: TBD
 */

#include <iostream>
#include <algorithm>
#include <string>
#include <iomanip>

#include <IqrfCdcChannel.h>
#include <IqrfSpiChannel.h>
#include <IqrfFakeChannel.h>
#include <TrIfc.h>
#include <IqrfLogging.h>

TRC_INIT("log.txt")

IChannel * getInterface(std::string interface, std::string dev) {
    IChannel * channel = nullptr;
    
    if (interface == "cdc") {
        channel = new IqrfCdcChannel(dev);
    }
    if (interface == "spi") {
        channel = new IqrfSpiChannel(dev);
    }
    if (interface == "test") {
        channel = new IqrfFakeChannel(dev);
    }
    return channel;
}

void printHexVal2(unsigned char val) {
    std::cout << std::setw(2) << std::setfill ('0') << std::hex << static_cast<int>(val) << " ";
}

void printHexVal4(unsigned int val) {
    std::cout << std::setw(4) << std::setfill ('0') << std::hex << static_cast<int>(val) << " ";
}

void printData(const std::string& comment, std::basic_string<unsigned char>& data) {
    std::cout << comment;
    for_each(data.begin(), data.end(), printHexVal2);
    std::cout << std::endl;
}

void printData(const std::string& comment, unsigned char val) {
    std::cout << comment;
    printHexVal2(val);
    std::cout << std::endl;
}

void printData(const std::string& comment, unsigned int addr, std::basic_string<unsigned char>& data) {
    std::cout << comment;
    printHexVal4(addr);
    std::cout << ": ";
    for_each(data.begin(), data.end(), printHexVal2);
    std::cout << std::endl;
}

void runTest(IChannel * channel) {
    TrIfc ifc(channel);
    std::basic_string<unsigned char> data;
    unsigned char val;
    
    ifc.enterProgrammingMode();
    
    try {
        ifc.downloadCfg(data);
        printData("Configuration: ", data);
        
        val = ifc.downloadRFPMG();
        printData("RFPMG: ", val);
        
        val = ifc.downloadRFBAND();
        printData("RFBAND: ", val);
        
        for (int i = 0x3A00; i < 0x3FFF; i += 32) {
            ifc.downloadFlash(i, data);
            printData("Application Flash: ", i, data);
        }
        
        for (int i = 0x2C00; i < 0x37BF; i += 32) {
            ifc.downloadFlash(i, data);
            printData("External Flash: ", i, data);
        }
        
        for (int i = 0; i < 0x00a0; i += 32) {
            ifc.downloadInternalEeprom(i, data);
            printData("Internal EEPROM: ", i, data);
        }
        
        for (int i = 0; i < 0x7fe0; i += 32) {
            ifc.downloadExternalEeprom(i, data);
            printData("External EEPROM: ", i, data);
        }
        
        ifc.downloadCfg("test.dat");
        
        ifc.downloadHex(TrMemory::FLASH, 0x3A00, 0x600, "flash_app.hex");
        ifc.downloadHex(TrMemory::FLASH, 0x2C00, 0xbc0, "flash_ext.hex");
        ifc.downloadHex(TrMemory::INTERNAL_EEPROM, 0, 0x00a0, "internal.hex");
        ifc.downloadHex(TrMemory::EXTERNAL_EEPROM, 0, 0x7fe0, "external.hex");
        
        ifc.terminateProgrammingMode();
    } catch (std::exception& e) {
        std::cout << "Standard exception: " << e.what() << std::endl;
        ifc.terminateProgrammingMode();
    }
    
}

int main (int argc, char * argv[]) {
    if (argc != 3) {
        std::cout << "readtr <interface> <dev>\n";
        std::cout << "Download data from TR connected to specified interface.\n";
        std::cout << "Parameters:\n";
        std::cout << "<interface> - interface for communication with TR. Supported interfaces are:\n";
        std::cout << "                cdc - TR attached via USB CDC\n";
        std::cout << "                spi - TR attached via SPI\n";
        std::cout << "                test - TR emulator for testing\n";
        std::cout << "<dev>       - interface device file\n";
        exit(1);
    }
    IChannel * channel = getInterface(argv[1], argv[2]);
    if (channel == nullptr) {
        std::cerr << "Unknown interface specified: " << argv[1] << "\n";
        exit(2);
    }
    runTest(channel);
}
