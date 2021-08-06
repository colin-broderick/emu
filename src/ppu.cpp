#include "ppu.hpp"

/** \brief Constructor. */
PPU::PPU()
{

}

/** \brief Run the PPU until the internal count of clock cycles reaches zero.
 * \param memory Reference to system memory.
 * \param cycles Number of cycles to add to internal count.
 * \return Code indicating whether to continue or stop running PPU.
 * 
 * The PPU will run until the number of cycles reaches zero. The number of cycles passed in is added to the internal
 * count, and the internal count is reduced when operations are performed. The internal count can become negative if an
 * operation takes more cycles than are currently available. Subsequent calls to the method will have no effect until
 * the internal cycle count becomes positive again.
 */
int PPU::run(Memory& memory, const int cycles)
{
    // TODO Everything

    // Update the internal registers of the PPU by reading them from memory.
    update_registers(memory);
    
    return PPU::CONTINUE;
}

/** \brief Reduce the number of available PPU clock cycles.
 * \param cycles_to_use How much to reduce the count of available cycles.
 */
void PPU::use_cycles(const int cycles_to_use)
{
    cycles_available -= cycles_to_use;
}

/** \brief Increases the number of available PPU clock cycles.
 * \param cycles_to_add How much to increase the count of available cycles.
 */
void PPU::add_cycles(const int cycles_to_add)
{
    cycles_available += cycles_to_add;
}

/** \brief Enable the interrupt flag on the CPU.
 * \param cpu Reference to the CPU to interrupt.
 */
void PPU::interrupt_cpu(CPU& cpu)
{
    cpu.set_interrupt();
}

/** \brief Updates the internal PPU regsiters from main memory.
 * \param memory Reference to main system memory.
 */
void PPU::update_registers(Memory& memory)
{
    ppuctrl   = {memory[PPU::ppuctrl_address]};
    ppumask   = {memory[PPU::ppumask_address]};
    ppustatus = {memory[PPU::ppustatus_address]};
}
