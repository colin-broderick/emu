#ifndef CPU_H
#define CPU_H

#include <map>

#include "utils.hpp"

class Memory;

class CPU
{
    private:
        /********** 6502 opcodes **************************************/

        // LDA - LoaD Accumulator
        // Load a byte from memory into the accumulator.
        // Set Z if A == 0.
        // Set N if bit 7 of A is on.
        const static Byte INSTR_6502_LDA_IMMEDIATE   = 0xA9;    // 2
        const static Byte INSTR_6502_LDA_ZEROPAGE    = 0xA5;    // 3
        const static Byte INSTR_6502_LDA_ZEROPAGE_X  = 0xB5;    // 4
        const static Byte INSTR_6502_LDA_ABSOLUTE    = 0xAD;    // 4
        const static Byte INSTR_6502_LDA_ABSOLUTE_X  = 0xBD;    // 4+
        const static Byte INSTR_6502_LDA_ABSOLUTE_Y  = 0xB9;    // 4+
        const static Byte INSTR_6502_LDA_INDIRECT_X  = 0xA1;    // 6
        const static Byte INSTR_6502_LDA_INDIRECT_Y  = 0xB1;    // 5+

        // LDX - LoaD X
        // Load a byte from memory into the X register.
        // Set Z if X == 0.
        // Set N if bit 7 of X is on.
        const static Byte INSTR_6502_LDX_IMMEDIATE   = 0xA2;    // 2
        const static Byte INSTR_6502_LDX_ZEROPAGE    = 0xA6;    // 3
        const static Byte INSTR_6502_LDX_ZEROPAGE_Y  = 0xB6;    // 4
        const static Byte INSTR_6502_LDX_ABSOLUTE    = 0xAE;    // 4
        const static Byte INSTR_6502_LDX_ABSOLUTE_Y  = 0xBE;    // 4+

        // CMP - CoMPare
        // Compare the accumulator to a byte from memory.
        // TODO What registers are affected?
        const static Byte INSTR_6502_CMP_IMMEDIATE   = 0xC9;    // 2
        const static Byte INSTR_6502_CMP_ZEROPAGE    = 0xC5;    // 3
        const static Byte INSTR_6502_CMP_ZEROPAGE_X  = 0xD5;    // 4
        const static Byte INSTR_6502_CMP_ABSOLUTE    = 0xCD;    // 4
        const static Byte INSTR_6502_CMP_ABSOLUTE_X  = 0xDD;    // 4+
        const static Byte INSTR_6502_CMP_ABSOLUTE_Y  = 0xD9;    // 4+
        const static Byte INSTR_6502_CMP_INDIRECT_X  = 0xC1;    // 6
        const static Byte INSTR_6502_CMP_INDIRECT_Y  = 0xD1;    // 5+

        // EOR - Exclusive OR of accumulator with value from memory.
        const static Byte INSTR_6502_EOR_IMMEDIATE   = 0x49;    // 2
        const static Byte INSTR_6502_EOR_ZEROPAGE    = 0x45;    // 3
        const static Byte INSTR_6502_EOR_ZEROPAGE_X  = 0x55;    // 4
        const static Byte INSTR_6502_EOR_ABSOLUTE    = 0x4D;    // 4
        const static Byte INSTR_6502_EOR_ABSOLUTE_X  = 0x5D;    // 4+
        const static Byte INSTR_6502_EOR_ABSOLUTE_Y  = 0x59;    // 4+
        const static Byte INSTR_6502_EOR_INDIRECT_X  = 0x41;    // 6
        const static Byte INSTR_6502_EOR_INDIRECT_Y  = 0x51;    // 5+

        // LDY - LoaD Y register
        // Load byte from memory into Y register.
        const static Byte INSTR_6502_LDY_IMMEDIATE   = 0xA0;    // 2
        const static Byte INSTR_6502_LDY_ZEROPAGE    = 0xA4;    // 3
        const static Byte INSTR_6502_LDY_ZEROPAGE_X  = 0xB4;    // 4
        const static Byte INSTR_6502_LDY_ABSOLUTE    = 0xAC;    // 4
        const static Byte INSTR_6502_LDY_ABSOLUTE_X  = 0xBC;    // 4+

