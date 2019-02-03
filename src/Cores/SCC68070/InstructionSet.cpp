#include "SCC68070.hpp"

#include "../../utils.hpp"
#include <wx/msgdlg.h>

void SCC68070::Exception(const uint8_t& vectorNumber, uint16_t& calcTime)
{
    uint16_t sr = SR;
    SetS();

    if(vectorNumber == 3) // TODO: implement long Stack format (please fix it)
    {
        int32_t last = lastAddress;
        SetWord(ARIWPr(7, 2), 0); // internal information
        SetWord(ARIWPr(7, 2), currentOpcode); // IRC
        SetWord(ARIWPr(7, 2), currentOpcode); // IR
        SetLong(ARIWPr(7, 4), 0); // DBIN
        SetLong(ARIWPr(7, 4), last); // TPF
        SetLong(ARIWPr(7, 4), 0); // TPD
        SetWord(ARIWPr(7, 2), 0); // internal information
        SetWord(ARIWPr(7, 2), 0); // internal information
        SetWord(ARIWPr(7, 2), 0); // MM
        SetWord(ARIWPr(7, 2), 0); // SSW
        SetWord(ARIWPr(7, 2), 0xF000 | ((uint16_t)vectorNumber << 2));
    }
    else
        SetWord(ARIWPr(7, 2), (uint16_t)vectorNumber << 2);

    SetLong(ARIWPr(7, 4), PC);
    SetWord(ARIWPr(7, 2), sr);

    switch(vectorNumber) // handle Exception Processing Clock Periods
    {
    case 2: case 3:
        calcTime += 158; break;
    case 4: case 8: case 9:
        calcTime += 55; break;
    case 32: case 33: case 34: case 35: case 36: case 37: case 38: case 39:
    case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47:
        calcTime += 52; break;
    case 5:
        calcTime += 64; break;
    default:
        if(vectorNumber == 15 || (vectorNumber >= 24 && vectorNumber < 32) || vectorNumber >= 57)
            calcTime += 65;
    break;
    }

    PC = GetLong(vectorNumber * 4);
    SetS(0);
}

uint16_t SCC68070::UnknownInstruction()
{
    instructionsBuffer += "Unknown instruction;\n";
    return 0;
}

uint16_t SCC68070::Abcd()
{
    uint8_t Rx = (currentOpcode & 0x0E00) >> 9;
    uint8_t Ry = (currentOpcode & 0x0007);
    uint8_t result;
    uint16_t calcTime;
    uint8_t x = GetX();

    if(currentOpcode & 0x0008) // R/M = 1 : Memory to Memory
    {
        uint8_t src = GetByte(ARIWPr(Ry, 1));
        uint8_t dst = GetByte(ARIWPr(Rx, 1));
        if((src & 0x0F) + (dst & 0x0F) > 9)
            SetXC();
        if(((src & 0xF0) >> 4) + ((dst & 0xF0) >> 4) > 9)
            SetXC();

        result = convertPBCD(src) + convertPBCD(dst) + x;
        SetByte(lastAddress, result);
        calcTime = 31;
    }
    else // R/M = 0 : Data register to Data register
    {
        uint8_t src = D[Ry] & 0x000000FF;
        uint8_t dst = D[Rx] & 0x000000FF;
        if((src & 0x0F) + (dst & 0x0F) > 9)
            SetXC();
        if(((src & 0xF0) >> 4) + ((dst & 0xF0) >> 4) > 9)
            SetXC();

        result = convertPBCD(src) + convertPBCD(dst) + x;
        D[Rx] &= 0xFFFFFF00;
        D[Rx] |= result;
        calcTime = 10;
    }

    if(result != 0)
        SetZ(0);

    return calcTime;
}

uint16_t SCC68070::Add()
{
    uint8_t REGISTER = (currentOpcode & 0x0E00) >> 9;
    uint8_t   OPMODE = (currentOpcode & 0x01C0) >> 6;
    uint8_t   EAMODE = (currentOpcode & 0x0038) >> 3;
    uint8_t    EAREG = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(OPMODE == 0)
    {
        int8_t src = GetByte(EAMODE, EAREG, calcTime);
        int8_t dst = D[REGISTER] & 0x000000FF;
        int16_t res = src + dst;
        uint16_t ures = (uint8_t)src + (uint8_t)dst;

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80) SetN();
        else SetN(0);

        D[REGISTER] &= 0xFFFFFF00;
        D[REGISTER] |= (res & 0xFF);

    }
    else if(OPMODE == 1)
    {
        int16_t src = GetWord(EAMODE, EAREG, calcTime);
        int16_t dst = D[REGISTER] & 0x0000FFFF;
        int32_t res = src + dst;
        uint32_t ures = (uint16_t)src + (uint16_t)dst;

        if(ures & 0x10000) SetXC();
        else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x8000) SetN();
        else SetN(0);

        D[REGISTER] &= 0xFFFF0000;
        D[REGISTER] |= (res & 0xFFFF);
    }
    else if(OPMODE == 2)
    {
        int32_t src = GetLong(EAMODE, EAREG, calcTime);
        int32_t dst = D[REGISTER];
        int64_t res = src + dst;
        uint64_t ures = (uint32_t)src + (uint32_t)dst;

        if(ures & 0x100000000) SetXC();
        else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80000000) SetN();
        else SetN(0);

        D[REGISTER] = res;
    }
    else if(OPMODE == 4)
    {
        int8_t src = D[REGISTER] & 0x000000FF;
        int8_t dst = GetByte(EAMODE, EAREG, calcTime);
        int16_t res = src + dst;
        uint16_t ures = (uint8_t)src + (uint8_t)dst;

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80) SetN();
        else SetN(0);

        SetByte(lastAddress, res);

        calcTime += 4;
    }
    else if(OPMODE == 5)
    {
        int16_t src = D[REGISTER] & 0x0000FFFF;
        int16_t dst = GetWord(EAMODE, EAREG, calcTime);
        int32_t res = src + dst;
        uint32_t ures = (uint16_t)src + (uint16_t)dst;

        if(ures & 0x10000) SetXC();
        else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x8000) SetN();
        else SetN(0);

        SetWord(lastAddress, res);

        calcTime += 4;
    }
    else
    {
        int32_t src = D[REGISTER];
        int32_t dst = GetLong(EAMODE, EAREG, calcTime);
        int64_t res = src + dst;
        uint64_t ures = (uint32_t)src + (uint32_t)dst;

        if(ures & 0x100000000) SetXC();
        else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80000000) SetN();
        else SetN(0);

        SetLong(lastAddress, res);

        calcTime += 8;
    }

    return calcTime;
}

