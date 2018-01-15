/*
 * TR interface for configuration.
 * Author: Vlastimil Kosar <kosar@rehivetrch.com>
 * License: TBD
 */

#include <TrException.h>
#include <TrIfc.h>
#include <HexFmtParser.h>
#include <IqrfFmtParser.h>
#include <TrconfFmtParser.h>
#include <CdcInterface.h>

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

// Programming communication direction
static const unsigned char UPLOAD                 = 0x80;
static const unsigned char DOWNLOAD               = 0x00;

// Programming targets
static const unsigned char CFG_TARGET             = 0x00;
static const unsigned char RFPMG_TARGET           = 0x01;
static const unsigned char RFBAND_TARGET          = 0x02;
static const unsigned char ACCESS_PWD_TARGET      = 0x03;
static const unsigned char USER_KEY_TARGET        = 0x04;
static const unsigned char FLASH_TARGET           = 0x05;
static const unsigned char INTERNAL_EEPROM_TARGET = 0x06;
static const unsigned char EXTERNAL_EEPROM_TARGET = 0x07;
static const unsigned char SPECIAL_TARGET         = 0x08;

// Length, range and other constants
static const size_t CFG_LEN                     = 32;
static const unsigned char CFG_CHKSUM_INIT      = 0x5f;
static const size_t ACCESS_PWD_LEN              = 16;
static const size_t USER_KEY_LEN                = 16;
static const size_t FLASH_UP_MODULO             = 16;
static const size_t FLASH_DOWN_MODULO           = 32;
static const unsigned int FLASH_APP_LOW         = 0x3a00;
static const unsigned int FLASH_APP_HIGH        = 0x3fff; // TODO: Fix upper limit
static const unsigned int FLASH_EXT_LOW         = 0x2c00; 
static const unsigned int FLASH_EXT_HIGH        = 0x37bf; // TODO: Fix upper limits
static const size_t FLASH_LEN                   = 32;
static const unsigned int INT_EEPROM_UP_LOW     = 0x0000;
static const unsigned int INT_EEPROM_UP_HIGH    = 0x00bf;
static const size_t INT_EEPROM_UP_ADDR_LEN_MAX  = 0x00c0;
static const size_t INT_EEPROM_UP_LEN_MIN       = 1;
static const size_t INT_EEPROM_UP_LEN_MAX       = 32;
static const unsigned int INT_EEPROM_DOWN_LOW   = 0x0000;
static const unsigned int INT_EEPROM_DOWN_HIGH  = 0x00a0;
static const size_t INT_EEPROM_DOWN_LEN         = 32;
static const unsigned int EXT_EEPROM_LOW        = 0x0000;
static const unsigned int EXT_EEPROM_UP_HIGH    = 0x3fe0;
static const unsigned int EXT_EEPROM_DOWN_HIGH  = 0x7fe0;
static const size_t EXT_EEPROM_MODULO           = 32;
static const size_t EXT_EEPROM_LEN              = 32;
static const size_t SPECIAL_LEN                 = 18;


void TrIfc::enterProgrammingMode() {
    if (!prgMode) {
        ifc->enterProgrammingMode();
        prgMode = true;
    }
}

void TrIfc::terminateProgrammingMode() {
    if (prgMode) {
        ifc->terminateProgrammingMode();
        prgMode = false;
    }
}

static unsigned char computeCfgChksum(const std::basic_string<unsigned char>& data) {
    unsigned char chksum = CFG_CHKSUM_INIT;

    for (int i = 1; i < data.length(); i++) {
        chksum ^= data[i];
    }
    
    return chksum;
}

void TrIfc::uploadCfg(const std::basic_string<unsigned char>& data) {
    if (data.length() != CFG_LEN) {
        TR_THROW_EXCEPTION(TrException, "Invalid length of the TR HWP configuration data!");
    }

    
    if (computeCfgChksum(data) != data[0]) {
        TR_THROW_EXCEPTION(TrException, "Invalid TR HWP configuration checksum!");
    }
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->upload(CFG_TARGET|UPLOAD, data); 
}

void TrIfc::uploadRFPMG(unsigned char rfpmg) {
    std::basic_string<unsigned char> data;
    
    data += rfpmg;
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->upload(RFPMG_TARGET|UPLOAD, data);
}