        // STA - STore A in memory
        const static Byte INSTR_6502_STA_ZEROPAGE    = 0x85;    // 3
        const static Byte INSTR_6502_STA_ZEROPAGE_X  = 0x95;    // 4
        const static Byte INSTR_6502_STA_ABSOLUTE    = 0x8D;    // 4
        const static Byte INSTR_6502_STA_ABSOLUTE_X  = 0x9D;    // 5
        const static Byte INSTR_6502_STA_ABSOLUTE_Y  = 0x99;    // 5
        const static Byte INSTR_6502_STA_INDIRECT_X  = 0x81;    // 6
        const static Byte INSTR_6502_STA_INDIRECT_Y  = 0x91;    // 6

        // STX - STore X in memory
        const static Byte INSTR_6502_STX_ZEROPAGE    = 0x86;    // 3
        const static Byte INSTR_6502_STX_ZEROPAGE_Y  = 0x96;    // 4
        const static Byte INSTR_6502_STX_ABSOLUTE    = 0x8E;    // 4

        // STY - STore Y in memory
        const static Byte INSTR_6502_STY_ZEROPAGE    = 0x84;    // 3
        const static Byte INSTR_6502_STY_ZEROPAGE_X  = 0x94;    // 4
        const static Byte INSTR_6502_STY_ABSOLUTE    = 0x8C;    // 4

        // TAX - Transfer A to X
        const static Byte INSTR_6502_TAX             = 0xAA;    // 2

        // TXA - Transfer X to A
        const static Byte INSTR_6502_TXA             = 0x8A;    // 2
        const static Byte INSTR_6502_TXS             = 0x9A;    // 2
        const static Byte INSTR_6502_TSX             = 0xBA;    // 2
        const static Byte INSTR_6502_TYA             = 0x98;    // 2
        const static Byte INSTR_6502_TAY             = 0xA8;    // 2

        // ADC - ADd with Carry
        const static Byte INSTR_6502_ADC_IMMEDIATE   = 0x69;    // 2
        const static Byte INSTR_6502_ADC_ZEROPAGE    = 0x65;    // 3
        const static Byte INSTR_6502_ADC_ZEROPAGE_X  = 0x75;    // 4
        const static Byte INSTR_6502_ADC_ABSOLUTE    = 0x6D;    // 4
        const static Byte INSTR_6502_ADC_ABSOLUTE_X  = 0x7D;    // 4+
        const static Byte INSTR_6502_ADC_ABSOLUTE_Y  = 0x79;    // 4+
        const static Byte INSTR_6502_ADC_INDIRECT_X  = 0x61;    // 6
        const static Byte INSTR_6502_ADC_INDIRECT_Y  = 0x71;    // 5+

        // SBC - SuBtract with Carry
        const static Byte INSTR_6502_SBC_IMMEDIATE   = 0xE9;    // 2
        const static Byte INSTR_6502_SBC_ZEROPAGE    = 0xE5;    // 3
        const static Byte INSTR_6502_SBC_ZEROPAGE_X  = 0xF5;    // 4
        const static Byte INSTR_6502_SBC_ABSOLUTE    = 0xED;    // 4
        const static Byte INSTR_6502_SBC_ABSOLUTE_X  = 0xFD;    // 4+
        const static Byte INSTR_6502_SBC_ABSOLUTE_Y  = 0xF9;    // 4+
        const static Byte INSTR_6502_SBC_INDIRECT_X  = 0xE1;    // 6
        const static Byte INSTR_6502_SBC_INDIRECT_Y  = 0xF1;    // 5+

        // PLP - PuLl Processor flags from stack
        const static Byte INSTR_6502_PLP             = 0x28;    // 4

        // INX - INcrement X
        const static Byte INSTR_6502_INX             = 0xE8;    // 2

        // INY - INcrement Y register
        const static Byte INSTR_6502_INY             = 0xC8;

        // DEX - DEcrement X
        const static Byte INSTR_6502_DEX             = 0xCA;    // 2

        // DEY - DEcrement Y register
        const static Byte INSTR_6502_DEY             = 0x88;

