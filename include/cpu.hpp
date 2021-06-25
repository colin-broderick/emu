#ifndef CPU_H
#define CPU_H

#include "semaphore.hpp"
#include "utils.hpp"

class Memory;

class CPU
{
    // LDA - LoaD Accumulator
    // Set Z if A == 0.
    // Set N if bit 7 of A is on.
    const static Byte INSTR_LDA_IMMEDIATE = 0xA9;       // 2, Load next byte into A.
    const static Byte INSTR_LDA_ZEROPAGE = 0xA5;        // 3, Load from zero page address in next byte into A.
    const static Byte INSTR_LDA_ZEROPAGE_X = 0xB5;      // 4, Load from (zero page addres + X) in next byte into A.
    const static Byte INSTR_LDA_ABSOLUTE = 0xAD;        // 5, Load from a 16-bit address, contained in next two bytes.
    const static Byte INSTR_LDA_ABSOLUTE_X = 0xBD;
    const static Byte INSTR_LDA_ABSOLUTE_Y = 0xB9;
    const static Byte INSTR_LDA_INDIRECT_X = 0xA1;
    const static Byte INSTR_LDA_INDIRECT_Y = 0xB1;

    // LDX - LoaD X register
    // Set Z if X == 0.
    // Set N if bit 7 of X is on.
    const static Byte INSTR_LDX_IMMEDIATE = 0xA2;
    const static Byte INSTR_LDX_ZEROPAGE = 0xA6;
    const static Byte INSTR_LDX_ZEROPAGE_Y = 0xB6;
    const static Byte INSTR_LDX_ABSOLUTE = 0xAE;
    const static Byte INSTR_LDX_ABOLUTE_Y = 0xBE;

    // STA - STore A in memory
    const static Byte INSTR_STA_ABSOLUTE = 0x8D;        // 4, Store contents of A in 16-bit memory address.


    private:
        void LDA_set_CPU_flags();

    public:
        Byte SP;
        Byte IP;
        Byte A;
        Byte X;
        Byte Y;

        bool C, Z, I, D, B, V, N;
        
        CPU();
        void run(Memory& memory, Semaphore& sem);

        void set_byte(Memory& memory, Word address, Byte value);

        Byte get_byte(Memory& memory);
        Byte get_byte(Memory& memory, const Byte address);
        Byte get_byte(Memory& memory, const Word address);
        Word get_word(Memory& memory);
};

std::ostream& operator<<(std::ostream& stream, const CPU& cpu);

#endif
