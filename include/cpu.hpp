#ifndef CPU_H
#define CPU_H

#include "semaphore.hpp"
#include "utils.hpp"

class Memory;

class CPU
{
    /********** 6502 opcodes in format: Instruction name = instruction byte // num cycles, description. ************************/

    // LDA - LoaD Accumulator
    // Set Z if A == 0.
    // Set N if bit 7 of A is on.
    const static Byte INSTR_6502_LDA_IMMEDIATE  = 0xA9;      // 2, Load next byte into A.
    const static Byte INSTR_6502_LDA_ZEROPAGE   = 0xA5;      // 3, Load from zero page address in next byte into A.
    const static Byte INSTR_6502_LDA_ZEROPAGE_X = 0xB5;      // 4, Load from (zero page addres + X) in next byte into A.
    const static Byte INSTR_6502_LDA_ABSOLUTE   = 0xAD;      // 4, Load from a 16-bit address, contained in next two bytes.
    const static Byte INSTR_6502_LDA_ABSOLUTE_X = 0xBD;      // 4+
    const static Byte INSTR_6502_LDA_ABSOLUTE_Y = 0xB9;      // 4+
    const static Byte INSTR_6502_LDA_INDIRECT_X = 0xA1;      // 6
    const static Byte INSTR_6502_LDA_INDIRECT_Y = 0xB1;      // 5+

    // LDX - LoaD X register
    // Set Z if X == 0.
    // Set N if bit 7 of X is on.
    const static Byte INSTR_6502_LDX_IMMEDIATE  = 0xA2;      // 2, Load byte from next memory location into X.
    const static Byte INSTR_6502_LDX_ZEROPAGE   = 0xA6;
    const static Byte INSTR_6502_LDX_ZEROPAGE_Y = 0xB6;
    const static Byte INSTR_6502_LDX_ABSOLUTE   = 0xAE;
    const static Byte INSTR_6502_LDX_ABOLUTE_Y  = 0xBE;

    // CMP - CoMPare accumulator to value from memory.
    const static Byte INSTR_6502_CMP_IMMEDIATE   = 0xC9;     // 2
    const static Byte INSTR_6502_CMP_ZERO_PAGE   = 0xC5;     // 3
    const static Byte INSTR_6502_CMP_ZERO_PAGE_X = 0xD5;     // 4
    const static Byte INSTR_6502_CMP_ABSOLUTE    = 0xCD;     // 4
    const static Byte INSTR_6502_CMP_ABSOLUTE_X  = 0xDD;     // 4+
    const static Byte INSTR_6502_CMP_ABSOLUTE_Y  = 0xD9;     // 4+
    const static Byte INSTR_6502_CMP_INDIRECT_X  = 0xC1;     // 6
    const static Byte INSTR_6502_CMP_INDIRECT_Y  = 0xD1;     // 5+

    // EOR - Exclusive OR of accumulator with value from memory.
    const static Byte INSTR_6502_EOR_IMMEDIATE   = 0x49;     // 2
    const static Byte INSTR_6502_EOR_ZERO_PAGE   = 0x45;     // 3
    const static Byte INSTR_6502_EOR_ZERO_PAGE_X = 0x55;     // 4
    const static Byte INSTR_6502_EOR_ABSOLUTE    = 0x4D;     // 4
    const static Byte INSTR_6502_EOR_ABSOLUTE_X  = 0x5D;     // 4+
    const static Byte INSTR_6502_EOR_ABSOLUTE_Y  = 0x59;     // 4+
    const static Byte INSTR_6502_EOR_INDIRECT_X  = 0x41;     // 6
    const static Byte INSTR_6502_EOR_INDIRECT_Y  = 0x51;     // 5+

    // LDY - LoaD Y register
    const static Byte INSTR_6502_LDY_IMMEDIATE   = 0xA0;     // 2, Load byte from next memory location into Y.
    const static Byte INSTR_6502_LDY_ZERO_PAGE   = 0xA4;     // 3, Load byte from next memory location into Y.
    const static Byte INSTR_6502_LDY_ZERO_PAGE_X = 0xB4;     // 4, Load byte from next memory location into Y.
    const static Byte INSTR_6502_LDY_ABSOLUTE    = 0xAC;     // 4, Load byte from next memory location into Y.
    const static Byte INSTR_6502_LDY_ABSOLUTE_X  = 0xBC;     // 4+, Load byte from next memory location into Y.