        // CPX - ComPare X register
        const static Byte INSTR_6502_CPX_IMMEDIATE   = 0xE0;    // 2
        const static Byte INSTR_6502_CPX_ZEROPAGE    = 0xE4;    // 3
        const static Byte INSTR_6502_CPX_ABSOLUTE    = 0xEC;    // 4

        // CPY - ComPare Y register
        const static Byte INSTR_6502_CPY_IMMEDIATE   = 0xC0;    // 2
        const static Byte INSTR_6502_CPY_ZEROPAGE    = 0xC4;    // 3
        const static Byte INSTR_6502_CPY_ABSOLUTE    = 0xCC;    // 4

        // BEQ - Branch if EQual
        const static Byte INSTR_6502_BEQ_RELATIVE    = 0xF0;    // 2+++
        
        // BNE - Branch if Not Equal
        const static Byte INSTR_6502_BNE_RELATIVE    = 0xD0;    // 2+++
        
        // BMI - Branch if MInus
        const static Byte INSTR_6502_BMI_RELATIVE    = 0x30;    // 2+++
        
        // BPL - Branch if Positive
        const static Byte INSTR_6502_BPL_RELATIVE    = 0x10;    // 2+++
        
        // BVS - Branch if oVerflow Set
        const static Byte INSTR_6502_BVS_RELATIVE    = 0x70;    // 2+++
        
        // BVC - Branch if oVerflow Clear
        const static Byte INSTR_6502_BVC_RELATIVE    = 0x50;    // 2+++
        
        // BCS - Branch if Carry Set
        const static Byte INSTR_6502_BCS_RELATIVE    = 0xB0;    // 2+++
        
        // BCC - Branch if Carry Clear
        const static Byte INSTR_6502_BCC_RELATIVE    = 0x90;    // 2+++

        // SED - SEt Decimal flag
        const static Byte INSTR_6502_SED             = 0xF8;    // 2

        // ORA - Logical inclusive or with A.
        const static Byte INSTR_6502_ORA_IMMEDIATE   = 0x09;    // 2
        const static Byte INSTR_6502_ORA_ZEROPAGE    = 0x05;    // 3
        const static Byte INSTR_6502_ORA_ZEROPAGE_X  = 0x15;    // 4
        const static Byte INSTR_6502_ORA_ABSOLUTE    = 0x0D;    // 4
        const static Byte INSTR_6502_ORA_ABSOLUTE_X  = 0x1D;    // 4+
        const static Byte INSTR_6502_ORA_ABSOLUTE_Y  = 0x19;    // 4+
        const static Byte INSTR_6502_ORA_INDIRECT_X  = 0x01;    // 6
        const static Byte INSTR_6502_ORA_INDIRECT_Y  = 0x11;    // 5+

        // ASL - Arithmetic Shift Left
        const static Byte INSTR_6502_ASL_ACCUMULATOR = 0x0A;    // 2
        const static Byte INSTR_6502_ASL_ZEROPAGE    = 0x06;    // 5
        const static Byte INSTR_6502_ASL_ZEROPAGE_X  = 0x16;    // 6
        const static Byte INSTR_6502_ASL_ABSOLUTE    = 0x0E;    // 6
        const static Byte INSTR_6502_ASL_ABSOLUTE_X  = 0x1E;    // 7

        // LSR - Logical Shift Right
        const static Byte INSTR_6502_LSR_ACCUMULATOR = 0x4A;    // 2
        const static Byte INSTR_6502_LSR_ZEROPAGE    = 0x46;    // 5
        const static Byte INSTR_6502_LSR_ZEROPAGE_X  = 0x56;    // 6
        const static Byte INSTR_6502_LSR_ABSOLUTE    = 0x4E;    // 6
        const static Byte INSTR_6502_LSR_ABSOLUTE_X  = 0x5E;    // 7

        // ROL - ROtate Left
        const static Byte INSTR_6502_ROL_ACCUMULATOR = 0x2A;    // 2
        const static Byte INSTR_6502_ROL_ZEROPAGE    = 0x26;    // 5
        const static Byte INSTR_6502_ROL_ZEROPAGE_X  = 0x36;    // 6
        const static Byte INSTR_6502_ROL_ABSOLUTE    = 0x2E;    // 6
        const static Byte INSTR_6502_ROL_ABSOLUTE_X  = 0x3E;    // 7

