#include <iomanip>

#include "memory.hpp"

/** \brief Constructor for memory object. Set all bytes to zero. */
Memory::Memory()
{
    data = {0};
}

/** \brief Prints summary of memory to an iostream.
 * \param stream Reference to iostream to which to print summary.
 * \param memory Reference to memory object.
 * \return Reference to iostream which was printed to.
 */
std::ostream &operator<<(std::ostream &stream, Memory &memory)
{
    stream << "Program:\t\t\t\t\t\t\t\t\t\t\tStack:\n";
    for (unsigned int i = 0; i < 16; i++)
    {
        for (unsigned int j = 0; j < 16; j++)
        {
            stream << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)memory[i * 16 + j] << " ";
        }
        stream << "\t\t";
        for (unsigned int j = 0; j < 16; j++)
        {
            stream << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)memory[0x100 + i * 16 + j] << " ";
        }
        stream << "\n";
    }
    return stream;
}

/** \brief Memory indexing operator.
 * \param index The position of the byte in memory to be read.
 * \return A reference to the particular byte to be read.
 */
Byte &Memory::operator[](int index)
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
