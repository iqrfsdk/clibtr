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

#ifdef _WIN32
#include <tclap/CmdLine.h>
#else
#include <unistd.h>
#endif // _WIN32

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

#ifdef _WIN32
// using tclap library
void Commands::parse(int argc, char * argv[]) {
  valid = true;
  bool isInterface = false;
  bool isDev = false;
  bool isTarget = false;

  try {
	// define the command line object, and insert a command description message
	TCLAP::CmdLine cmd(
	  "programtr -i <interface> -d <dev> [-c <trconf> | -p <hex> -t <target> | -q <iqrf> ] \n",
	  ' ', 
	  "0.9"
	);

	// Define a value argument and add it to the command line.
	// A value arg defines a flag and a type of value that it expects,
	// such as "-n Bishop".
	TCLAP::ValueArg<std::string> ifaceArg(
	  "i", 
	  "inteface", 
	  "interface for communication with TR", 
	  false, 
	  "",
	  "string"
	);

	// The CmdLine object uses this Arg to parse the command line.
	cmd.add(ifaceArg);

	TCLAP::ValueArg<std::string> ifaceDeviceArg(
	  "d",
	  "device",
	  "interface device file",
	  false,
	  "",
	  "string"
	);
	cmd.add(ifaceDeviceArg);

	TCLAP::ValueArg<std::string> configFileArg(
	  "c",
	  "TRCONF_configuration_file",
	  "program TR with TRCONF configuration file <trconf>",
	  false,
	  "",
	  "string"
	);
	cmd.add(configFileArg);

	TCLAP::ValueArg<std::string> hexProgFileArg(
	  "p",
	  "HEX_programmong_file",
	  "program TR with HEX programming file <hex>",
	  false,
	  "",
	  "string"
	);
	cmd.add(hexProgFileArg);

	TCLAP::ValueArg<std::string> targetMemoryArg(
	  "t",
	  "target_memory_for_programming_file",
	  "target memory for HEX programming file",
	  false,
	  "",
	  "flash | internal | external"
	);
	cmd.add(targetMemoryArg);

	TCLAP::ValueArg<std::string> iqrfProgFileArg(
	  "q",
	  "iqrf_programming_file",
	  "program TR with IQRF programming file",
	  false,
	  "",
	  "string"
	);
	cmd.add(iqrfProgFileArg);


	// Parse the argv array.
	cmd.parse(argc, argv);

	// Get the value parsed by each arg. 
	std::string iface = ifaceArg.getValue();
	if ( !iface.empty() ) {
	  interface = iface;
	  isInterface = true;
	}

	std::string ifaceDevice = ifaceDeviceArg.getValue();
	if ( !ifaceDevice.empty() ) {
	  dev = ifaceDevice;
	  isDev = true;
	}

	std::string configFile = configFileArg.getValue();
	if ( !configFile.empty() ) {
	  trconf = configFile;
	  isTrconf = true;
	}

	std::string hexProgFile = hexProgFileArg.getValue();
	if ( !hexProgFile.empty() ) {
	  hex = hexProgFile;
	  isHex = true;
	}

	std::string targetMemory = targetMemoryArg.getValue();
	if ( !targetMemory.empty() ) {
	  target = parseTarget(targetMemory);
	  if (target == TrMemory::ERROR) {
		isTarget = false;
	  }
	  else {
		isTarget = true;
	  }
	}

	std::string iqrfProgFile = iqrfProgFileArg.getValue();
	if ( !iqrfProgFile.empty() ) {
	  iqrf = iqrfProgFile;
	  isIqrf = true;
	}

  } catch (TCLAP::ArgException &e) {
	  std::cerr << "Error while parsing commandline parameters!\n";
	  valid = false;
	  return;
  }

  parsed = true;

  if (!(isHex || isIqrf || isTrconf)) {
	std::cerr << "No configuration file specified!\n";
	valid = false;
  }

  if (isTarget && !isHex) {
	std::cerr << "Configuration option -t has no effect without command line option -p!\n";
  }

  if (isHex && !isTarget) {
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


#else
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
            if (optopt == 'i' || optopt == 'd' || optopt == 'c' || optopt == 'p' || optopt == 't' || optopt == 'q') {
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
    if (!(isHex || isIqrf || isTrconf)) {
        std::cerr << "No configuration file specified!\n";
        valid = false;
    }
    
    if (isTarget && !isHex) {
        std::cerr << "Configuration option -t has no effect without command line option -p!\n";
    }
    
    if (isHex &&  !isTarget) {
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
#endif


Commands::Commands(int argc, char * argv[]) {
    isHex = false;
    isIqrf = false;
    isTrconf = false;
    valid = false;
    parsed = false;
    
    parse(argc, argv);
}

bool Commands::isValid(void) {
    if (parsed && valid) {
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
    if (isValid() && isTrconf) {
        return true;
    } else {
        return false;
    }
}

std::string Commands::getTrconf(void) {
    if (isValid() && isTrconf) {
        return trconf;
    } else {
        throw std::runtime_error("Can not get nonexistent file name of TRCONF file!");
    }
}

bool Commands::programHex(void) {
    if (isValid() && isHex) {
        return true;
    } else {
        return false;
    }
}

std::string Commands::getHex(void) {
    if (isValid() && isHex) {
        return hex;
    } else {
        throw std::runtime_error("Can not get nonexistent file name of HEX file!");
    }
}

TrMemory Commands::getTarget(void) {
    if (isValid() && isHex) {
        return target;
    } else {
        throw std::runtime_error("Can not get nonexistent target memory for HEX file!");
    }
}

bool Commands::programIqrf(void) {
    if (isValid() && isIqrf) {
        return true;
    } else {
        return false;
    }
}

std::string Commands::getIqrf(void) {
    if (isValid() && isIqrf) {
        return iqrf;
    } else {
        throw std::runtime_error("Can not get nonexistent file name of IQRF file!");
    }
}
