/*
 * Example application for clibtr - programm TR.
 * Author: Vlastimil Kosar <kosar@rehivetech.com>
 * License: TBD
 */

#include <iostream>
#include <string>

#include <IqrfCdcChannel.h>
#include <IqrfSpiChannel.h>
#include <IqrfFakeChannel.h>
#include <TrIfc.h>
#include <IqrfLogging.h>
#include <programtr_cmd.h>

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


void programTr(IChannel * channel, Commands& cmd) {
    TrIfc ifc(channel);
    std::basic_string<unsigned char> data;
    unsigned char val;
    
    ifc.enterProgrammingMode();
    
    try {
        if (cmd.programTrconf()) {
            ifc.uploadCfg(cmd.getTrconf());
        }
        
        if (cmd.programHex()) {
            ifc.uploadHex(cmd.getTarget(), cmd.getHex());
        }
        
        if (cmd.programIqrf()) {
            ifc.uploadIqrf(cmd.getIqrf());
        }
        
        ifc.terminateProgrammingMode();
    } catch (std::exception& e) {
        std::cout << "Standard exception: " << e.what() << std::endl;
        ifc.terminateProgrammingMode();
    }
}

void help(void) {
    std::cout << "programtr -i <interface> -d <dev> [-c <trconf> | -p <hex> -t <target> | -q <iqrf> ] \n";
    std::cout << "Program TR connected to specified interface.\n";
    std::cout << "Parameters:\n";
    std::cout << "-i <interface> - interface for communication with TR. Supported interfaces are:\n";
    std::cout << "                   cdc - TR attached via USB CDC\n";
    std::cout << "                   spi - TR attached via SPI\n";
    std::cout << "                   test - TR emulator for testing\n";
    std::cout << "-d <dev>       - interface device file\n";
    std::cout << "-c <trconf>    - program TR with TRCONF configuration file <trconf>\n";
    std::cout << "-p <hex>       - program TR with HEX programming file <hex>\n";
    std::cout << "-t <target>    - target memory for HEX programming file. Valid values are:\n";
    std::cout << "                   flash    - flash onchip memory\n";
    std::cout << "                   internal - internal eeprom memory\n";
    std::cout << "                   external - external eeprom memory\n";
    std::cout << "-q <hex>       - program TR with IQRF programming file <iqrf>\n";
}

int main (int argc, char * argv[]) {
    Commands cmd(argc, argv);
    
    if (!cmd.isValid()) {
        help();
        exit(1);
    }
    
    IChannel * channel = getInterface(cmd.getInterface(), cmd.getDev());
    if (channel == nullptr) {
        std::cerr << "Unknown interface specified: " << cmd.getInterface() << "\n";
        exit(2);
    }
    programTr(channel, cmd);
}
