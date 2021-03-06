#ifndef CDI_HPP
#define CDI_HPP

class CDI;

#include <string>

#include "CDIDisk.hpp"
#include "CDIFile.hpp"
#include "CDIDirectory.hpp"

class CDI
{
    CDIDisk disk;
    CDIDirectory rootDirectory;

    void LoadCDIFileSystem();
    bool CreateSubfoldersFromROMDirectory(std::string path = "");

public:
    std::string mainModule;
    std::string gameName;
    std::string romPath;
    std::string gameFolder; // romPath + gameName + "/"

    CDI();
    ~CDI();

    bool OpenROM(const std::string& rom);
    void CloseROM();
    CDIFile* GetFile(std::string path);

    bool ExportAudio();
    bool ExportFiles();
    void ExportFilesInfo();
    bool ExportVideo();
    void ExportSectorsInfo();
};

#endif // CDI_HPP
