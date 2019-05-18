#include "SCC68070.hpp"

SCC68070::SCC68070(CeDImu& cedimu, VDSC& gpu) : app(cedimu), vdsc(gpu), instructionsBuffer()
{
    Execute = Interpreter;
    internal = new int8_t[0xBFFFFFFF-INTERNAL];
    count = 0;
}

void SCC68070::RebootCore()
{
    SetS();
    executionTime = 0;
    for(uint8_t i = 0; i < 8; i++)
    {
        D[i] = 0;
        A[i] = 0;
    }
    instructionsBuffer.clear();
    InitSSPPC();
    instructionsBufferChanged = true;
    stop = false;
}

void SCC68070::InitSSPPC()
{
    vdsc.MemorySwap();
    SSP = vdsc.GetLong(0);
    A[7] = SSP;
    PC = vdsc.GetLong(4);
}

void SCC68070::SingleStep()
{
    (this->*Execute)();
}

void SCC68070::Run()
{
    while(run)
    {
        (this->*Execute)();
    }
}

uint16_t SCC68070::GetNextWord()
{
    uint16_t opcode = vdsc.GetWord(PC);
    PC += 2;
    return opcode;
}

void SCC68070::SetCCR(const uint8_t X, const uint8_t N, const uint8_t Z, const uint8_t V, const uint8_t C) // use the define UNCHANGED to not change the value of a bit
{
    if(X != UNCHANGED)
        SetX(X);
    if(N != UNCHANGED)
        SetN(N);
    if(Z != UNCHANGED)
        SetZ(Z);
    if(V != UNCHANGED)
        SetV(V);
    if(C != UNCHANGED)
        SetC(C);
}

void SCC68070::SetXC(const uint8_t XC)
{
    SetX(XC);
    SetC(XC);
}

void SCC68070::SetVC(const uint8_t VC)
{
    SetV(VC);
    SetC(VC);
}

void SCC68070::SetX(const uint8_t X)
{
    SR &= 0b1111111111101111;
    SR |= (X << 4);
}

uint8_t SCC68070::GetX()
{
    return (SR & 0b0000000000010000) >> 4;
}

void SCC68070::SetN(const uint8_t N)
{
    SR &= 0b1111111111110111;
    SR |= (N << 3);
}

uint8_t SCC68070::GetN()
{
    return (SR & 0b0000000000001000) >> 3;
}

void SCC68070::SetZ(const uint8_t Z)
{
    SR &= 0b1111111111111011;
    SR |= (Z << 2);
}

uint8_t SCC68070::GetZ()
{
    return (SR & 0b0000000000000100) >> 2;
}

void SCC68070::SetV(const uint8_t V)
{
    SR &= 0b1111111111111101;
    SR |= (V << 1);
}

uint8_t SCC68070::GetV()
{
    return (SR & 0b0000000000000010) >> 1;
}

void SCC68070::SetC(const uint8_t C)
{
    SR &= 0b1111111111111110;
    SR |= C;
}

uint8_t SCC68070::GetC()
{
    return SR & 0b0000000000000001;
}

void SCC68070::SetS(const uint8_t S)
{
    if(S == GetS()) return; // From Bizhawk
    if(S) // entering supervisor mode
    {
        USP = A[7];
        A[7] = SSP;
    }
    else
    { // exiting supervisor mode
        SSP = A[7];
        A[7] = USP;
    }
    SR &= 0b1101111111111111;
    SR |= (S << 13);
}

uint8_t SCC68070::GetS()
{
    return (SR & 0b0010000000000000) >> 13;
}
