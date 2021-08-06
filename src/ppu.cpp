#include "ppu.hpp"

PPU::PPU()
{

}

int PPU::run(Memory& memory, const int cycles)
{
    // TODO
    return PPU::CONTINUE;
}

void PPU::use_cycles(const int cycles_to_use)
{
    cycles_available -= cycles_to_use;
}

void PPU::add_cycles(const int cycles_to_add)
{
    cycles_available += cycles_to_add;
}

void PPU::interrupt_cpu(CPU& cpu)
{
    cpu.set_interrupt();
}
