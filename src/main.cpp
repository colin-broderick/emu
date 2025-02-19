#include <sstream>

#include "system.hpp"
#include "input_parser.hpp"

/** \brief Application entry point. Creates a NES system and executes a loaded program. */
int main(int argc, char *argv[])
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
        std::string rom_file_name = input.get_command_option("-r");
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
        Word stack_pointer;
        std::istringstream(input.get_command_option("-sp")) >> std::hex >> stack_pointer;
        NES.cpu.set_stack_pointer(stack_pointer);
    }

    // Check for and set instruction pointer.
    if (input.contains("-ip"))
    {
        Word instruction_pointer;
        std::istringstream(input.get_command_option("-ip")) >> std::hex >> instruction_pointer;
        NES.cpu.set_instruction_pointer(instruction_pointer);
    }

    std::cout << "SP:" << (int)NES.cpu.get_stack_pointer() << std::endl;
    NES.run();

    return 0;
}