uint16_t SCC68070::Adda()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t   size = (currentOpcode & 0x0100) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size) // Long
    {
        int32_t src = GetLong(eamode, eareg, calcTime);
        A[reg] += src;
    }
    else // Word
    {
        int16_t src = GetWord(eamode, eareg, calcTime);
        int16_t dst = A[reg] & 0x0000FFFF;
        A[reg] = src + dst;
    }

    return calcTime;
}

uint16_t SCC68070::Addi()
{
    uint8_t   size = (currentOpcode & 0b0000000011000000) >> 6;
    uint8_t eamode = (currentOpcode & 0b0000000000111000) >> 3;
    uint8_t  eareg = (currentOpcode & 0b0000000000000111);
    uint16_t calcTime = 14;

    if(size == 0) // Byte
    {
        int8_t src = GetNextWord() & 0x00FF;
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int16_t res = src + dst;
        uint16_t ures = (uint8_t)src + (uint8_t)dst;

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= (res & 0xFF); }
        else
        {   SetByte(lastAddress, res); calcTime += 4; }
    }
    else if(size == 1) // Word
    {
        int16_t src = GetNextWord();
        int16_t dst = GetWord(eamode, eareg, calcTime);
        int32_t res = src + dst;
        uint32_t ures = (uint16_t)src + (uint16_t)dst;

        if(ures & 0x10000) SetXC();
        else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x8000) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= (res & 0xFFFF); }
        else
        {   SetWord(lastAddress, res); calcTime += 4; }
    }
    else if(size == 2) // Long
    {
        int32_t src = (GetNextWord() << 16) | GetNextWord();
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int64_t res = src + dst;
        uint64_t ures = (uint32_t)src + (uint32_t)dst;

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] = res; calcTime += 4; }
        else
        {   SetLong(lastAddress, res); calcTime += 12; }
    }

    instructionsBuffer += "ADDI;\n";

    return calcTime;
}

uint16_t SCC68070::Addq()
{
    uint8_t   doto = (currentOpcode & 0x0E00) >> 9;
    uint8_t   data = doto ? doto : 8;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(eamode == 1)
    {
        if(size == 1)
        {
            int16_t dst = A[eareg] & 0x0000FFFF;
            A[eareg] = data + dst;
        }
        else
            A[eareg] += data;
    }
    else if(size == 0)
    {
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int16_t res = data + dst;
        uint16_t ures = data + (uint8_t)dst;

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= (res & 0xFF); }
        else
        {   SetByte(lastAddress, res); calcTime += 4; }
    }
    else if(size == 1)
    {
        int16_t dst = GetWord(eamode, eareg, calcTime);
        int32_t res = data + dst;
        uint32_t ures = data + (uint16_t)dst;

        if(ures & 0x10000) SetXC();
        else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x8000) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= (res & 0xFFFF); }
        else
        {   SetByte(lastAddress, res); calcTime += 4; }
    }
    else
    {
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int64_t res = data + dst;
        uint64_t ures = data + (uint32_t)dst;

        if(ures & 0x100000000) SetXC();
        else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(eamode == 0)
            D[eareg] = res;
        else
        {   SetByte(lastAddress, res); calcTime += 8; }
    }

    instructionsBuffer += "ADDQ; ";

    return calcTime;
}

