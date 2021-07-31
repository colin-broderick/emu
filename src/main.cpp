#include <sstream>

#include "system.hpp"
#include "input_parser.hpp"

/** \brief Application entry point. Creates a NES system and executes a loaded program. */
int main(int argc, char* argv[])
{
    System NES;
    
    InputParser input{argc, argv};
    if (input.contains("-h") || input.contains("-help") || !input.contains("-r"))
    {
        // TODO Add proper help text, then quit.
        std::cout << "Usage:" << std::endl;
        std::cout << "  -r    Path to ROM file" << std::endl;
        std::cout << "  -ip   Specify the starting instruction pointer (in hex)" << std::endl;
        std::cout << "  -sp   Specify the starting stack pointer (in hex)" << std::endl;
        return 0;
    }

    // Get ROM file name from arguments.
    if (input.contains("-r"))
    {
        std::string rom_file_name = input.getCmdOption("-r");
        NES.load_rom(rom_file_name);
    }
    else
    {
        std::cout << "No ROM provided" << std::endl;
        return 0;
    }

    // Check for and set stack pointer.
    if (input.contains("-sp"))
    {
        Word sp;
        std::istringstream(input.getCmdOption("-sp")) >> std::hex >> sp;
        NES.cpu.setSP(sp);
    }

    // Check for and set instruction pointer.
    if (input.contains("-ip"))
    {
        Word ip;
        std::istringstream(input.getCmdOption("-ip")) >> std::hex >> ip;
        NES.cpu.setIP(ip);
    }

    std::cout << "SP:" << (int)NES.cpu.getSP() << std::endl;
    NES.run();

    return 0;
}
