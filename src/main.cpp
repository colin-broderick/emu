#include <iostream>
#include <thread>
#include <chrono>

#include "system.hpp"

int main()
{
    System NES;
    NES.load_example_prog(1);
    NES.run();

    return 0;
}
