#ifndef CDI_HPP
#define CDI_HPP

class CDI;

#include <string>

#include "CDIDisk.hpp"
#include "CDIFile.hpp"
#include "CDIDirectory.hpp"
#include "../CeDImu.hpp"

enum AudioCodingInformation
{
    emphasis = 0b01000000,
    bps      = 0b00110000, // bits per sample
    sf       = 0b00001100, // sampling frequency
    ms       = 0b00000011  // mono/stereo
};

class CDI
{
public:
    CeDImu* cedimu;
    CDIDisk disk;
    CDIFile mainModule;
    CDIDirectory rootDirectory;
    std::string gameName;
    std::string romPath;
    std::string gameFolder; // romPath + gameName + "/"

    CDI(CeDImu* app);
    ~CDI();

    bool OpenROM(const std::string rom);
    void CloseROM();
    void LoadFileTree();
    bool CreateSubfoldersFromROMDirectory(std::string path = "");
    bool LoadModuleInMemory(std::string moduleName, uint32_t address);

    bool ExportFiles();
    void ExportFilesInfo();
    bool ExportAudio();
    void ExportAudioInfo();
    void ExportSectorsInfo();
};

#endif // CDI_HPP