        // ROR - ROtate Right
        const static Byte INSTR_6502_ROR_ACCUMULATOR = 0x6A;    // 2
        const static Byte INSTR_6502_ROR_ZEROPAGE    = 0x66;    // 5
        const static Byte INSTR_6502_ROR_ZEROPAGE_X  = 0x76;    // 6
        const static Byte INSTR_6502_ROR_ABSOLUTE    = 0x6E;    // 6
        const static Byte INSTR_6502_ROR_ABSOLUTE_X  = 0x7E;    // 7

        // BIT
        const static Byte INSTR_6502_BIT_ZEROPAGE    = 0x24;    // 3
        const static Byte INSTR_6502_BIT_ABSOLUTE    = 0x2C;    // 4

        // BRK - Break
        const static Byte INSTR_6502_BRK             = 0x00;    // 7

        // SEC - SEt Carry flag on
        const static Byte INSTR_6502_SEC             = 0x38;    // 2

        // SEI - SEt Interrupt
        const static Byte INSTR_6502_SEI             = 0x78;    // 2

        // CLD - CLear Decimal flag
        const static Byte INSTR_6502_CLD             = 0xD8;    // 2

        // CLI - CLear Interrupt disable flag
        const static Byte INSTR_6502_CLI             = 0x58;    // 2

        // CLC - CLear Carry flag
        const static Byte INSTR_6502_CLC             = 0x18;

        // CLV - CLear oVerflow flag
        const static Byte INSTR_6502_CLV             = 0xB8;

        // NOP - No OPeration, i.e. do nothing for 2 cycles
        const static Byte INSTR_6502_NOP             = 0xEA;

        // PHA - PusH a copy of A onto the stack, and decrement stack pointer.
        const static Byte INSTR_6502_PHA             = 0x48;

        // PHP - PusH Processor status onto stack.
        const static Byte INSTR_6502_PHP             = 0x08;

        // PLA - Pull from stack into A;
        const static Byte INSTR_6502_PLA             = 0x68;

        // JSR - Jump to SubRoutine
        const static Byte INSTR_6502_JSR_ABSOLUTE    = 0x20;

        // RTS - ReTurn from Subroutine
        const static Byte INSTR_6502_RTS             = 0x60;    // 6

        // JMP - JuMP to address
        const static Byte INSTR_6502_JMP_ABSOLUTE    = 0x4c;    // 3
        const static Byte INSTR_6502_JMP_INDIRECT    = 0x6c;    // 5

        // DEC - DECrement memory
        const static Byte INSTR_6502_DEC_ZEROPAGE    = 0xC6;    // 5
        const static Byte INSTR_6502_DEC_ZEROPAGE_X  = 0xD6;    // 6
        const static Byte INSTR_6502_DEC_ABSOLUTE    = 0xCE;    // 6
        const static Byte INSTR_6502_DEC_ABSOLUTE_X  = 0xDE;    // 7

        // INC - INCrement memory
        const static Byte INSTR_6502_INC_ZEROPAGE    = 0xE6;    // 5
        const static Byte INSTR_6502_INC_ZEROPAGE_X  = 0xF6;    // 6
        const static Byte INSTR_6502_INC_ABSOLUTE    = 0xEE;    // 6
        const static Byte INSTR_6502_INC_ABSOLUTE_X  = 0xFE;    // 7

        // AND - bitwise logical AND operation
        const static Byte INSTR_6502_AND_IMMEDIATE   = 0x29;    // 2
        const static Byte INSTR_6502_AND_ZEROPAGE_X  = 0x35;    // 4
        const static Byte INSTR_6502_AND_ZEROPAGE    = 0x25;    // 3
        const static Byte INSTR_6502_AND_ABSOLUTE    = 0x2d;    // 4
        const static Byte INSTR_6502_AND_ABSOLUTE_X  = 0x3d;    // 4+
        const static Byte INSTR_6502_AND_ABSOLUTE_Y  = 0x39;    // 4+
        const static Byte INSTR_6502_AND_INDIRECT_X  = 0x21;    // 6
        const static Byte INSTR_6502_AND_INDIRECT_Y  = 0x31;    // 5+

