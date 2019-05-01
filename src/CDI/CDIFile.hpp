#ifndef CDIFILE_HPP
#define CDIFILE_HPP

struct CDIFile;

#include <string>

struct CDIFile
{
    uint8_t nameSize;
    uint8_t fileNumber;
    uint16_t attributes;
    uint16_t parent;
    uint32_t LBN;
    uint32_t size;
    std::string name;

    CDIFile(uint32_t lbn, uint32_t filesize, uint8_t namesize, std::string filename, uint16_t attr, uint8_t fileNumber, uint16_t parentRelpos);
};

#endif // CDIFILE_HPP