void TrIfc::uploadRFBAND(unsigned char rfband) {
    std::basic_string<unsigned char> data;
    
    data += rfband;
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->upload(RFBAND_TARGET|UPLOAD, data);
}

void TrIfc::uploadAccessPwd(const std::basic_string<unsigned char>& data) {
    if (data.length() != ACCESS_PWD_LEN) {
        TR_THROW_EXCEPTION(TrException, "Invalid length of access password!");
    }
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->upload(ACCESS_PWD_TARGET|UPLOAD, data);
}

void TrIfc::uploadUserKey(const std::basic_string<unsigned char>& data) {
    if (data.length() != USER_KEY_LEN) {
        TR_THROW_EXCEPTION(TrException, "Invalid length of user key!");
    }
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->upload(USER_KEY_TARGET|UPLOAD, data);
}

static void insertAddress(std::basic_string<unsigned char> &msg, unsigned int addr) {
	msg += addr & 0xff;
    msg += (addr >> 8) &  0xff;
}

static void insertAddressData(std::basic_string<unsigned char> &msg, unsigned int addr, 
				   const std::basic_string<unsigned char>& data) {
	msg += addr & 0xff;
    msg += (addr >> 8) &  0xff;
    msg += data;
}

void TrIfc::uploadFlash(unsigned int addr, const std::basic_string<unsigned char>& data) {
    std::basic_string<unsigned char> msg;
    
    // Address in Flash is in 16b words not in bytes
    addr = addr / 2;
    
    if (addr % FLASH_UP_MODULO != 0) {
        TR_THROW_EXCEPTION(TrException, "Address in flash memory should be modulo 16!");
    }
    
    if (!(((addr >= FLASH_APP_LOW) && (addr <= FLASH_APP_HIGH)) || ((addr >= FLASH_EXT_LOW) && (addr <= FLASH_EXT_HIGH)))) {
        TR_THROW_EXCEPTION(TrException, "Address in flash memory is outside application or extended flash memory!");
    }
    
    if (data.length() != FLASH_LEN) {
        TR_THROW_EXCEPTION(TrException, "Data to be programmed into the flash memory must be 32B long!");
    }
    
    insertAddressData(msg, addr, data);
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->upload(FLASH_TARGET|UPLOAD, msg);
}

void TrIfc::uploadInternalEeprom(unsigned int addr, const std::basic_string<unsigned char>& data) {
    std::basic_string<unsigned char> msg;
        
    if (!((addr >= INT_EEPROM_UP_LOW) && (addr <= INT_EEPROM_UP_HIGH))) {
        TR_THROW_EXCEPTION(TrException, "Address in internal eeprom memory is outside of addressable range!");
    }
    
    if (addr + data.length() >= INT_EEPROM_UP_ADDR_LEN_MAX) {
        TR_THROW_EXCEPTION(TrException, "End of write is out of the addressable range of the internal eeprom!");
    }
    
    if ((data.length() < INT_EEPROM_UP_LEN_MIN) || (data.length() > INT_EEPROM_UP_LEN_MAX)) {
        TR_THROW_EXCEPTION(TrException, "Data to be programmed into the internal eeprom memory must be 1-32B long!");
    }
    
    insertAddressData(msg, addr, data);
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->upload(INTERNAL_EEPROM_TARGET|UPLOAD, msg);
}

void TrIfc::uploadExternalEeprom(unsigned int addr, const std::basic_string<unsigned char>& data) {
    std::basic_string<unsigned char> msg;
    
    if (!((addr >= EXT_EEPROM_LOW) && (addr <= EXT_EEPROM_UP_HIGH))) {
        TR_THROW_EXCEPTION(TrException, "Address in external eeprom memory is outside of addressable range!");
    }
    
    if (addr % EXT_EEPROM_MODULO != 0) {
        TR_THROW_EXCEPTION(TrException, "Address in external eeprom memory should be modulo 32!");
    }
    
    if (data.length() != EXT_EEPROM_LEN) {
        TR_THROW_EXCEPTION(TrException, "Data to be programmed into the external eeprom memory must be 32B long!");
    }
    
    insertAddressData(msg, addr, data);
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->upload(EXTERNAL_EEPROM_TARGET|UPLOAD, msg);
}