uint16_t SCC68070::Addx()
{
    uint8_t   Rx = (currentOpcode & 0x0E00) >> 9;
    uint8_t   Ry = (currentOpcode & 0x0007);
    uint8_t size = (currentOpcode & 0x00C0) >> 6;
    uint8_t   rm = (currentOpcode & 0x0008) >> 3;
    uint16_t calcTime = 7;

    if(size == 0)
    {
        int8_t src; int8_t dst;
        if(rm)
        {   src = GetByte(ARIWPr(Ry, 1));
            dst = GetByte(ARIWPr(Rx, 1)); calcTime += 2 * ITARIWPrBW + 21; }
        else
        {   src = D[Ry] & 0x000000FF;
            dst = D[Rx] & 0x000000FF; }

        int16_t res = src + dst + GetX();
        uint16_t ures = (uint8_t)src + (uint8_t)dst + GetX();

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80) SetN();
        else SetN(0);

        if(rm)
            SetByte(lastAddress, res);
        else
        {   D[Rx] &= 0xFFFFFF00; D[Rx] |= (res & 0xFF); }
    }
    else if(size == 1)
    {
        int16_t src; int16_t dst;
        if(rm)
        {   src = GetWord(ARIWPr(Ry, 2));
            dst = GetWord(ARIWPr(Rx, 2)); calcTime += 2 * ITARIWPrBW + 21; }
        else
        {   src = D[Ry] & 0x0000FFFF;
            dst = D[Rx] & 0x0000FFFF; }

        int32_t res = src + dst + GetX();
        uint32_t ures = (uint16_t)src + (uint16_t)dst + GetX();

        if(ures & 0x10000) SetXC();
        else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x8000) SetN();
        else SetN(0);

        if(rm)
            SetWord(lastAddress, res);
        else
        {   D[Rx] &= 0xFFFF0000; D[Rx] |= (res & 0xFFFF); }
    }
    else
    {
        int32_t src; int32_t dst;
        if(rm)
        {   src = GetLong(ARIWPr(Ry, 4));
            dst = GetLong(ARIWPr(Rx, 4)); calcTime += 2 * ITARIWPrL + 33; }
        else
        {   src = D[Ry];
            dst = D[Rx]; }

        int64_t res = src + dst + GetX();
        uint64_t ures = (uint32_t)src + (uint32_t)dst + GetX();

        if(ures & 0x100000000) SetXC();
        else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(rm)
            SetLong(lastAddress, res);
        else
            D[Rx] = res;
    }

    return calcTime;
}

uint16_t SCC68070::And()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(opmode == 0)
    {
        uint8_t src = GetByte(eamode, eareg, calcTime);
        uint8_t dst = D[reg] & 0x000000FF;
        uint8_t res = src & dst;

        if(res & 0x80) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        D[reg] &= 0xFFFFFF00;
        D[reg] |= res;
    }
    else if(opmode == 1)
    {
        uint16_t src = GetWord(eamode, eareg, calcTime);
        uint16_t dst = D[reg] & 0x0000FFFF;
        uint16_t res = src & dst;

        if(res & 0x8000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        D[reg] &= 0xFFFF0000;
        D[reg] |= res;
    }
    else if(opmode == 2)
    {
        uint32_t src = GetLong(eamode, eareg, calcTime);
        uint32_t dst = D[reg];
        uint32_t res = src & dst;

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        D[reg] = res;
    }
    else if(opmode == 3)
    {
        uint8_t src = D[reg] & 0x000000FF;
        uint8_t dst = GetByte(eamode, eareg, calcTime);
        uint8_t res = src & dst;

        if(res & 0x80) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        SetByte(lastAddress, res);

        calcTime += 4;
    }
    else if(opmode == 4)
    {
        uint16_t src = D[reg] & 0x0000FFFF;
        uint16_t dst = GetWord(eamode, eareg, calcTime);
        uint16_t res = src & dst;

        if(res & 0x8000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        SetWord(lastAddress, res);

        calcTime += 4;
    }
    else
    {
        uint32_t src = D[reg];
        uint32_t dst = GetLong(eamode, eareg, calcTime);
        uint32_t res = src & dst;

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        SetLong(lastAddress, res);

        calcTime += 8;
    }

    SetC(0);
    SetV(0);
    return calcTime;
}

uint16_t SCC68070::Andi()
{
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;

    if(size == 0)
    {
        uint8_t data = GetNextWord() & 0xFF;
        uint8_t  dst = GetByte(eamode, eareg, calcTime);
        uint8_t  res = data & dst;

        if(res & 0x80) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(eamode)
        {   SetByte(lastAddress, res); calcTime += 4; }
        else
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= res; }
    }
    else if(size == 1)
    {
        uint16_t data = GetNextWord();
        uint16_t  dst = GetWord(eamode, eareg, calcTime);
        uint16_t  res = data & dst;

        if(res & 0x8000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(eamode)
        {   SetWord(lastAddress, res); calcTime += 4; }
        else
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= res; }
    }
    else
    {
        uint32_t data = GetNextWord();
        uint32_t  dst = GetWord(eamode, eareg, calcTime);
        uint32_t  res = data & dst;

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(eamode)
        {   SetWord(lastAddress, res); calcTime += 10; }
        else
        {   D[eareg] = res; calcTime += 4; }
    }

    SetV(0);
    SetC(0);
    return calcTime;
}

