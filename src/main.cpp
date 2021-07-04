#include "system.hpp"

int main()
{
    System NES;
    NES.load_example_prog(7);
    NES.run();

    return 0;
}
