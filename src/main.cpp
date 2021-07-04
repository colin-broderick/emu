#include "system.hpp"

int main()
{
    System NES;
    NES.load_example_prog(6);
    NES.run();

    return 0;
}