uint16_t SCC68070::AsM()
{
    uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint16_t data = GetWord(eamode, eareg, calcTime);

    bool b = false;
    if(dr) // left
    {
        uint8_t a = data & 0x8000;
        SetXC(a);
        data <<= 1;
        if(a != (data & 0x8000))
            b = true;
        instructionsBuffer += "ASL;\n ";
    }
    else // right
    {
        uint16_t msb = data & 0x8000;
        uint16_t a = data & 0x0001;
        SetXC(a);
        data >>= 1;
        data |= msb;
        instructionsBuffer += "ASR;\n ";
    }

    if(data & 0x8000) SetN(); else SetN(0);
    if(data == 0) SetZ(); else SetZ(0);
    if(b) SetV(); else SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::AsR()
{
    uint8_t count = (currentOpcode & 0x0E00) >> 9;
    uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    uint8_t   reg = (currentOpcode & 0x0007);
    uint8_t shift;
    if(ir)
        shift = D[reg] % 64;
    else
        shift = (count) ? count : 8;

    if(!shift)
    {
        SetV(0);
        SetC(0);
        return 13;
    }

    bool b = false;
    if(size == 0) // byte
    {
        uint8_t data = D[reg] & 0x000000FF;
        if(dr)
        {
            uint8_t old = data & 0x80;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x80;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            uint8_t msb = data & 0x80;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x01;
                SetXC(a);
                data >>= 1;
                data |= msb;
            }
        }

        if(data & 0x80) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] &= 0xFFFFFF00;
        D[reg] |= data;
    }
    else if(size == 1) // word
    {
        uint16_t data = D[reg] & 0x0000FFFF;
        if(dr)
        {
            uint16_t old = data & 0x8000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint16_t a = data & 0x8000;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            uint16_t msb = data & 0x8000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint16_t a = data & 0x0001;
                SetXC(a);
                data >>= 1;
                data |= msb;
            }
        }

        if(data & 0x8000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] &= 0xFFFF0000;
        D[reg] |= data;
    }
    else // long
    {
        uint32_t data = D[reg];
        if(dr)
        {
            uint32_t old = data & 0x80000000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint32_t a = data & 0x80000000;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            uint32_t msb = data & 0x80000000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint32_t a = data & 0x00000001;
                SetXC(a);
                data >>= 1;
                data |= msb;
            }
        }

        if(data & 0x80000000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] = data;
    }

    if(b)
        SetV();
    else
        SetV(0);

    if(dr)
        instructionsBuffer += "ASL;\n ";
    else
        instructionsBuffer += "ASR;\n ";

    return 13 + 3 * shift;
}

uint16_t SCC68070::BCC()
{
    uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    int8_t       disp = (currentOpcode & 0x00FF);
    uint16_t calcTime;

    if(disp) // 8-bit
    {
        calcTime = 13;
        if((this->*ConditionalTests[condition])())
            PC += disp;
    }
    else // 16 bit
    {
        calcTime = 14;
        if((this->*ConditionalTests[condition])())
            PC += (int16_t)GetNextWord();
    }

    instructionsBuffer += "BCC; ";

    return calcTime;
}

uint16_t SCC68070::Bchg()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;
    uint8_t shift;
    if(currentOpcode & 0x0100)
    {   shift = D[reg] % 32; calcTime = 10; }
    else
    {   shift = (GetNextWord() & 0x00FF) % 8; calcTime = 17; }

    if(eamode == 0)
    {
        uint32_t data = D[eareg];
        uint32_t mask = 1 << shift;
        if(data & mask)
        {
            SetZ(0);
            data &= ~(mask);
        }
        else
        {
            SetZ();
            data |= mask;
        }
        D[eareg] = data;
    }
    else
    {
        calcTime += 4;
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;
        if(data & mask)
        {
            SetZ(0);
            data &= ~(mask);
        }
        else
        {
            SetZ();
            data |= mask;
        }
        SetByte(lastAddress, data);
    }

    return calcTime;
}

uint16_t SCC68070::Bclr()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;
    uint8_t shift;
    if(currentOpcode & 0x0100)
    {   shift = D[reg] % 32; calcTime = 10; }
    else
    {   shift = (GetNextWord() & 0x00FF) % 8; calcTime = 17; }

    if(eamode == 0)
    {
        uint32_t data = D[eareg];
        uint32_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        data &= ~(mask);
        D[eareg] = data;
    }
    else
    {
        calcTime += 4;
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        data &= ~(mask);
        SetByte(lastAddress, data);
    }

    return calcTime;
}

uint16_t SCC68070::Bra()
{
    int8_t disp = currentOpcode & 0x00FF;
    uint16_t calcTime;

    if(disp)
    {
        PC += disp;
        calcTime = 13;
    }
    else
    {
        PC += (int16_t)GetNextWord();
        calcTime = 14;
    }

    instructionsBuffer += "BRA; ";

    return calcTime;
}

uint16_t SCC68070::Bset()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;
    uint8_t shift;
    if(currentOpcode & 0x0100)
    {   shift = D[reg] % 32; calcTime = 10; }
    else
    {   shift = (GetNextWord() & 0x00FF) % 8; calcTime = 17; }

    if(eamode == 0)
    {
        uint32_t data = D[eareg];
        uint32_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        data |= mask;
        D[eareg] = data;
    }
    else
    {
        calcTime += 4;
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        data |= mask;
        SetByte(lastAddress, data);
    }

    return calcTime;
}

uint16_t SCC68070::Bsr()
{
    int8_t disp = currentOpcode & 0x00FF;
    uint16_t calcTime;

    if(disp)
    {
        SetLong(ARIWPr(7, 4), PC);
        PC += disp;
        calcTime = 21;
    }
    else
    {
        int16_t dsp = GetNextWord();
        SetLong(ARIWPr(7, 4), PC);
        PC += dsp - 2;
        calcTime = 25;
    }

    instructionsBuffer += "BSR; ";

    return calcTime;
}

