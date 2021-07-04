#include "system.hpp"

int main()
{
    System NES;
    NES.load_example_prog(2);
    NES.run();

    return 0;
}
