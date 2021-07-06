#ifndef EMU_TESTS_H
#define EMU_TESTS_H

#include <array>

#include "system.hpp"

class MemoryTest
{
    private:
        System nes;
        std::array<Byte, 128> expected_result;
    
    public:
        std::string name;
        MemoryTest(const std::array<Byte, 128>& program, const std::array<Byte, 128>& expected_result, std::string name);
        MemoryTest(const MemoryTest& e);
        bool run();
};

#endif