void TrIfc::uploadSpecial(const std::basic_string<unsigned char>& data) {   
    if (data.length() != SPECIAL_LEN) {
        TR_THROW_EXCEPTION(TrException, "Data to be programmed by the special upload must be 18B long!");
    }
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->upload(SPECIAL_TARGET|UPLOAD, data);
}


void TrIfc::uploadHex(TrMemory memory, std::string name) {
    HexFmtParser::iterator itr;
    
    HexFmtParser parser(memory, name);
    
    parser.parse();
    
    for (itr = parser.begin(); itr != parser.end(); itr++) {
        switch(memory) {
            case TrMemory::FLASH:
                uploadFlash((*itr).addr, (*itr).data);
                break;
            case TrMemory::INTERNAL_EEPROM:
                uploadInternalEeprom((*itr).addr, (*itr).data);
                break;
            case TrMemory::EXTERNAL_EEPROM:
                uploadExternalEeprom((*itr).addr, (*itr).data);
                break;
        }
    }
}

static TrModuleInfo getTrModuleInfo(ModuleInfo* moduleInfo) {
    TrModuleInfo info;
    
    info.osVersion = moduleInfo->osVersion;
    switch(moduleInfo->PICType & 0x7) {
        case 4:
            info.mcu = TrMcu::PIC16F1938;
            break;
        default:
            TR_THROW_EXCEPTION(TrException, "Unknown type of MCU download from TR: " + std::to_string(moduleInfo->PICType & 0x7) + "!");
            break;
    }
    
    info.osBuild = (moduleInfo->osBuild[1] << 8) | moduleInfo->osBuild[0];
    switch(moduleInfo->PICType >> 4) {
        case 0:
        case 1:
        case 3:
        case 8:
        case 9:
        case 10:
            info.serie = TrSerie::DCTR_5xD;
            break;
        case 2:
        case 11:
            info.serie = TrSerie::DCTR_7xD;
            break;
        default:
            TR_THROW_EXCEPTION(TrException, "Unknown serie downloaded from TR: " + std::to_string(moduleInfo->PICType >> 4) + "!");
            break;
    }
    
    return info;
}

void TrIfc::uploadIqrf(std::string name) {
    IqrfFmtParser::iterator itr;
    terminateProgrammingMode();
    TrModuleInfo info = getTrModuleInfo(static_cast<ModuleInfo*>(ifc->getTRModuleInfo()));
    enterProgrammingMode();
    IqrfFmtParser parser(name);
    
    parser.parse();
    
    if (!parser.check(info)) {
        TR_THROW_EXCEPTION(TrException, "IQRF file " + name + " can not be upload to TR! TR is not in supported types specified in the IQRF file. This message is caused by incopatible type of TR, OS version or OS build.");
    }
    
    for (itr = parser.begin(); itr != parser.end(); itr++) {
        uploadSpecial(*itr);
    }
}

void TrIfc::uploadCfg(std::string name) {
    unsigned char rfpmg;
    TrconfFmtParser parser(name);
    parser.parse();
    
    rfpmg = parser.getRFPMG();
    
    parser.checkChannels(downloadRFBAND());
    
    uploadCfg(parser.getData());
    uploadRFPMG(rfpmg);
}

void TrIfc::downloadCfg(std::basic_string<unsigned char>& data) {
    std::basic_string<unsigned char> msg;
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->download(CFG_TARGET|DOWNLOAD, msg, data);
    
    if (data.length() != CFG_LEN) {
        TR_THROW_EXCEPTION(TrException, "Invalid length of downloaded configuration data!");
    }
    
    if (computeCfgChksum(data) != data[0]) {
        TR_THROW_EXCEPTION(TrException, "Invalid TR HWP configuration checksum in downloaded configuration data!");
    }
}

unsigned char TrIfc::downloadRFPMG() {
    std::basic_string<unsigned char> msg;
    std::basic_string<unsigned char> data;
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->download(RFPMG_TARGET|DOWNLOAD, msg, data);
    return data[0];
}
unsigned char TrIfc::downloadRFBAND() {
    std::basic_string<unsigned char> msg;
    std::basic_string<unsigned char> data;
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->download(RFBAND_TARGET|DOWNLOAD, msg, data);
    return data[0];
}