uint16_t SCC68070::Btst()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;
    uint8_t shift;
    if(currentOpcode & 0x0100)
    {   shift = D[reg] % 32; calcTime = 7; }
    else
    {   shift = (GetNextWord() & 0x00FF) % 8; calcTime = 14; }

    if(eamode == 0)
    {
        uint32_t data = D[eareg];
        uint32_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        D[eareg] = data;
    }
    else
    {
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        SetByte(lastAddress, data);
    }

    return calcTime;
}

uint16_t SCC68070::Chk()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 19;

    int16_t source = GetWord(eamode, eareg, calcTime);
    int16_t data = D[reg] & 0xFFFF;

    if(data < 0 || data > source)
    {
        calcTime += 45;
        Exception(6, calcTime);
        if(data < 0) SetN();
        if(data > source) SetN(0);
    }

    return calcTime;
}

uint16_t SCC68070::Clr()
{


    return 0;
}

uint16_t SCC68070::Cmp()
{


    return 0;
}

uint16_t SCC68070::Cmpa()
{


    return 0;
}

uint16_t SCC68070::Cmpi()
{


    return 0;
}

uint16_t SCC68070::Cmpm()
{
    uint8_t size = (currentOpcode & 0x00C0) >> 6;
    uint8_t Ax = (currentOpcode & 0x0E00) >> 9;
    uint8_t Ay = (currentOpcode & 0x0007);

    if(size == 0) // byte
    {
        int8_t src = GetByte(ARIWPo(Ay, 1));
        int8_t dst = GetByte(ARIWPo(Ax, 1));
        int16_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res < 0) SetN(); else SetZ(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if((dst < src) ^ (((dst ^ src) >= 0) == false)) SetC(); else SetC(0); // From bizhawk : C = ((a < b) ^ ((a ^ b) >= 0) == false);
    }
    else if(size == 1) // word
    {
        int16_t src = GetWord(ARIWPo(Ay, 2));
        int16_t dst = GetWord(ARIWPo(Ax, 2));
        int32_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res < 0) SetN(); else SetZ(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if((dst < src) ^ (((dst ^ src) >= 0) == false)) SetC(); else SetC(0);
    }
    else // long
    {
        int32_t src = GetLong(ARIWPo(Ay, 4));
        int32_t dst = GetLong(ARIWPo(Ax, 4));
        int64_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res < 0) SetN(); else SetZ(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if((dst < src) ^ (((dst ^ src) >= 0) == false)) SetC(); else SetC(0);
    }

    return (size == 2) ? 26 : 18;
}

uint16_t SCC68070::DbCC()
{
    uint8_t condition = (currentOpcode & 0x0F00) >> 12;
    uint8_t reg = (currentOpcode & 0x0007);
    int16_t disp = GetNextWord();

    if((this->*ConditionalTests[condition])())
       return 14;

    int16_t data = D[reg] & 0x0000FFFF;
    if(--data == -1)
        return 17;

    PC += signExtend16(disp) - 2;

    return 17;
}

uint16_t SCC68070::Divs()
{


    return 0;
}

uint16_t SCC68070::Divu()
{


    return 0;
}

uint16_t SCC68070::Eor()
{


    return 0;
}

uint16_t SCC68070::Eori()
{


    return 0;
}

uint16_t SCC68070::Exg()
{
    uint8_t   Rx = (currentOpcode & 0x0E00) >> 9;
    uint8_t mode = (currentOpcode & 0x00F8) >> 3;
    uint8_t   Ry = (currentOpcode & 0x0007);

    uint32_t tmp;

    if(mode == 0x08)
    {
        tmp = D[Rx];
        D[Rx] = D[Ry];
        D[Ry] = tmp;
    }
    else if(mode == 0x09)
    {
        tmp = A[Rx];
        A[Rx] = A[Ry];
        A[Ry] = tmp;
    }
    else
    {
        tmp = D[Rx];
        D[Rx] = A[Ry];
        A[Ry] = tmp;
    }

    return 13;
}

uint16_t SCC68070::Ext()
{
    uint8_t mode = (currentOpcode & 0x01C0) >> 6;
    uint8_t  reg = (currentOpcode & 0x0007);

    if(mode == 2) // byte to word
    {
        uint16_t tmp = signExtend816(D[reg] & 0x000000FF);
        D[reg] &= 0xFFFF0000;
        D[reg] |= tmp;
    }
    else if(mode == 3) // word to long
    {
        D[reg] = signExtend16(D[reg] & 0x0000FFFF);
    }
    else // byte to long
    {
        D[reg] = signExtend8(D[reg] & 0x000000FF);
    }

    SetVC(0);
    return 7;
}

uint16_t SCC68070::Jmp()
{
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;

    if(eamode == 2)
    {   PC = A[eareg]; calcTime = 7; }
    else if(eamode == 5)
    {   PC = ARIWD(eareg); calcTime = 14; }
    else if(eamode == 6)
    {   PC = ARIWI8(eareg); calcTime = 17; }
    else
    {
        if(eareg == 0)
        {   PC = ASA(); calcTime = 14; }
        else if(eareg == 1)
        {   PC = ALA(); calcTime = 18; }
        else if(eareg == 2)
        {   PC = PCIWD(); calcTime = 14; }
        else
        {   PC = PCIWI8(); calcTime = 17; }
    }

    instructionsBuffer += "JMP;";

    return calcTime;
}

