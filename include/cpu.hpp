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
    const static Byte INSTR_6502_LDA_IMMEDIATE = 0xA9;       // 2, Load next byte into A.
    const static Byte INSTR_6502_LDA_ZEROPAGE = 0xA5;        // 3, Load from zero page address in next byte into A.
    const static Byte INSTR_6502_LDA_ZEROPAGE_X = 0xB5;      // 4, Load from (zero page addres + X) in next byte into A.
    const static Byte INSTR_6502_LDA_ABSOLUTE = 0xAD;        // 5, Load from a 16-bit address, contained in next two bytes.
    const static Byte INSTR_6502_LDA_ABSOLUTE_X = 0xBD;
    const static Byte INSTR_6502_LDA_ABSOLUTE_Y = 0xB9;
    const static Byte INSTR_6502_LDA_INDIRECT_X = 0xA1;
    const static Byte INSTR_6502_LDA_INDIRECT_Y = 0xB1;

    // LDX - LoaD X register
    // Set Z if X == 0.
    // Set N if bit 7 of X is on.
    const static Byte INSTR_6502_LDX_IMMEDIATE = 0xA2;       // 2, Load byte from next memory location into X.
    const static Byte INSTR_6502_LDX_ZEROPAGE = 0xA6;
    const static Byte INSTR_6502_LDX_ZEROPAGE_Y = 0xB6;
    const static Byte INSTR_6502_LDX_ABSOLUTE = 0xAE;
    const static Byte INSTR_6502_LDX_ABOLUTE_Y = 0xBE;

    // LDY - LoaD Y register
    const static Byte INSTR_6502_LDY_IMMEDIATE = 0xA0;       // 2, Load byte from next memory location into Y.

    // STA - STore A in memory
    const static Byte INSTR_6502_STA_ZEROPAGE = 0x85;        // 3, Load accumulator from following memory pointer.
    const static Byte INSTR_6502_STA_ABSOLUTE = 0x8D;        // 4, Store contents of A in 16-bit memory address.

    // STX - STore X in memory
    const static Byte INSTR_6502_STX_ABSOLUTE = 0x8E;        // 4, Store contents of X in 16-bit memory address.

    // STY - STore Y in memory
    const static Byte INSTR_6502_STY_ABSOLUTE = 0x8C;        // 4, Store contents of Y in 16-bit memory address.

    // TAX - Transfer A to X
    const static Byte INSTR_6502_TAX = 0xAA;                 // 2, Copies A into X.

    // ADC - ADd with Carry
    const static Byte INSTR_6502_ADC_IMMEDIATE = 0x69;       // 2, Add to A with carry and all sorts of flag nonsense.

    // INX - INcrement X
    const static Byte INSTR_6502_INX = 0xE8;                 // 2, Incremement the value in X.

    // DEX - DEcrement X
    const static Byte INSTR_6502_DEX = 0xCA;                 // 2, Decrement the value in X.

    // CPX - ComPare X register
    const static Byte INSTR_6502_CPX_IMMEDIATE = 0xE0;       // 2, Compare value in X with value in next memory location.

    // BNE - Branch if Not Equal
    const static Byte INSTR_6502_BNE_RELATIVE = 0xD0;        // 2 (+1 if branch, +2 if new page), branch if the Z flag is not set.

    // SED - SEt Decimal flag
    const static Byte INSTR_6502_SED = 0xF8;                 // 2, Set D flag to on.

    // ORA - Logical inclusive or with A.
    const static Byte INSTR_6502_ORA_INDIRECT_X = 0x01;      // 6, Incluse OR with register A, result going into register A.

    // BRK - Break
    const static Byte INSTR_6502_BRK = 0x00;                 // 7, Halt the program.

    private:
        void LDA_set_CPU_flags();
        void LDX_set_CPU_flags();
        void LDY_set_CPU_flags();
        void TAX_set_CPU_flags();
        void ORA_set_CPU_flags();

    public:
        Word SP;
        Word IP;
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
        Word get_word(Memory& memory, const Byte address);

        Byte flags_as_byte();
};

std::ostream& operator<<(std::ostream& stream, const CPU& cpu);

#endif
