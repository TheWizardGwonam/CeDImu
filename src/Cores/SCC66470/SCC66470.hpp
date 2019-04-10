#ifndef SCC66470_HPP
#define SCC66470_HPP

class SCC66470;

#include <cstdint>
#include <string>

#include "../VDSC.hpp"

class SCC66470 : public VDSC
{
public:
    SCC66470();
    ~SCC66470();

    virtual bool LoadBIOS(std::string filename) override;
    virtual void PutDataInMemory(const uint8_t* s, unsigned int size, unsigned int position) override;
    virtual void ResetMemory() override;
    virtual void MemorySwap() override;

    virtual uint8_t  GetByte(const uint32_t& addr) const override;
    virtual uint16_t GetWord(const uint32_t& addr) override;
    virtual uint32_t GetLong(const uint32_t& addr) override;
    virtual void SetByte(const uint32_t& addr, const uint8_t& data) override;
    virtual void SetWord(const uint32_t& addr, const uint16_t& data) override;
    virtual void SetLong(const uint32_t& addr, const uint32_t& data) override;

    void DisplayLine() override;
};

#endif // SCC66470_HPP