uint16_t SCC68070::Jsr()
{
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint32_t pc;
    uint16_t calcTime;

    if(eamode == 2)
    {   pc = A[eareg]; calcTime = 18; }
    else if(eamode == 5)
    {   pc = ARIWD(eareg); calcTime = 25; }
    else if(eamode == 6)
    {   pc = ARIWI8(eareg); calcTime = 28; }
    else
    {
        if(eareg == 0)
        {   pc = ASA(); calcTime = 25; }
        else if(eareg == 1)
        {   pc = ALA(); calcTime = 29; }
        else if(eareg == 2)
        {   pc = PCIWD(); calcTime = 25; }
        else
        {   pc = PCIWI8(); calcTime = 28; }
    }

    SetLong(ARIWPr(7, 4), PC);
    PC = pc;

    return calcTime;
}

uint16_t SCC68070::Lea()
{


    return 0;
}

uint16_t SCC68070::Link()
{


    return 0;
}

uint16_t SCC68070::LsM()
{
    uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint16_t data = GetWord(eamode, eareg, calcTime);

    bool b = false;
    if(dr) // left
    {
        uint8_t a = data & 0x8000;
        SetXC(a);
        data <<= 1;
        if(a != (data & 0x8000))
            b = true;
        instructionsBuffer += "LSL;\n ";
    }
    else // right
    {
        uint8_t a = data & 0x0001;
        SetXC(a);
        data >>= 1;
        instructionsBuffer += "LSR;\n ";
    }

    if(data & 0x8000) SetN(); else SetN(0);
    if(data == 0) SetZ(); else SetZ(0);
    if(b) SetV(); else SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::LsR()
{
    uint8_t count = (currentOpcode & 0x0E00) >> 9;
    uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    uint8_t   reg = (currentOpcode & 0x0007);
    uint8_t shift;
    if(ir)
        shift = D[reg] % 64;
    else
        shift = (count) ? count : 8;

    if(!shift)
    {
        SetV(0);
        SetC(0);
        return 13;
    }

    bool b = false;
    if(size == 0) // byte
    {
        uint8_t data = D[reg] & 0x000000FF;
        if(dr)
        {
            uint8_t old = data & 0x80;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x80;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x01;
                SetXC(a);
                data >>= 1;
            }
        }

        if(data & 0x80) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] &= 0xFFFFFF00;
        D[reg] |= data;
    }
    else if(size == 1) // word
    {
        uint16_t data = D[reg] & 0x0000FFFF;
        if(dr)
        {
            uint8_t old = data & 0x8000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x8000;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x0001;
                SetXC(a);
                data >>= 1;
            }
        }

        if(data & 0x8000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] &= 0xFFFF0000;
        D[reg] |= data;
    }
    else // long
    {
        uint32_t data = D[reg];
        if(dr)
        {
            uint8_t old = data & 0x80000000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x80000000;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x00000001;
                SetXC(a);
                data >>= 1;
            }
        }

        if(data & 0x80000000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] = data;
    }

    if(b)
        SetV();
    else
        SetV(0);

    if(dr)
        instructionsBuffer += "LSL;\n ";
    else
        instructionsBuffer += "LSR;\n ";

    return 13 + 3 * shift;
}

uint16_t SCC68070::Move()
{
    uint8_t    size = (currentOpcode & 0x3000) >> 12;
    uint8_t  dstreg = (currentOpcode & 0x0E00) >> 9;
    uint8_t dstmode = (currentOpcode & 0x01C0) >> 6;
    uint8_t srcmode = (currentOpcode & 0x0038) >> 3;
    uint8_t  srcreg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 1) // byte
    {
        uint8_t src = GetByte(srcmode, srcreg, calcTime);
        if(src == 0) SetZ(); else SetZ(0);
        if(src & 0x80) SetN(); else SetN(0);
        SetByte(dstmode, dstreg, calcTime, src);
    }
    else if(size == 3) // word
    {
        uint16_t src = GetWord(srcmode, srcreg, calcTime);
        if(src == 0) SetZ(); else SetZ(0);
        if(src & 0x8000) SetN(); else SetN(0);
        SetWord(dstmode, dstreg, calcTime, src);
    }
    else // long
    {
        uint32_t src = GetLong(srcmode, srcreg, calcTime);
        if(src == 0) SetZ(); else SetZ(0);
        if(src & 0x80000000) SetN(); else SetN(0);
        SetLong(dstmode, dstreg, calcTime, src);
    }

    SetVC(0);

    instructionsBuffer += "MOVE;\n ";

    return calcTime;
}

uint16_t SCC68070::Moveccr()
{


    return 0;
}

uint16_t SCC68070::Movesr()
{


    return 0;
}

uint16_t SCC68070::MoveFsr()
{
#ifdef LOG_OPCODE
    std::cout << "MOVEfSR may not be used"; // According to the Green Book Chapter VI.2.2.2
#endif // LOG_OPCODE

    return 0;
}

uint16_t SCC68070::Moveusp()
{


    return 0;
}