    // STA - STore A in memory
    const static Byte INSTR_6502_STA_ZERO_PAGE   = 0x85;     // 3, Load accumulator from following memory pointer.
    const static Byte INSTR_6502_STA_ZERO_PAGE_X = 0x95;     // 4
    const static Byte INSTR_6502_STA_ABSOLUTE    = 0x8D;     // 4, Store contents of A in 16-bit memory address.
    const static Byte INSTR_6502_STA_ABSOLUTE_X  = 0x9D;     // 5, Store contents of Y in following 16-bit address.
    const static Byte INSTR_6502_STA_ABSOLUTE_Y  = 0x99;     // 5, Store contents of Y in following 16-bit address.
    const static Byte INSTR_6502_STA_INDIRECT_X  = 0x81;     // 6, Store contents of Y in following 16-bit address.
    const static Byte INSTR_6502_STA_INDIRECT_Y  = 0x91;     // 6, Store contents of Y in following 16-bit address.

    // STX - STore X in memory
    const static Byte INSTR_6502_STX_ZERO_PAGE   = 0x86;     // 3, Store contents of X in 16-bit memory address.
    const static Byte INSTR_6502_STX_ZERO_PAGE_Y = 0x96;     // 4, Store contents of X in 16-bit memory address.
    const static Byte INSTR_6502_STX_ABSOLUTE    = 0x8E;     // 4, Store contents of X in 16-bit memory address.

    // STY - STore Y in memory
    const static Byte INSTR_6502_STY_ABSOLUTE = 0x8C;        // 4, Store contents of Y in 16-bit memory address.

    // TAX - Transfer A to X
    const static Byte INSTR_6502_TAX = 0xAA;                 // 2, Copies A into X.

    // TXA - Transfer X to A
    const static Byte INSTR_6502_TXA = 0x8A;                 // 2,
    const static Byte INSTR_6502_TXS = 0x9A;                 // 2
    const static Byte INSTR_6502_TSX = 0xBA;                 // 2
    const static Byte INSTR_6502_TYA = 0x98;                 // 2
    const static Byte INSTR_6502_TAY = 0xA8;                 // 2

    // ADC - ADd with Carry
    const static Byte INSTR_6502_ADC_IMMEDIATE   = 0x69;     // 2, Add to A with carry and all sorts of flag nonsense.
    const static Byte INSTR_6502_ADC_ZERO_PAGE   = 0x65;     // 3, Add to A with carry and all sorts of flag nonsense.
    const static Byte INSTR_6502_ADC_ZERO_PAGE_X = 0x75;     // 4, Add to A with carry and all sorts of flag nonsense.
    const static Byte INSTR_6502_ADC_ABSOLUTE    = 0x6D;     // 4, Add to A with carry and all sorts of flag nonsense.
    const static Byte INSTR_6502_ADC_ABSOLUTE_X  = 0x7D;     // 4+, Add to A with carry and all sorts of flag nonsense.
    const static Byte INSTR_6502_ADC_ABSOLUTE_Y  = 0x79;     // 4+, Add to A with carry and all sorts of flag nonsense.
    const static Byte INSTR_6502_ADC_INDIRECT_X  = 0x61;     // 6, Add to A with carry and all sorts of flag nonsense.
    const static Byte INSTR_6502_ADC_INDIRECT_Y  = 0x71;     // 5, Add to A with carry and all sorts of flag nonsense.


    // INX - INcrement X
    const static Byte INSTR_6502_INX = 0xE8;                 // 2, Incremement the value in X.

    // INY - INcrement Y register
    const static Byte INSTR_6502_INY = 0xC8;

    // DEX - DEcrement X
    const static Byte INSTR_6502_DEX = 0xCA;                 // 2, Decrement the value in X.

    // DEY - DEcrement Y register
    const static Byte INSTR_6502_DEY = 0x88;

    // CPX - ComPare X register
    const static Byte INSTR_6502_CPX_IMMEDIATE = 0xE0;       // 2, Compare value in X with value in next memory location.

