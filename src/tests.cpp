#include <vector>
#include <iostream>

#include "tests.hpp"

EmulatorTest::EmulatorTest(const std::array<Byte, 128>& program, const std::array<Byte, 128>& expected_result, std::string name)
{
    this->nes.load_short_program(program);
    this->name = name;
    this->expected_result = expected_result;
}

/** \brief Copy constructor. */
EmulatorTest::EmulatorTest(const EmulatorTest& other)
{
    this->nes.memory.data = other.nes.memory.data;
    this->name = other.name;
    this->expected_result = other.expected_result;
}

bool EmulatorTest::run()
{
    if (name == "Test3")
        return false;
    else
        return true;
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
    std::cout << "COMPILING TESTS" << std::endl;
    std::cout << "====================================" << std::endl;
    std::vector<EmulatorTest> tests;
    tests.emplace_back(EmulatorTest{std::array<Byte, 128>{1, 2, 3, 4}, std::array<Byte, 128>{1, 2, 3, 4}, "Test1"});
    tests.emplace_back(EmulatorTest{std::array<Byte, 128>{1, 2, 3, 4}, std::array<Byte, 128>{1, 2, 3, 4}, "Test2"});
    tests.emplace_back(EmulatorTest{std::array<Byte, 128>{1, 2, 3, 4}, std::array<Byte, 128>{1, 2, 3, 4}, "Test3"});
    tests.emplace_back(EmulatorTest{std::array<Byte, 128>{1, 2, 3, 4}, std::array<Byte, 128>{1, 2, 3, 4}, "Test4"});

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

    std::cout << "====================================" << std::endl;
    std::cout << "TESTS COMPLETED // PASSED " << tests_passed << "/" << tests.size() << std::endl;
}