uint16_t SCC68070::Movea()
{
    uint8_t   size = (currentOpcode & 0x3000) >> 12;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 3) // word
    {
        int16_t data = GetWord(eamode, eareg, calcTime);
        A[reg] = signExtend16(data);
    }
    else // long
    {
        A[reg] = GetWord(eamode, eareg, calcTime);
    }

    instructionsBuffer += "MOVEA;\n ";

    return calcTime;
}

uint16_t SCC68070::Movem()
{


    return 0;
}

uint16_t SCC68070::Movep()
{


    return 0;
}

uint16_t SCC68070::Moveq()
{


    return 0;
}

uint16_t SCC68070::Muls()
{


    return 0;
}

uint16_t SCC68070::Mulu()
{


    return 0;
}

uint16_t SCC68070::Nbcd()
{
    uint8_t mode = (currentOpcode & 0x0038) >> 3;
    uint8_t reg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint8_t x = GetX();
    uint8_t data = GetByte(mode, reg, calcTime);
    uint8_t result = 0 - convertPBCD(data);

    if(!(data & 0x0F))
        SetXC();

    result -= x;

    if(result != 0)
        SetZ(0);

    if(mode == 0)
    {
        D[reg] &= 0xFFFFFF00;
        D[reg] |= result;
        return 10;
    }
    SetByte(lastAddress, result);
    return calcTime - 4;
}

uint16_t SCC68070::Neg()
{
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 0;

    if(size == 0) // byte
    {
        int8_t data = GetByte(eamode, eareg, calcTime);

        int16_t res = 0 - data;
        uint16_t ures = (uint8_t)0 - (uint8_t)data;

        if(ures & 0x100) SetXC(); else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV(); else SetV(0);

        if(data == 0) SetZ(); else SetZ(0);
        if(data & 0x80) SetN(); else SetN(0);

        if(eamode)
        {
            SetByte(lastAddress, res);
        }
        else
        {
            D[eareg] &= 0xFFFFFF00;
            D[eareg] |= res & 0xFF;
        }
    }
    else if(size == 1) // word
    {
        int16_t data = GetWord(eamode, eareg, calcTime);

        int32_t res = 0 - data;
        uint32_t ures = (uint16_t)0 - (uint16_t)data;

        if(ures & 0x10000) SetXC(); else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV(); else SetV(0);

        if(data == 0) SetZ(); else SetZ(0);
        if(data & 0x8000) SetN(); else SetN(0);

        if(eamode)
        {
            SetWord(lastAddress, res);
        }
        else
        {
            D[eareg] &= 0xFFFF0000;
            D[eareg] |= res & 0xFFFF;
        }
    }
    else // long
    {
        int32_t data = GetLong(eamode, eareg, calcTime);

        int64_t res = 0 - data;
        uint64_t ures = (uint32_t)0 - (uint32_t)data;

        if(ures & 0x100000000) SetXC(); else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV(); else SetV(0);

        if(data == 0) SetZ(); else SetZ(0);
        if(data & 0x80000000) SetN(); else SetN(0);

        if(eamode)
        {
            SetLong(lastAddress, res);
        }
        else
        {
            D[eareg] = res;
        }
    }

    instructionsBuffer += "NEG;\n ";

    return calcTime;
}

uint16_t SCC68070::Negx()
{


    return 0;
}

uint16_t SCC68070::Nop()
{
    return 7; // I love this instruction :D
}

uint16_t SCC68070::Not() // ok
{
#ifdef LOG_OPCODE
    log("NOT ");
#endif // LOG_OPCODE
    uint8_t size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 0; // arbitrary, set as default

    if(size == 0) // Byte
    {
        int8_t data = GetByte(eamode,  eareg, calcTime);
        int8_t res = ~data;
        SetByte(lastAddress, res);

        if(res == 0)
            SetZ();
        else
            SetZ(0);
        if(res < 0)
            SetN();
        else
            SetN(0);
    }
    else if(size == 1) // Word
    {
        int16_t data = GetWord(eamode,  eareg, calcTime);
        int16_t res = ~data;
        SetWord(lastAddress, res);

        if(res == 0)
            SetZ();
        else
            SetZ(0);
        if(res < 0)
            SetN();
        else
            SetN(0);
    }
    else // long
    {
        int32_t data = GetLong(eamode,  eareg, calcTime);
        int32_t res = ~data;
        SetLong(lastAddress, res);

        if(res == 0)
            SetZ();
        else
            SetZ(0);
        if(res < 0)
            SetN();
        else
            SetN(0);
    }
    SetVC(0);

    if(eamode <= 1)
        calcTime += 7;
    else
        if(size <= 1)
            calcTime += 11;
        else
            calcTime += 15;

    return calcTime;
}

uint16_t SCC68070::Or()
{


    return 0;
}

uint16_t SCC68070::Ori()
{


    return 1;
}

uint16_t SCC68070::Pea()
{


    return 0;
}

uint16_t SCC68070::Reset()
{
#ifdef LOG_OPCODE
    log("RESET ");
#endif // LOG_OPCODE

    return 154;
}

uint16_t SCC68070::RoM()
{


    return 0;
}

uint16_t SCC68070::RoR()
{


    return 0;
}

uint16_t SCC68070::RoxM()
{
#ifdef LOG_OPCODE
    std::cout << "ROXm";
#endif // LOG_OPCODE

    return 0;
}

uint16_t SCC68070::RoxR()
{
#ifdef LOG_OPCODE
    std::cout << "ROXr";
#endif // LOG_OPCODE

    return 0;
}