        /**************************************************************/

    public:
        const static int CPU_frequency = 1790000; // Hz
        const static int cycles_per_frame = 29833;
        const static int microseconds_per_frame = 16667;

        /** CPU return codes. The CPU will generally run until it exhausts the supply of cycles, but under
         * certain conditions will return one of these codes. */
        enum RETURN_CODE
        {
            /** Instructs the CPU to stop. */
            BREAK,
             /** Instructs the CPU to continue. */
            CONTINUE
        };

        const std::array<std::string, 256> instruction_names = {
            "BRK impl", "ORA X,ind",  "---",   "---",  "---",       "ORA zpg",    "ASL zpg",    "---",  "PHP impl",  "ORA #",      "ASL A",    "---", "---",       "ORA abs",    "ASL abs",    "---",
            "BPL rel",  "ORA ind,Y",  "---",   "---",  "---",       "ORA zpg,X",  "ASL zpg,X",  "---",  "CLC impl",  "ORA abs,Y",  "---",      "---", "---",       "ORA abs,X",  "ASL abs,X",  "---",
            "JSR abs ", "AND X,ind",  "---",   "---",  "BIT zpg",   "AND zpg",    "ROL zpg",    "---",  "PLP impl",  "AND #",      "ROL A",    "---", "BIT abs",   "AND abs",    "ROL abs",    "---",
            "BMI rel",  "AND ind,Y",  "---",   "---",  "---",       "AND zpg,X",  "ROL zpg,X",  "---",  "SEC impl",  "AND abs,Y",  "---",      "---", "---",       "AND abs,X",  "ROL abs,X",  "---",
            "RTI impl", "EOR X,ind",  "---",   "---",  "---",       "EOR zpg",    "LSR zpg",    "---",  "PHA impl",  "EOR #",      "LSR A",    "---", "JMP abs",   "EOR abs",    "LSR abs",    "---",
            "BVC rel",  "EOR ind,Y",  "---",   "---",  "---",       "EOR zpg,X",  "LSR zpg,X",  "---",  "CLI impl",  "EOR abs,Y",  "---",      "---", "---",       "EOR abs,X",  "LSR abs,X",  "---",
            "RTS impl", "ADC X,ind",  "---",   "---",  "---",       "ADC zpg",    "ROR zpg",    "---",  "PLA impl",  "ADC #",      "ROR A",    "---", "JMP ind",   "ADC abs",    "ROR abs",    "---",
            "BVS rel",  "ADC ind,Y",  "---",   "---",  "---",       "ADC zpg,X",  "ROR zpg,X",  "---",  "SEI impl",  "ADC abs,Y",  "---",      "---", "---",       "ADC abs,X",  "ROR abs,X",  "---",
            "---",      "STA X,ind",  "---",   "---",  "STY zpg",   "STA zpg",    "STX zpg",    "---",  "DEY impl",  "---",        "TXA impl", "---", "STY abs",   "STA abs",    "STX abs",    "---",
            "BCC rel",  "STA ind,Y",  "---",   "---",  "STY zpg,X", "STA zpg,X",  "STX zpg,Y",  "---",  "TYA impl",  "STA abs,Y",  "TXS impl", "---", "---",       "STA abs,X",  "---",        "---",
            "LDY #",    "LDA X,ind",  "LDX #", "---",  "LDY zpg",   "LDA zpg",    "LDX zpg",    "---",  "TAY impl",  "LDA #",      "TAX impl", "---", "LDY abs",   "LDA abs",    "LDX abs",    "---",
            "BCS rel",  "LDA ind,Y",  "---",   "---",  "LDY zpg,X", "LDA zpg,X",  "LDX zpg,Y",  "---",  "CLV impl",  "LDA abs,Y",  "TSX impl", "---", "LDY abs,X", "LDA abs,X",  "LDX abs,Y",  "---",
            "CPY #",    "CMP X,ind",  "---",   "---",  "CPY zpg",   "CMP zpg",    "DEC zpg",    "---",  "INY impl",  "CMP #",      "DEX impl", "---", "CPY abs",   "CMP abs",    "DEC abs",    "---",
            "BNE rel",  "CMP ind,Y",  "---",   "---",  "---",       "CMP zpg,X",  "DEC zpg,X",  "---",  "CLD impl",  "CMP abs,Y",  "---",      "---", "---",       "CMP abs,X",  "DEC abs,X",  "---",
            "CPX #",    "SBC X,ind",  "---",   "---",  "CPX zpg",   "SBC zpg",    "INC zpg",    "---",  "INX impl",  "SBC #",      "NOP impl", "---", "CPX abs",   "SBC abs",    "INC abs",    "---",
            "BEQ rel",  "SBC ind,Y",  "---",   "---",  "---",       "SBC zpg,X",  "INC zpg,X",  "---",  "SED impl",  "SBC abs,Y",  "---",      "---", "---",       "SBC abs,X",  "INC abs,X",  "---"
        };

