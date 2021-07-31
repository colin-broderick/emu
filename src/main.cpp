#include "system.hpp"

/** \brief Application entry point. Creates a NES system and executes a loaded program. */
int main(int argc, char* argv[])
{
    // Check whether ROM file name is provided, and quit if not.
    // TODO Would like proper input parser, but wanted to keep it simple and understandble for now.
    if (argc < 2)
    {
        std::cout << "No ROM provided" << std::endl;
        return 0;
    }
    std::string rom_file_name = argv[1];
    
    System NES;
    NES.load_rom(rom_file_name);
    NES.cpu.setIP(0x400);
    NES.run();

    return 0;
}
