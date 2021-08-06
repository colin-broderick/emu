#ifndef PPU_H
#define PPU_H

#include "memory.hpp"
#include "cpu.hpp"

class PPU
{
    public:
        PPU();
        int run(Memory& memory, const int cycles);

        enum RETURN_CODE
        {
            BREAK,
            CONTINUE
        };

        friend std::ostream& operator<<(std::ostream& stream, const PPU& ppu);

    private:
        int cycles_available = 0;
        void use_cycles(const int cycles_to_use);
        void add_cycles(const int cycles_to_add);
        void interrupt_cpu(CPU& cpu);
};

#endif