    private:
        // Internal flag setters ------------------------------------------------------------------------------------------------
        void LDA_set_CPU_flags();
        void AND_set_CPU_flags();
        void CMP_set_CPU_flags(Byte data_from_memory);
        void EOR_set_CPU_flags();
        void LDX_set_CPU_flags();
        void LDY_set_CPU_flags();
        void TAX_set_CPU_flags();
        void TXA_set_CPU_flags();
        void ORA_set_CPU_flags();
        void DEX_set_CPU_flags();
        void INX_set_CPU_flags();
        void DEY_set_CPU_flags();
        void INY_set_CPU_flags();
        void DEC_set_CPU_flags(Byte data_from_memory);
        void INC_set_CPU_flags(Byte data_from_memory);
        void CPX_set_CPU_flags(const int data_from_memory);
        void CPY_set_CPU_flags(const int data_from_memory);

        // Miscellaneous --------------------------------------------------------------------------------------------------------
        void branch_relative(Byte distance);
        Byte add_with_carry(const Byte data);
        Byte sub_with_carry(const Byte data);
        bool page_crossed = false;

        // Internal cycle storage -----------------------------------------------------------------------------------------------
        int cycles_available = 0;
        void use_cycles(const int cycles_to_use);
        void add_cycles(const int cycles_to_add);

        // Flags and registers --------------------------------------------------------------------------------------------------
        Word SP;                       // Stack pointer.
        Word IP;                       // Instruction pointer.
        Byte A, X, Y;                  // Accumulator and registers.
        bool C, Z, I, D, B, V, N;      // CPU flags.     

        // Internal setters -----------------------------------------------------------------------------------------------------
        void set_byte(Memory& memory, Word address, Byte value);
        void set_data_absolute(Memory& memory, Byte data);
        void set_data_absolute(Memory& memory, Byte data, Byte index);
        void set_data_zeropage(Memory& memory, Byte data);
        void set_data_zeropage(Memory& memory, Byte data, Byte index);
        void set_data_indexed_indirect(Memory& memory, Byte data, Byte index);
        void set_data_indirect_indexed(Memory& memory, const Byte data, const Byte index);

        // Internal getters -----------------------------------------------------------------------------------------------------
        Byte flags_as_byte();
        Byte pop_from_stack(Memory& memory);
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
        Byte get_data_zeropage(Memory& memory);
        Byte get_data_zeropage(Memory& memory, const Byte index);
        Byte get_data_indexed_indirect(Memory& memory, const Byte index);   //(Indirect,X) uses IP as indirect address
        Byte get_data_indirect_indexed(Memory& memory, const Byte index);   //(Indirect),Y uses IP as indirect address
    
    public:
        // Constructors ---------------------------------------------------------------------------------------------------------
        CPU();
        CPU(const unsigned int ip, const unsigned int sp);

        // Setters --------------------------------------------------------------------------------------------------------------
        void setIP(const Word newIP);
        void setSP(const Word newSP);

        // Getters --------------------------------------------------------------------------------------------------------------
        Word getIP();
        Word getSP();

        // Operators ------------------------------------------------------------------------------------------------------------
        friend std::ostream& operator<<(std::ostream& stream, const CPU& cpu);

        // General --------------------------------------------------------------------------------------------------------------
        int run(Memory& memory, const int cycles);
};

std::ostream& operator<<(std::ostream& stream, const CPU& cpu);

#endif
