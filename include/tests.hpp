#ifndef EMU_TESTS_H
#define EMU_TESTS_H

#include <array>

#include "system.hpp"

class EmulatorTest
{
    private:
        System nes;
        std::array<Byte, 128> expected_result;
    
    public:
        std::string name;
        EmulatorTest(const std::array<Byte, 128>& program, const std::array<Byte, 128>& expected_result, std::string name);
        EmulatorTest(const EmulatorTest& e);
        bool run();
};

#endif
