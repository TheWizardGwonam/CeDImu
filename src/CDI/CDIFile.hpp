#ifndef CDIFILE_HPP
#define CDIFILE_HPP

class CDIFile;

#include <string>

#include "CDIDisk.hpp"

class CDIFile
{
    CDIDisk& disk;

public:
    const uint32_t fileLBN;
    const uint32_t filesize;
    const uint8_t nameSize;
    const std::string filename;
    const uint16_t attributes;
    const uint8_t fileNumber;
    const uint16_t parent;

    CDIFile() = delete;
    CDIFile(CDIDisk& cdidisk, uint32_t lbn, uint32_t filesize, uint8_t namesize, std::string name, uint16_t attr, uint8_t fileNumber, uint16_t parentRelpos);

    void ExportAudio(std::string directoryPath);
    void ExportFile(std::string directoryPath);
    char* GetFileContent(uint32_t& size);
};

#endif // CDIFILE_HPP
