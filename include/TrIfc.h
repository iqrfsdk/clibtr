/*
 * TR interface for configuration.
 * Author: Vlastimil Kosar <kosar@rehivetrch.com>
 * License: TBD
 */

#ifndef __TRIFC_H__
#define __TRIFC_H__

#include <string>

#include <IChannel.h>
#include <TrTypes.h>

class TrIfc {
private:
    // Interface channel to communicate with TR
    IChannel* ifc;
    
    // We are in proggramming mode
    bool prgMode;
public:
    TrIfc(IChannel* c) : ifc(c), prgMode(false) {}
    
    // Enter programming mode
    void enterProgrammingMode();
    
    // Terminate proggramming mode
    void terminateProgrammingMode();
    
    // Upload to device
    // Upload Tr configuration - HWP profile
    void uploadCfg(const std::basic_string<unsigned char>& data);
    // Upload RFPMG
    void uploadRFPMG(unsigned char rfpmg);
    // Upload RFBAND
    void uploadRFBAND(unsigned char rfband);
    // Upload TR access password
    void uploadAccessPwd(const std::basic_string<unsigned char>& data);
    // Upload TR user key
    void uploadUserKey(const std::basic_string<unsigned char>& data);
    // Upload TR flash memory
    void uploadFlash(unsigned int addr, const std::basic_string<unsigned char>& data);
    // Upload TR internal eeprom memory
    void uploadInternalEeprom(unsigned int addr, const std::basic_string<unsigned char>& data);
    // Upload TR external eeprom memory
    void uploadExternalEeprom(unsigned int addr, const std::basic_string<unsigned char>& data);
    // Upload special
    void uploadSpecial(const std::basic_string<unsigned char>& data);
    
    // Upload files
    void uploadHex(TrMemory memory, std::string name);
    void uploadIqrf(std::string name);
    void uploadCfg(std::string name);
    
    // Download from device
    // Download Tr configuration - HWP profile
    void downloadCfg(std::basic_string<unsigned char>& data);
    // Download RFPMG
    unsigned char downloadRFPMG();
    // Download RFBAND
    unsigned char downloadRFBAND();
    // Download TR flash memory
    void downloadFlash(unsigned int addr, std::basic_string<unsigned char>& data);
    // Download TR internal eeprom memory
    void downloadInternalEeprom(unsigned int addr, std::basic_string<unsigned char>& data);
    // Download TR external eeprom memory
    void downloadExternalEeprom(unsigned int addr, std::basic_string<unsigned char>& data);
    
    // Download files
    void downloadCfg(std::string name);
    void downloadHex(TrMemory memory, unsigned int addr, size_t len, std::string name);
};

#endif // __TRIFC_H__
