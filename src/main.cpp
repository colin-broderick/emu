#include "system.hpp"

int main()
{
    System NES;
    NES.load_example_prog(5);
    NES.run();

    return 0;
}