uint16_t SCC68070::Rte()
{


    return 0;
}

uint16_t SCC68070::Rtr()
{
    SR &= 0xFFE0;
    SR |= GetWord(ARIWPo(7, 2)) & 0x001F;
    PC = GetLong(ARIWPo(7, 4));
    instructionsBuffer += "RTR; ";
    return 22;
}

uint16_t SCC68070::Rts()
{
    PC = GetLong(ARIWPo(7, 4));
    instructionsBuffer += "RTS; ";
    return 15;
}

uint16_t SCC68070::Sbcd()
{
    uint8_t Ry = (currentOpcode & 0x0E00) >> 9;
    uint8_t Rx = (currentOpcode & 0x0007);
    uint8_t result = 0;
    uint16_t calcTime;
    uint8_t x = GetX();

    if(currentOpcode & 0x0008) // memory to memory
    {
        uint8_t src = GetByte(ARIWPr(Rx, 1));
        uint8_t dst = GetByte(ARIWPr(Ry, 1));
        if((dst & 0x0F) < (src & 0x0F))
            SetXC();
        if(((dst & 0xF0) >> 4) < ((src & 0xF0) >> 4))
            SetXC();

        result = convertPBCD(dst) - convertPBCD(src) - x;
        SetByte(lastAddress, result);
        calcTime = 31;
    }
    else
    {
        uint8_t src = D[Rx] & 0x000000FF;
        uint8_t dst = D[Ry] & 0x000000FF;
        if((dst & 0x0F) < (src & 0x0F))
            SetXC();
        if(((dst & 0xF0) >> 4) < ((src & 0xF0) >> 4))
            SetXC();

        result = convertPBCD(dst) - convertPBCD(src) - x;
        D[Ry] &= 0xFFFFFF00;
        D[Ry] |= result;
        calcTime = 10;
    }

    if(result != 0)
        SetZ(0);

    return calcTime;
}

uint16_t SCC68070::SCC()
{
    uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    uint8_t    eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t     eareg = (currentOpcode & 0x0007);
    uint8_t data;
    uint16_t calcTime = 14; // in case of default:

    if((this->*ConditionalTests[condition])())
        data = 0xFF;
    else
        data = 0x00;

    SetByte(eamode, eareg, calcTime, data);

    return calcTime;
}

uint16_t SCC68070::Stop() // Not fully emulated
{
    uint16_t data = GetNextWord();
    uint16_t calcTime = 17;

    if(GetS())
    {
        SR = data;
        if(data == 0)
            SetZ();
        else
            SetZ(0);

        if(data & 0x8000)
            SetN();
        else
            SetN(0);

        SetV(0);
        SetC(0);
    }
    else
        Exception(8, calcTime);

    return calcTime;
}

uint16_t SCC68070::Sub()
{


    return 0;
}

uint16_t SCC68070::Suba()
{


    return 0;
}

uint16_t SCC68070::Subi()
{


    return 0;
}

uint16_t SCC68070::Subq()
{


    return 0;
}

uint16_t SCC68070::Subx()
{


    return 0;
}

uint16_t SCC68070::Swap()
{
    uint8_t reg = currentOpcode & 0x0007;

    uint16_t tmp = (D[reg] & 0xFFFF0000) >> 16;
    D[reg] <<= 16;
    D[reg] |= tmp;

    if(D[reg] == 0)
        SetZ();
    else
        SetZ(0);

    if(D[reg] & 0x80000000)
        SetN();
    else
        SetN(0);

    SetV(0);
    SetC(0);

    return 7;
}

uint16_t SCC68070::Tas()
{
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 10;

    uint8_t data = GetByte(eamode, eareg, calcTime);
    if(data == 0)
        SetZ();
    else
        SetZ(0);

    if(data & 0x80)
        SetN();
    else
    {   SetZ(0); data |= 0x80; }

    SetV(0);
    SetC(0);

    if(eamode)
    {   calcTime++; SetByte(lastAddress, data); }
    else
    {   D[eareg] &= 0xFFFFFF00; D[eareg] |= data; }
    return calcTime;
}

uint16_t SCC68070::Trap()
{
    uint8_t vec = currentOpcode & 0x000F;
    uint16_t calcTime = 0;
    Exception(32 + vec, calcTime);
    return calcTime;
}

uint16_t SCC68070::Trapv()
{
    uint16_t calcTime = 0;

    if(GetV())
        Exception(7, calcTime);
    else
        calcTime = 10;

    return calcTime;
}

uint16_t SCC68070::Tst()
{
#define COMPARE if(!data) SetZ(); else SetZ(0); if(data < 0) SetN(); else SetN(0);
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 0)
    {
        int8_t data = GetByte(eamode, eareg, calcTime);
        COMPARE
    }
    else if(size == 1)
    {
        int16_t data = GetWord(eamode, eareg, calcTime);
        COMPARE
    }
    else
    {
        int32_t data = GetLong(eamode, eareg, calcTime);
        COMPARE
    }

    SetC(0);
    SetV(0);
    return calcTime;
}

uint16_t SCC68070::Unlk()
{
    uint8_t reg = currentOpcode & 0x0007;
    SP = A[reg];
    A[reg] = GetLong(ARIWPo(7, 4));
    return 15;
}
