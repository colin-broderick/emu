namespace Bus
{
    bool load_rom(const std::string &filename);
    void run();
}

namespace Cpu
{
    static uint16_t stack_pointer;
    static uint16_t instruction_pointer;
}