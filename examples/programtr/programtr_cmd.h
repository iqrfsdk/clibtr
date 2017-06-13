/*
 * Parse command line arguments for example application programtr.
 * Author: Vlastimil Kosar <kosar@rehivetrch.com>
 * License: TBD
 */

#ifndef __PROGRAMTR_CMD_H__
#define __PROGRAMTR_CMD_H__

#include <string>
#include <TrIfc.h>

class Commands {
private:
    std::string interface;
    std::string dev;
    std::string hex;
    std::string iqrf;
    std::string trconf;
    TrMemory target;
    bool isHex;
    bool isIqrf;
    bool isTrconf;
    bool valid;
    bool parsed;
    
    void parse(int argc, char * argv[]);
public:
    Commands(int argc, char * argv[]);
    bool isValid(void);
    std::string getInterface(void);
    std::string getDev(void);
    bool programTrconf(void);
    std::string getTrconf(void);
    bool programHex(void);
    std::string getHex(void);
    TrMemory getTarget(void);
    bool programIqrf(void);
    std::string getIqrf(void);
};

#endif // __PROGRAMTR_CMD_H__