    // CPY - ComPare Y register
    const static Byte INSTR_6502_CPY_IMMEDIATE = 0xC0;       // 2, As CPX, but for Y register.

    // BEQ - Branch if EQual
    const static Byte INSTR_6502_BEQ_RELATIVE = 0xF0;        // 2 (+1 if branch, +2 if new page), branch if the Z flag is set.
    
    // BNE - Branch if Not Equal
    const static Byte INSTR_6502_BNE_RELATIVE = 0xD0;        // 2 (+1 if branch, +2 if new page), branch if the Z flag is not set.
    
    // BMI - Branch if MInus
    const static Byte INSTR_6502_BMI_RELATIVE = 0x30;        // 2 (+1 if branch, +2 if new page), branch if the N flag is set.
    
    // BPL - Branch if Positive
    const static Byte INSTR_6502_BPL_RELATIVE = 0x10;        // 2 (+1 if branch, +2 if new page), branch if the N flag is not set.
    
    // BVS - Branch if oVerflow Set
    const static Byte INSTR_6502_BVS_RELATIVE = 0x70;        // 2 (+1 if branch, +2 if new page), branch if the V flag is set.
    
    // BVC - Branch if oVerflow Clear
    const static Byte INSTR_6502_BVC_RELATIVE = 0x50;        // 2 (+1 if branch, +2 if new page), branch if the V flag is not set.
    
    // BCS - Branch if Carry Set
    const static Byte INSTR_6502_BCS_RELATIVE = 0xB0;        // 2 (+1 if branch, +2 if new page), branch if the C flag is set.
    
    // BCC - Branch if Carry Clear
    const static Byte INSTR_6502_BCC_RELATIVE = 0x90;        // 2 (+1 if branch, +2 if new page), branch if the C flag is not set.

    // SED - SEt Decimal flag
    const static Byte INSTR_6502_SED = 0xF8;                 // 2, Set D flag to on.

    // ORA - Logical inclusive or with A.
    const static Byte INSTR_6502_ORA_INDIRECT_X = 0x01;      // 6, Incluse OR with register A, result going into register A.

    // BRK - Break
    const static Byte INSTR_6502_BRK = 0x00;                 // 7, Halt the program.

    // CLD - CLear Decimal flag
    const static Byte INSTR_6502_CLD = 0xD8;                 // 2, Sets D flag to off.

    // CLI - CLear Interrupt disable flag
    const static Byte INSTR_6502_CLI = 0x58;                 // 2, Clear interrupt disable flag.

    // CLC - CLear Carry flag
    const static Byte INSTR_6502_CLC = 0x18;

    // CLV - CLear oVerflow flag
    const static Byte INSTR_6502_CLV = 0xB8;

    // NOP - No OPeration, i.e. do nothing for 2 cycles
    const static Byte INSTR_6502_NOP = 0xEA;

    // PHA - PusH a copy of A onto the stack, and decrement stack pointer.
    const static Byte INSTR_6502_PHA = 0x48;

    // PHP - PusH Processor status onto stack.
    const static Byte INSTR_6502_PHP = 0x08;

    // PLA - Pull from stack into A;
    const static Byte INSTR_6502_PLA = 0x68;

    // JSR - Jump to SubRoutine
    const static Byte INSTR_6502_JSR_ABSOLUTE = 0x20;

    // RTS - ReTurn from Subroutine
    const static Byte INSTR_6502_RTS = 0x60;                 // 6, Pulls IP from stack and jumps to that address.

    // JMP - JuMP to address
    const static Byte INSTR_6502_JMP_ABSOLUTE = 0x4c;        // 3, Sets IP equal to address
    const static Byte INSTR_6502_JMP_INDIRECT = 0x6c;        // 5, Sets IP equal to word stored at address & address + 1

    // DEC - DECrement memory
    const static Byte INSTR_6502_DEC_ABSOLUTE   = 0xCE;      // 6, Subtracts one from memory location.
    const static Byte INSTR_6502_DEC_ABSOLUTE_X = 0xDE;      // 7, Subtracts one from memory location.

    // INC - INCrement memory
    const static Byte INSTR_6502_INC_ABSOLUTE =   0xEE;      // 6, Adds one to the value in memory, setting Z and N if required
    const static Byte INSTR_6502_INC_ABSOLUTE_X = 0xFE;      // 7, Adds one to the value in memory, setting Z and N if required