void TrIfc::downloadFlash(unsigned int addr, std::basic_string<unsigned char>& data) {
    std::basic_string<unsigned char> msg;
    
    if (addr % FLASH_DOWN_MODULO != 0) {
        TR_THROW_EXCEPTION(TrException, "Address in flash memory should be modulo 16!");
    }
    
    if (!(((addr >= FLASH_APP_LOW) && (addr <= FLASH_APP_HIGH)) || ((addr >= FLASH_EXT_LOW) && (addr <= FLASH_EXT_HIGH)))) {
        TR_THROW_EXCEPTION(TrException, "Address in flash memory is outside application or extended flash memory!");
    }
       
    insertAddress(msg, addr);
	
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->download(FLASH_TARGET|DOWNLOAD, msg, data);
}

void TrIfc::downloadInternalEeprom(unsigned int addr, std::basic_string<unsigned char>& data) {
    std::basic_string<unsigned char> msg;
        
    if (!((addr >= INT_EEPROM_DOWN_LOW) && (addr <= INT_EEPROM_DOWN_HIGH))) {
        TR_THROW_EXCEPTION(TrException, "Address in internal eeprom memory is outside of addressable range!");
    }
        
    insertAddress(msg, addr);
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->download(INTERNAL_EEPROM_TARGET|DOWNLOAD, msg, data);
	
	if (data.length() != INT_EEPROM_DOWN_LEN) {
        TR_THROW_EXCEPTION(TrException, "Data from internal eeprom memory must be 32B long!");
    }
}
void TrIfc::downloadExternalEeprom(unsigned int addr, std::basic_string<unsigned char>& data) {
    std::basic_string<unsigned char> msg;
    
    if (!((addr >= EXT_EEPROM_LOW) && (addr <= EXT_EEPROM_DOWN_HIGH))) {
        TR_THROW_EXCEPTION(TrException, "Address in external eeprom memory is outside of addressable range!");
    }
    
    if (addr % EXT_EEPROM_MODULO != 0) {
        TR_THROW_EXCEPTION(TrException, "Address in external eeprom memory should be modulo 32!");
    }
    
    insertAddress(msg, addr);
    
    if (!prgMode) {
        TR_THROW_EXCEPTION(TrException, "TR is not in programming mode!");
    }
    
    ifc->download(EXTERNAL_EEPROM_TARGET|DOWNLOAD, msg, data);
	
	if (data.length() != EXT_EEPROM_LEN) {
        TR_THROW_EXCEPTION(TrException, "Data from external eeprom memory must be 32B long!");
    }
}

void TrIfc::downloadCfg(std::string name) {
    char buffer[CFG_LEN + 1];
    unsigned char *bptr = reinterpret_cast<unsigned char*>(buffer);
    std::ofstream outfile(name, std::ios::binary);
    std::basic_string<unsigned char> data;

    downloadCfg(data);
 
    std::copy_n(data.begin(), CFG_LEN, bptr);
    buffer[32] = downloadRFPMG();
    
    if (!outfile.write(buffer, CFG_LEN + 1)) {
        TR_THROW_EXCEPTION(TrException, "Can not write downloaded configuration data!");
    }
}

void TrIfc::downloadHex(TrMemory memory, unsigned int addr, size_t len, std::string name) {
    HexFmtParser parser(memory, name);
    
    for (size_t i = addr; i < addr + len; i+= 32) {
        std::basic_string<unsigned char> data;
        data.resize(32);
        
        switch(memory) {
            case TrMemory::FLASH:
                downloadFlash(i, data);
                break;
            case TrMemory::INTERNAL_EEPROM:
                downloadInternalEeprom(i, data);
                break;
            case TrMemory::EXTERNAL_EEPROM:
                downloadExternalEeprom(i, data);
                break;
        }
        // Truncate last memory block
        if ((i + 32) > addr + len) {
            data.resize(addr + len - i);
        }
        // Push back into Hex file internal representation
        parser.pushBack(i, data);
    }
    parser.save();
}
