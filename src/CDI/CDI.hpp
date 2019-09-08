#ifndef CDI_HPP
#define CDI_HPP

class CDI;

#include <fstream>
#include <string>

#include "CDIFile.hpp"
#include "CDIDirectory.hpp"
#include "../CeDImu.hpp"

typedef struct
{
    uint8_t Minutes;
    uint8_t Seconds;
    uint8_t Sectors;
    uint8_t Mode; // should be 2 for CD-I tracks
} CDIHeader;

typedef struct
{
    uint8_t FileNumber;
    uint8_t ChannelNumber;
    uint8_t Submode;
    uint8_t CodingInformation;
} CDISubheader;

enum SubmodeBits
{
    cdieof  = 0b10000000, // End of File
    cdirt   = 0b01000000, // Real Time
    cdiform = 0b00100000, // Form
    cditr   = 0b00010000, // Trigger
    cdid    = 0b00001000, // Data
    cdia    = 0b00000100, // Audio
    cdiv    = 0b00000010, // Video
    cdieor  = 0b00000001, // End of Record
};

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
    bool romOpened;

    CDI(CeDImu* appp);
    ~CDI();

    bool OpenROM(std::string rom);
    void LoadFiles();
    bool CloseROM();
    bool CreateSubfoldersFromROMDirectory(std::string path = "");
    bool LoadModuleInMemory(std::string moduleName, uint32_t address);

    uint16_t GetWord(const uint32_t& addr, bool stay = false);
    uint16_t GetNextWord();

    // Export functions
    bool ExportFiles();
    void ExportFilesInfo();
    bool ExportAudio();
    void ExportAudioInfo();
    void ExportSectorsInfo();

    bool SetDiskPosition(const uint32_t& pos);

    void UpdateSectorInfo();
    bool CheckPosition();
    bool GotoNextSector(uint8_t submodeMask = 0, bool includingCurrentSector = false);
    bool GotoLBN(uint32_t lbn, uint32_t offset = 0);
    bool IsEmptySector();

    CeDImu* app;
    CDIHeader header;
    CDISubheader subheader;

    std::ifstream disk;
    std::string romPath;
    std::string romName;
    std::string gameName;
    std::string gameFolder; // romPath + gameName + "/"
    CDIDirectory rootDirectory;
    CDIFile mainModule;
};

#endif // CDI_HPP