    // AND - logical AND operation
    const static Byte INSTR_6502_AND_IMMEDIATE  = 0x29;      // 2, performs (accumulator AND operand) then stores in accumulator
    const static Byte INSTR_6502_AND_ZEROPAGE_X = 0x35;      // 4, performs (accumulator AND operand) then stores in accumulator
    const static Byte INSTR_6502_AND_ZEROPAGE   = 0x25;      // 3, performs (accumulator AND operand) then stores in accumulator
    const static Byte INSTR_6502_AND_ABSOLUTE   = 0x2d;      // 4, performs (accumulator AND operand) then stores in accumulator
    const static Byte INSTR_6502_AND_ABSOLUTE_X = 0x3d;      // 4 (+1 if page boundary crossed), performs (accumulator AND operand) then stores in accumulator
    const static Byte INSTR_6502_AND_ABSOLUTE_Y = 0x39;      // 4 (+1 if page boundary crossed), performs (accumulator AND operand) then stores in accumulator
    const static Byte INSTR_6502_AND_INDIRECT_X = 0x21;      // 6 performs (accumulator AND operand) then stores in accumulator
    const static Byte INSTR_6502_AND_INDIRECT_Y = 0x31;      // 5 (+1 if page boundary crossed), performs (accumulator AND operand) then stores in accumulator

    /***************************************************************************************************************************/

    private:
        void LDA_set_CPU_flags();
        void CMP_set_CPU_flags(Byte data_from_memory);
        void EOR_set_CPU_flags();
        void LDX_set_CPU_flags();
        void LDY_set_CPU_flags();
        void TAX_set_CPU_flags();
        void TXA_set_CPU_flags();
        void ORA_set_CPU_flags();

        void branch_relative(Byte distance);
        Byte add_with_carry(Byte data);

        bool page_crossed = false;

    public:
        // Attributes -----------------------------------------------------------------------------------------------------------
        Word SP;           // Stack pointer.
        Word IP;           // Instruction pointer.
        Byte A, X, Y;      // Accumulator and registers.
        bool C, Z, I, D, B, V, N;   // CPU flags.     
        Semaphore sem;     // The semaphore is notified on every clock tick to release a cycle for consumption by the CPU.

        // Constructors ---------------------------------------------------------------------------------------------------------
        CPU();

        // Setters --------------------------------------------------------------------------------------------------------------
        void set_byte(Memory& memory, Word address, Byte value);

        // Getters --------------------------------------------------------------------------------------------------------------
        Byte get_byte(Memory& memory);
        Byte get_byte(Memory& memory, const Byte address);
        Byte get_byte(Memory& memory, const Word address);
        Word get_word(Memory& memory);
        Word get_word(Memory& memory, const Byte address);
        Word get_word(Memory& memory, const Word address);
        Word get_word_zpg_wrap(Memory& memory, const Byte address);

        Byte get_data_absolute(Memory& memory);
        Byte get_data_absolute(Memory& memory, const Byte index);
        Byte get_data_relative(Memory& memory);
        Byte get_data_immediate(Memory& memory);
        Byte get_data_zero_page(Memory& memory);
        Byte get_data_zero_page(Memory& memory, const Byte index);
        Byte get_data_indexed_indirect(Memory& memory, const Byte index);   //(Indirect,X) uses IP as indirect address
        Byte get_data_indirect_indexed(Memory& memory, const Byte index);   //(Indirect),Y uses IP as indirect address

        void set_data_absolute(Memory& memory, Byte data);
        void set_data_absolute(Memory& memory, Byte data, Byte index);
        void set_data_zero_page(Memory& memory, Byte data);
        void set_data_zero_page(Memory& memory, Byte data, Byte index);
        void set_data_indexed_indirect(Memory& memory, Byte data, Byte index);
        void set_data_indirect_indexed(Memory& memory, const Byte data, const Byte index);

        Byte flags_as_byte();

        // General --------------------------------------------------------------------------------------------------------------
        void run(Memory& memory);
};

std::ostream& operator<<(std::ostream& stream, const CPU& cpu);

#endif
