#include <string.h>
#include <vector>
#include <iostream>

#include "tests.hpp"

MemoryTest::MemoryTest(const std::array<Byte, 128>& program, const std::array<Byte, 128>& expected_result, std::string name)
{
    this->nes.load_short_program(program);
    this->name = name;
    this->expected_result = expected_result;
}

/** \brief Copy constructor. */
MemoryTest::MemoryTest(const MemoryTest& other)
{
    this->nes.memory.data = other.nes.memory.data;
    this->name = other.name;
    this->expected_result = other.expected_result;
}

bool MemoryTest::run()
{
    this->nes.run();
    return !memcmp(&(this->nes.memory.data.data()[0x0300]), this->expected_result.data(), 128);
}

std::string red_text(std::string str)
{
    return "\033[1;31m" + str + "\033[0m";
}

std::string green_text(std::string str)
{
    return "\033[1;32m" + str + "\033[0m";
}

int main()
{
    std::cout << "COMPILING MEMORY TESTS" << std::endl;
    std::cout << "================================================" << std::endl;
    std::vector<MemoryTest> tests;
    tests.emplace_back(MemoryTest{std::array<Byte, 128>{CPU::INSTR_6502_INC_ABSOLUTE, 0x00, 0x03}, std::array<Byte, 128>{0x01, 0x00, 0x00, 0x00}, "INC, absolute addressing, once"});
    tests.emplace_back(MemoryTest{std::array<Byte, 128>{CPU::INSTR_6502_INC_ABSOLUTE, 0x00, 0x03}, std::array<Byte, 128>{0x01, 0x00, 0x00, 0x00}, "INC, absolute addressing, twice"});
    tests.emplace_back(MemoryTest{std::array<Byte, 128>{CPU::INSTR_6502_INC_ABSOLUTE, 0x00, 0x03}, std::array<Byte, 128>{0x01, 0x01, 0x00, 0x00}, "Deliberately failing test"});
    tests.emplace_back(MemoryTest{std::array<Byte, 128>{CPU::INSTR_6502_INC_ABSOLUTE, 0x00, 0x03}, std::array<Byte, 128>{0x01, 0x00, 0x00, 0x00}, "Some other test"});

    int tests_passed = 0;

    for (auto& test : tests)
    {
        bool success = test.run();
        if (!success)
        {
            std::cout << red_text("Test failed: ") << test.name << std::endl;
        }
        else
        {
            tests_passed++;
            std::cout << green_text("Test passed: ") << test.name << std::endl;
        }
    }

    std::cout << "================================================" << std::endl;
    std::cout << "MEMORY TESTS COMPLETED  //  PASSED " << tests_passed << "/" << tests.size() << std::endl;
}