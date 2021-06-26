#include <iomanip>

#include "memory.hpp"

Memory::Memory()
{
    for (int i = 0; i < 64; i++)
    {
        data[i] = 3;
    }
}

std::ostream& operator<<(std::ostream& stream, Memory& memory)
{
    for (unsigned int i = 0; i < 8; i++)
    {
        for (unsigned int j = 0; j < 8; j++)
        {
            stream << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)memory[i*8+j] << " ";
        }
        stream << "\n";
    }
    return stream;
}

Byte& Memory::operator[](int index)
{
    if (index >= 0)
    {
        return data[index];
    }
    else
    {
        return data[data.size() + index];
    }
}
