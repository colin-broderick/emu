#include <cstdint>
#include <array>
#include <exception>
#include <iostream>
#include <iomanip>
#include <string_view>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>

#include "input_parser.hpp"

#include <cstdint>

/** CPU return codes. The CPU will generally run until it exhausts the supply of cycles, but under
 * certain conditions will return one of these codes. */
enum class ReturnCode
{
    /** Instructs the CPU to stop. */
    BREAK,
    /** Instructs the CPU to continue. */
    CONTINUE
};

namespace Cpu
{
    uint16_t get_word(uint16_t address);
    uint16_t get_word_zpg_wrap(const uint8_t address);
    void LDX_set_CPU_flags();
    uint8_t add_with_carry(const uint8_t data);
    ReturnCode tick(const int cycles_to_add);
}

#define DEBUG 1

#if DEBUG
#define LOG(x) std::cout << x << std::endl
#else
#define LOG(x)
#endif

#define BIT0 0b00000001
#define BIT1 0b00000010
#define BIT2 0b00000100
#define BIT3 0b00001000
#define BIT4 0b00010000
#define BIT5 0b00100000
#define BIT6 0b01000000
#define BIT7 0b10000000

constexpr uint32_t ROM_BUFFER_SIZE = 0xFFFF;

namespace Cpu
{
    constexpr static int CPU_frequency = 1790000; // Hz
    constexpr static int frame_rate = 60;
    constexpr static int cycles_per_frame = CPU_frequency / frame_rate;
    constexpr static int microseconds_per_frame = 1000000 / frame_rate;
}

constexpr std::array<std::string_view, 256> instruction_names = {
    "BRK impl", "ORA X,ind", "---", "---", "---", "ORA zpg", "ASL zpg", "---", "PHP impl", "ORA #", "ASL A", "---", "---", "ORA abs", "ASL abs", "---",
    "BPL rel", "ORA ind,Y", "---", "---", "---", "ORA zpg,X", "ASL zpg,X", "---", "CLC impl", "ORA abs,Y", "---", "---", "---", "ORA abs,X", "ASL abs,X", "---",
    "JSR abs ", "AND X,ind", "---", "---", "BIT zpg", "AND zpg", "ROL zpg", "---", "PLP impl", "AND #", "ROL A", "---", "BIT abs", "AND abs", "ROL abs", "---",
    "BMI rel", "AND ind,Y", "---", "---", "---", "AND zpg,X", "ROL zpg,X", "---", "SEC impl", "AND abs,Y", "---", "---", "---", "AND abs,X", "ROL abs,X", "---",
    "RTI impl", "EOR X,ind", "---", "---", "---", "EOR zpg", "LSR zpg", "---", "PHA impl", "EOR #", "LSR A", "---", "JMP abs", "EOR abs", "LSR abs", "---",
    "BVC rel", "EOR ind,Y", "---", "---", "---", "EOR zpg,X", "LSR zpg,X", "---", "CLI impl", "EOR abs,Y", "---", "---", "---", "EOR abs,X", "LSR abs,X", "---",
    "RTS impl", "ADC X,ind", "---", "---", "---", "ADC zpg", "ROR zpg", "---", "PLA impl", "ADC #", "ROR A", "---", "JMP ind", "ADC abs", "ROR abs", "---",
    "BVS rel", "ADC ind,Y", "---", "---", "---", "ADC zpg,X", "ROR zpg,X", "---", "SEI impl", "ADC abs,Y", "---", "---", "---", "ADC abs,X", "ROR abs,X", "---",
    "---", "STA X,ind", "---", "---", "STY zpg", "STA zpg", "STX zpg", "---", "DEY impl", "---", "TXA impl", "---", "STY abs", "STA abs", "STX abs", "---",
    "BCC rel", "STA ind,Y", "---", "---", "STY zpg,X", "STA zpg,X", "STX zpg,Y", "---", "TYA impl", "STA abs,Y", "TXS impl", "---", "---", "STA abs,X", "---", "---",
    "LDY #", "LDA X,ind", "LDX #", "---", "LDY zpg", "LDA zpg", "LDX zpg", "---", "TAY impl", "LDA #", "TAX impl", "---", "LDY abs", "LDA abs", "LDX abs", "---",
    "BCS rel", "LDA ind,Y", "---", "---", "LDY zpg,X", "LDA zpg,X", "LDX zpg,Y", "---", "CLV impl", "LDA abs,Y", "TSX impl", "---", "LDY abs,X", "LDA abs,X", "LDX abs,Y", "---",
    "CPY #", "CMP X,ind", "---", "---", "CPY zpg", "CMP zpg", "DEC zpg", "---", "INY impl", "CMP #", "DEX impl", "---", "CPY abs", "CMP abs", "DEC abs", "---",
    "BNE rel", "CMP ind,Y", "---", "---", "---", "CMP zpg,X", "DEC zpg,X", "---", "CLD impl", "CMP abs,Y", "---", "---", "---", "CMP abs,X", "DEC abs,X", "---",
    "CPX #", "SBC X,ind", "---", "---", "CPX zpg", "SBC zpg", "INC zpg", "---", "INX impl", "SBC #", "NOP impl", "---", "CPX abs", "SBC abs", "INC abs", "---",
    "BEQ rel", "SBC ind,Y", "---", "---", "---", "SBC zpg,X", "INC zpg,X", "---", "SED impl", "SBC abs,Y", "---", "---", "---", "SBC abs,X", "INC abs,X", "---"};

/********** 6502 opcodes **************************************/

// LDA - LoaD Accumulator
// Load a byte from memory into the accumulator.
// Set Z if A == 0.
// Set N if bit 7 of A is on.
constexpr static uint8_t INSTR_6502_LDA_IMMEDIATE = 0xA9;  // 2
constexpr static uint8_t INSTR_6502_LDA_ZEROPAGE = 0xA5;   // 3
constexpr static uint8_t INSTR_6502_LDA_ZEROPAGE_X = 0xB5; // 4
constexpr static uint8_t INSTR_6502_LDA_ABSOLUTE = 0xAD;   // 4
constexpr static uint8_t INSTR_6502_LDA_ABSOLUTE_X = 0xBD; // 4+
constexpr static uint8_t INSTR_6502_LDA_ABSOLUTE_Y = 0xB9; // 4+
constexpr static uint8_t INSTR_6502_LDA_INDIRECT_X = 0xA1; // 6
constexpr static uint8_t INSTR_6502_LDA_INDIRECT_Y = 0xB1; // 5+

// LDX - LoaD X
// Load a byte from memory into the X register.
// Set Z if X == 0.
// Set N if bit 7 of X is on.
constexpr static uint8_t INSTR_6502_LDX_IMMEDIATE = 0xA2;  // 2
constexpr static uint8_t INSTR_6502_LDX_ZEROPAGE = 0xA6;   // 3
constexpr static uint8_t INSTR_6502_LDX_ZEROPAGE_Y = 0xB6; // 4
constexpr static uint8_t INSTR_6502_LDX_ABSOLUTE = 0xAE;   // 4
constexpr static uint8_t INSTR_6502_LDX_ABSOLUTE_Y = 0xBE; // 4+

// CMP - CoMPare
// Compare the accumulator to a byte from memory.
// TODO What registers are affected?
constexpr static uint8_t INSTR_6502_CMP_IMMEDIATE = 0xC9;  // 2
constexpr static uint8_t INSTR_6502_CMP_ZEROPAGE = 0xC5;   // 3
constexpr static uint8_t INSTR_6502_CMP_ZEROPAGE_X = 0xD5; // 4
constexpr static uint8_t INSTR_6502_CMP_ABSOLUTE = 0xCD;   // 4
constexpr static uint8_t INSTR_6502_CMP_ABSOLUTE_X = 0xDD; // 4+
constexpr static uint8_t INSTR_6502_CMP_ABSOLUTE_Y = 0xD9; // 4+
constexpr static uint8_t INSTR_6502_CMP_INDIRECT_X = 0xC1; // 6
constexpr static uint8_t INSTR_6502_CMP_INDIRECT_Y = 0xD1; // 5+

// EOR - Exclusive OR of accumulator with value from memory.
constexpr static uint8_t INSTR_6502_EOR_IMMEDIATE = 0x49;  // 2
constexpr static uint8_t INSTR_6502_EOR_ZEROPAGE = 0x45;   // 3
constexpr static uint8_t INSTR_6502_EOR_ZEROPAGE_X = 0x55; // 4
constexpr static uint8_t INSTR_6502_EOR_ABSOLUTE = 0x4D;   // 4
constexpr static uint8_t INSTR_6502_EOR_ABSOLUTE_X = 0x5D; // 4+
constexpr static uint8_t INSTR_6502_EOR_ABSOLUTE_Y = 0x59; // 4+
constexpr static uint8_t INSTR_6502_EOR_INDIRECT_X = 0x41; // 6
constexpr static uint8_t INSTR_6502_EOR_INDIRECT_Y = 0x51; // 5+

// LDY - LoaD Y register
// Load byte from memory into Y register.
constexpr static uint8_t INSTR_6502_LDY_IMMEDIATE = 0xA0;  // 2
constexpr static uint8_t INSTR_6502_LDY_ZEROPAGE = 0xA4;   // 3
constexpr static uint8_t INSTR_6502_LDY_ZEROPAGE_X = 0xB4; // 4
constexpr static uint8_t INSTR_6502_LDY_ABSOLUTE = 0xAC;   // 4
constexpr static uint8_t INSTR_6502_LDY_ABSOLUTE_X = 0xBC; // 4+

// STA - STore A in memory
constexpr static uint8_t INSTR_6502_STA_ZEROPAGE = 0x85;   // 3
constexpr static uint8_t INSTR_6502_STA_ZEROPAGE_X = 0x95; // 4
constexpr static uint8_t INSTR_6502_STA_ABSOLUTE = 0x8D;   // 4
constexpr static uint8_t INSTR_6502_STA_ABSOLUTE_X = 0x9D; // 5
constexpr static uint8_t INSTR_6502_STA_ABSOLUTE_Y = 0x99; // 5
constexpr static uint8_t INSTR_6502_STA_INDIRECT_X = 0x81; // 6
constexpr static uint8_t INSTR_6502_STA_INDIRECT_Y = 0x91; // 6

// STX - STore X in memory
constexpr static uint8_t INSTR_6502_STX_ZEROPAGE = 0x86;   // 3
constexpr static uint8_t INSTR_6502_STX_ZEROPAGE_Y = 0x96; // 4
constexpr static uint8_t INSTR_6502_STX_ABSOLUTE = 0x8E;   // 4

// STY - STore Y in memory
constexpr static uint8_t INSTR_6502_STY_ZEROPAGE = 0x84;   // 3
constexpr static uint8_t INSTR_6502_STY_ZEROPAGE_X = 0x94; // 4
constexpr static uint8_t INSTR_6502_STY_ABSOLUTE = 0x8C;   // 4

// TAX - Transfer A to X
constexpr static uint8_t INSTR_6502_TAX = 0xAA; // 2

// TXA - Transfer X to A
constexpr static uint8_t INSTR_6502_TXA = 0x8A; // 2
constexpr static uint8_t INSTR_6502_TXS = 0x9A; // 2
constexpr static uint8_t INSTR_6502_TSX = 0xBA; // 2
constexpr static uint8_t INSTR_6502_TYA = 0x98; // 2
constexpr static uint8_t INSTR_6502_TAY = 0xA8; // 2

// ADC - ADd with Carry
constexpr static uint8_t INSTR_6502_ADC_IMMEDIATE = 0x69;  // 2
constexpr static uint8_t INSTR_6502_ADC_ZEROPAGE = 0x65;   // 3
constexpr static uint8_t INSTR_6502_ADC_ZEROPAGE_X = 0x75; // 4
constexpr static uint8_t INSTR_6502_ADC_ABSOLUTE = 0x6D;   // 4
constexpr static uint8_t INSTR_6502_ADC_ABSOLUTE_X = 0x7D; // 4+
constexpr static uint8_t INSTR_6502_ADC_ABSOLUTE_Y = 0x79; // 4+
constexpr static uint8_t INSTR_6502_ADC_INDIRECT_X = 0x61; // 6
constexpr static uint8_t INSTR_6502_ADC_INDIRECT_Y = 0x71; // 5+

// SBC - SuBtract with Carry
constexpr static uint8_t INSTR_6502_SBC_IMMEDIATE = 0xE9;  // 2
constexpr static uint8_t INSTR_6502_SBC_ZEROPAGE = 0xE5;   // 3
constexpr static uint8_t INSTR_6502_SBC_ZEROPAGE_X = 0xF5; // 4
constexpr static uint8_t INSTR_6502_SBC_ABSOLUTE = 0xED;   // 4
constexpr static uint8_t INSTR_6502_SBC_ABSOLUTE_X = 0xFD; // 4+
constexpr static uint8_t INSTR_6502_SBC_ABSOLUTE_Y = 0xF9; // 4+
constexpr static uint8_t INSTR_6502_SBC_INDIRECT_X = 0xE1; // 6
constexpr static uint8_t INSTR_6502_SBC_INDIRECT_Y = 0xF1; // 5+

// PLP - PuLl Processor flags from stack
constexpr static uint8_t INSTR_6502_PLP = 0x28; // 4

// INX - INcrement X
constexpr static uint8_t INSTR_6502_INX = 0xE8; // 2

// INY - INcrement Y register
constexpr static uint8_t INSTR_6502_INY = 0xC8;

// DEX - DEcrement X
constexpr static uint8_t INSTR_6502_DEX = 0xCA; // 2

// DEY - DEcrement Y register
constexpr static uint8_t INSTR_6502_DEY = 0x88;

// CPX - ComPare X register
constexpr static uint8_t INSTR_6502_CPX_IMMEDIATE = 0xE0; // 2
constexpr static uint8_t INSTR_6502_CPX_ZEROPAGE = 0xE4;  // 3
constexpr static uint8_t INSTR_6502_CPX_ABSOLUTE = 0xEC;  // 4

// CPY - ComPare Y register
constexpr static uint8_t INSTR_6502_CPY_IMMEDIATE = 0xC0; // 2
constexpr static uint8_t INSTR_6502_CPY_ZEROPAGE = 0xC4;  // 3
constexpr static uint8_t INSTR_6502_CPY_ABSOLUTE = 0xCC;  // 4

// BEQ - Branch if EQual
constexpr static uint8_t INSTR_6502_BEQ_RELATIVE = 0xF0; // 2+++

// BNE - Branch if Not Equal
constexpr static uint8_t INSTR_6502_BNE_RELATIVE = 0xD0; // 2+++

// BMI - Branch if MInus
constexpr static uint8_t INSTR_6502_BMI_RELATIVE = 0x30; // 2+++

// BPL - Branch if Positive
constexpr static uint8_t INSTR_6502_BPL_RELATIVE = 0x10; // 2+++

// BVS - Branch if oVerflow Set
constexpr static uint8_t INSTR_6502_BVS_RELATIVE = 0x70; // 2+++

// BVC - Branch if oVerflow Clear
constexpr static uint8_t INSTR_6502_BVC_RELATIVE = 0x50; // 2+++

// BCS - Branch if Carry Set
constexpr static uint8_t INSTR_6502_BCS_RELATIVE = 0xB0; // 2+++

// BCC - Branch if Carry Clear
constexpr static uint8_t INSTR_6502_BCC_RELATIVE = 0x90; // 2+++

// SED - SEt Decimal flag
constexpr static uint8_t INSTR_6502_SED = 0xF8; // 2

// ORA - Logical inclusive or with A.
constexpr static uint8_t INSTR_6502_ORA_IMMEDIATE = 0x09;  // 2
constexpr static uint8_t INSTR_6502_ORA_ZEROPAGE = 0x05;   // 3
constexpr static uint8_t INSTR_6502_ORA_ZEROPAGE_X = 0x15; // 4
constexpr static uint8_t INSTR_6502_ORA_ABSOLUTE = 0x0D;   // 4
constexpr static uint8_t INSTR_6502_ORA_ABSOLUTE_X = 0x1D; // 4+
constexpr static uint8_t INSTR_6502_ORA_ABSOLUTE_Y = 0x19; // 4+
constexpr static uint8_t INSTR_6502_ORA_INDIRECT_X = 0x01; // 6
constexpr static uint8_t INSTR_6502_ORA_INDIRECT_Y = 0x11; // 5+

// ASL - Arithmetic Shift Left
constexpr static uint8_t INSTR_6502_ASL_ACCUMULATOR = 0x0A; // 2
constexpr static uint8_t INSTR_6502_ASL_ZEROPAGE = 0x06;    // 5
constexpr static uint8_t INSTR_6502_ASL_ZEROPAGE_X = 0x16;  // 6
constexpr static uint8_t INSTR_6502_ASL_ABSOLUTE = 0x0E;    // 6
constexpr static uint8_t INSTR_6502_ASL_ABSOLUTE_X = 0x1E;  // 7

// LSR - Logical Shift Right
constexpr static uint8_t INSTR_6502_LSR_ACCUMULATOR = 0x4A; // 2
constexpr static uint8_t INSTR_6502_LSR_ZEROPAGE = 0x46;    // 5
constexpr static uint8_t INSTR_6502_LSR_ZEROPAGE_X = 0x56;  // 6
constexpr static uint8_t INSTR_6502_LSR_ABSOLUTE = 0x4E;    // 6
constexpr static uint8_t INSTR_6502_LSR_ABSOLUTE_X = 0x5E;  // 7

// ROL - ROtate Left
constexpr static uint8_t INSTR_6502_ROL_ACCUMULATOR = 0x2A; // 2
constexpr static uint8_t INSTR_6502_ROL_ZEROPAGE = 0x26;    // 5
constexpr static uint8_t INSTR_6502_ROL_ZEROPAGE_X = 0x36;  // 6
constexpr static uint8_t INSTR_6502_ROL_ABSOLUTE = 0x2E;    // 6
constexpr static uint8_t INSTR_6502_ROL_ABSOLUTE_X = 0x3E;  // 7

// ROR - ROtate Right
constexpr static uint8_t INSTR_6502_ROR_ACCUMULATOR = 0x6A; // 2
constexpr static uint8_t INSTR_6502_ROR_ZEROPAGE = 0x66;    // 5
constexpr static uint8_t INSTR_6502_ROR_ZEROPAGE_X = 0x76;  // 6
constexpr static uint8_t INSTR_6502_ROR_ABSOLUTE = 0x6E;    // 6
constexpr static uint8_t INSTR_6502_ROR_ABSOLUTE_X = 0x7E;  // 7

// BIT
constexpr static uint8_t INSTR_6502_BIT_ZEROPAGE = 0x24; // 3
constexpr static uint8_t INSTR_6502_BIT_ABSOLUTE = 0x2C; // 4

// BRK - Break
constexpr static uint8_t INSTR_6502_BRK = 0x00; // 7

// SEC - SEt Carry flag on
constexpr static uint8_t INSTR_6502_SEC = 0x38; // 2

// SEI - SEt Interrupt
constexpr static uint8_t INSTR_6502_SEI = 0x78; // 2

// CLD - CLear Decimal flag
constexpr static uint8_t INSTR_6502_CLD = 0xD8; // 2

// CLI - CLear Interrupt disable flag
constexpr static uint8_t INSTR_6502_CLI = 0x58; // 2

// CLC - CLear Carry flag
constexpr static uint8_t INSTR_6502_CLC = 0x18;

// CLV - CLear oVerflow flag
constexpr static uint8_t INSTR_6502_CLV = 0xB8;

// NOP - No OPeration, i.e. do nothing for 2 cycles
constexpr static uint8_t INSTR_6502_NOP = 0xEA;

// PHA - PusH a copy of A onto the stack, and decrement stack pointer.
constexpr static uint8_t INSTR_6502_PHA = 0x48;

// PHP - PusH Processor status onto stack.
constexpr static uint8_t INSTR_6502_PHP = 0x08;

// PLA - Pull from stack into A;
constexpr static uint8_t INSTR_6502_PLA = 0x68;

// JSR - Jump to SubRoutine
constexpr static uint8_t INSTR_6502_JSR_ABSOLUTE = 0x20;

// RTS - ReTurn from Subroutine
constexpr static uint8_t INSTR_6502_RTS = 0x60; // 6

// JMP - JuMP to address
constexpr static uint8_t INSTR_6502_JMP_ABSOLUTE = 0x4c; // 3
constexpr static uint8_t INSTR_6502_JMP_INDIRECT = 0x6c; // 5

// DEC - DECrement memory
constexpr static uint8_t INSTR_6502_DEC_ZEROPAGE = 0xC6;   // 5
constexpr static uint8_t INSTR_6502_DEC_ZEROPAGE_X = 0xD6; // 6
constexpr static uint8_t INSTR_6502_DEC_ABSOLUTE = 0xCE;   // 6
constexpr static uint8_t INSTR_6502_DEC_ABSOLUTE_X = 0xDE; // 7

// INC - INCrement memory
constexpr static uint8_t INSTR_6502_INC_ZEROPAGE = 0xE6;   // 5
constexpr static uint8_t INSTR_6502_INC_ZEROPAGE_X = 0xF6; // 6
constexpr static uint8_t INSTR_6502_INC_ABSOLUTE = 0xEE;   // 6
constexpr static uint8_t INSTR_6502_INC_ABSOLUTE_X = 0xFE; // 7

// AND - bitwise logical AND operation
constexpr static uint8_t INSTR_6502_AND_IMMEDIATE = 0x29;  // 2
constexpr static uint8_t INSTR_6502_AND_ZEROPAGE_X = 0x35; // 4
constexpr static uint8_t INSTR_6502_AND_ZEROPAGE = 0x25;   // 3
constexpr static uint8_t INSTR_6502_AND_ABSOLUTE = 0x2d;   // 4
constexpr static uint8_t INSTR_6502_AND_ABSOLUTE_X = 0x3d; // 4+
constexpr static uint8_t INSTR_6502_AND_ABSOLUTE_Y = 0x39; // 4+
constexpr static uint8_t INSTR_6502_AND_INDIRECT_X = 0x21; // 6
constexpr static uint8_t INSTR_6502_AND_INDIRECT_Y = 0x31; // 5+

namespace Memory
{
    std::array<uint8_t, 256 * 256> main_memory{0};

    void clear()
    {
        main_memory = {0};
    }
}

namespace Bus
{

    /** \brief Run the loaded program until it exits. */
    void run()
    { 
        auto time = std::chrono::high_resolution_clock::now();
        auto interval = std::chrono::microseconds{Cpu::microseconds_per_frame};

        while (Cpu::tick(Cpu::cycles_per_frame) != ReturnCode::BREAK)
        {
            time += interval;
            std::this_thread::sleep_until(time);
        }
    }

    void write(const uint8_t data, const uint16_t address)
    {
        Memory::main_memory[address] = data;
    }

    uint8_t read(const uint16_t address)
    {
        return Memory::main_memory[address];
    }

    bool load_rom(const std::string &filename)
    {
        std::ifstream input_file(filename, std::ios::binary);
        char buf[ROM_BUFFER_SIZE];
        input_file.read(buf, ROM_BUFFER_SIZE);
        uint8_t *buf2 = (uint8_t *)buf;
        std::memcpy(Memory::main_memory.data(), buf2, ROM_BUFFER_SIZE);
        return true;
    }
}

namespace Cpu
{
    bool C, Z, I, D, B, V, N; // CPU flags.

    int cycles_available = 0;
    bool page_crossed = false;
    uint16_t instruction_pointer = 0;
    uint16_t stack_pointer = 0;
    uint8_t A, X, Y; // Accumulator and registers.

    /** \brief Sets appropriate flags after performing LDA operations.
     *
     * Flags are set based on the current value in the accumulator.
     * The negative (N) flag is set if the value of the accumulator is negative, i.e. if bit 7 is set.
     * The zero (Z) flag is set if the value of the accumulator is zero.
     */
    void LDA_set_CPU_flags()
    {
        N = (A & BIT7);
        Z = (A == 0);
    }

    /** \brief Sets a value in memory.
     * \param data A byte of data to store in the 8-bit (zero page) address at the current instruction pointer.
     */
    void set_data_zeropage(uint8_t data)
    {
        uint8_t data_address = Bus::read(instruction_pointer);
        Bus::write(data, data_address);
    }

    /** \brief Sets a value in memory.
     * \param data A byte of data to store in the 8-bit (zero page) address at the current instruction pointer.
     * \param index Offset from the memory location read by the instruction pointer.
     */
    void set_data_zeropage(uint8_t data, uint8_t index)
    {
        uint8_t data_address = Bus::read(instruction_pointer) + index;
        Bus::write(data, data_address);
    }

    /** \brief Sets a value in memory.
     * \param data A byte of data to store in the 16-bit address at the current instruction pointer.
     */
    void set_data_absolute(const uint8_t data)
    {
        uint16_t address = get_word(instruction_pointer);
        Bus::write(data, address);
    }

    /** \brief Sets appropriate flags after performing a CMP operation.
     * \param data_from_memory The flags to set depend on the data read from memory to do the comparison.
     *
     * The data against which the flags are set is passed into this function.
     * The data is subtracted from the value in the accumulator, with the result potentially being negative.
     * The carry (C) flag is set if the result of the subtraction is greater than or equal to zero.
     * The zero (Z) flag is set if the result is equal to zero.
     * The negative (N) flag is set if the result is negative, i.e. bit 7 is set.
     */
    void CMP_set_CPU_flags(const uint8_t data_from_memory)
    {
        int difference = static_cast<int>(A - data_from_memory);
        C = (difference >= 0);
        Z = (difference == 0);
        N = (difference & BIT7);
    }

    /** \brief Sets appropriate flags after performing LDY operations.
     *
     * Flags are set based on the current value of the Y register.
     * The negative (N) flag is set if Y is negative, i.e. bit 7 is set.
     * The zero (Z) flag is set if Y is equal to zero.
     */
    void LDY_set_CPU_flags()
    {
        N = (Y & BIT7);
        Z = (Y == 0);
    }

    /** \brief Get data from memory using the (indirect),y addressing mode. Will set the page_crossed flag if a page is
     * crossed.
     * \param memory Reference to system memory.
     * \param index Index to add to address.
     * \return 8-bit value from memory.
     */
    uint8_t get_data_indirect_indexed(const uint8_t index)
    {
        // read next byte and add index without carry
        uint8_t indirect_address = Bus::read(instruction_pointer);

        // get target address from indirect_address data and next on zero page and add index
        uint16_t target_address = get_word_zpg_wrap(indirect_address);
        uint8_t page1 = static_cast<uint8_t>(target_address >> 8);
        target_address += index;
        uint8_t page2 = static_cast<uint8_t>(target_address >> 8);

        // Check if page crossed.
        if (page1 != page2)
        {
            page_crossed = true;
        }

        // get data from target address and return
        return Bus::read(target_address);
    }

    /** \brief Fetches a byte using immediate addressing mode.
     * \param memory A reference to a memory array object.
     * \return A uint8_t from memory.
     */
    uint8_t get_data_immediate()
    {
        return Bus::read(instruction_pointer);
    }

    /** \brief Get a word from the zero page with full wrapping.
     * \param memory Reference to system memory.
     * \param address The zero page address of the first byte to be read.
     * \return 16-bit value from the zero page.
     *
     * This function gets a word from the zero page in memory. Importantly, the FULL word is guaranteed to come from the
     * zero page. If the low byte is at the end of the zero page, the high byte will come from the start of the zero page.
     */
    uint16_t get_word_zpg_wrap(const uint8_t address)
    {
        uint16_t addr1 = address % 256U;
        uint16_t addr2 = (address + 1) % 256U;
        uint16_t val1 = Bus::read(addr1);
        uint16_t val2 = Bus::read(addr2);
        return (val2 << 8) | val1;
    }

    /** \brief Get data from memory using the (indirect,x) addressing mode.
     * \param memory Reference to system memory.
     * \param index Index added to address.
     * \return 8-bit value from memory.
     */
    uint8_t get_data_indexed_indirect(const uint8_t index)
    {
        // read next byte and add index without carry
        uint8_t indirect_address = Bus::read(instruction_pointer) + index;

        // get target address from indirect_address data and next on zero page
        uint16_t target_address = get_word_zpg_wrap(indirect_address);

        // get data from target address and return
        return Bus::read(target_address);
    }

    /** \brief Fetches a byte using zpg addressing mode.
     * \param memory A reference to a memory array object.
     * \return A uint8_t from memory.
     */
    uint8_t get_data_zeropage()
    {
        uint8_t data_address = Bus::read(instruction_pointer);
        return Bus::read(data_address);
    }

    /** \brief Set CPU flags following an AND operation.
     *
     * Flags are set based on the current value in the accumulator.
     * The negative (N) flag is set if the value of the accumulator is negative, i.e. if bit 7 is set.
     * The zero (Z) flag is set if the value of the accumulator is zero.
     */
    void AND_set_CPU_flags()
    {
        LDA_set_CPU_flags();
    }

    /** \brief Sets appropriate CPU flags following a DEC operation. DEC changes a value
     * in memory, and the N and Z flags are set against the new value.
     * \param data_from_memory The new value of the byte in memory.
     *
     * The negative (N) flag is set if the value is negative, i.e. if bit 7 is set.
     * The zero (Z) flag is set if the value is equal to zero.
     */
    void DEC_set_CPU_flags(const uint8_t data_from_memory)
    {
        N = (data_from_memory & BIT7); // Set N on if sign bit of result is set.
        Z = (data_from_memory == 0);   // Set Z on if result is zero.
    }

    /** \brief Sets appropriate CPU flags following an INC operation. INC changes the value
     * in memory, and the N and Z flags are set against the new value.
     * \param data_from_memory The new value of the byte in memory.
     *
     * The negative (N) flag is set if the value is negative, i.e. if bit 7 is set.
     * The zero (Z) flag is set if the value is equal to zero.
     */
    void INC_set_CPU_flags(const uint8_t data_from_memory)
    {
        DEC_set_CPU_flags(data_from_memory);
    }

    /** \brief Fetches a byte using zpg addressing mode with an index, typically X or Y register.
     * \param memory A reference to a memory array object.
     * \param index An index into a memory region.
     * \return A uint8_t from memory.
     */
    uint8_t get_data_zeropage(const uint8_t index)
    {
        uint8_t data_address = Bus::read(instruction_pointer) + index;
        return Bus::read(data_address);
    }

    /** \brief Get data byte from memory using absolute addressing, with data addressed by current instruction pointer.
     * \param memory Reference to system memory.
     * \return 8-bit value from memory.
     */
    uint8_t get_data_absolute()
    {
        // get address from next two bytes and add index
        uint16_t address = Bus::read(instruction_pointer);
        // return data at address
        return Bus::read(address);
    }

    /** \brief Get data byte from memory using absolute addressing, with data addressed by current instruction pointer and
     * an index.
     * \param memory Reference to system memory.
     * \param index A byte to add to the address to be read from.
     * \return 8-bit value from memory.
     */
    uint8_t get_data_absolute(const uint8_t index)
    {
        // get address from next two bytes and add index
        uint16_t address = get_word(instruction_pointer);
        uint8_t page1 = static_cast<uint8_t>(address >> 8);
        address += index;
        uint8_t page2 = static_cast<uint8_t>(address >> 8);

        // Check for page crossing for extra cycle.
        if (page1 != page2)
        {
            page_crossed = true;
        }

        // return data at address
        return Bus::read(address);
    }

    /** \brief Sets appropriate CPU flags following an EOR operation.
     *
     * The negative (N) flag is set if the value of the accumulator is negative, i.e. if bit 7 is set.
     * The zero (Z) flag is set if the value of the accumulator is zero.
     */
    void EOR_set_CPU_flags()
    {
        LDA_set_CPU_flags();
    }

    /** \brief Set CPU flags following INX operaiton.
     *
     * Flags are set based on the current value of the X register.
     * The negative (N) flag is set if X is negative, i.e. bit 7 is set.
     * The zero (Z) flag is set if X is equal to zero.
     */
    void INX_set_CPU_flags()
    {
        LDX_set_CPU_flags();
    }

    /** \brief Set CPU flags following CPX operation.
     * \param data The setting of the flags depends upon some non-stored calculation result.
     *
     * Flags are set based on the non-stored result of the calculation, which is passed to this function.
     * The carry (C) flag is set if the value is greater than or equal to zero.
     * The zero (Z) flag is set if the value is equal to zero.
     * The negative (N) flag is set if bit 7 of the value is equal to one; this is the sign bit if the value is interpreted as a signed.
     */
    void CPX_set_CPU_flags(const int data)
    {
        C = (data >= 0);
        Z = (data == 0);
        N = (data & BIT7);
    }

    /** \brief Performs subtracton of accumulator and data, setting the carry bit as required.
     * \param data A byte of data to be subtracted from the accumulator.
     * \return A byte to be stored in the accumulator.
     */
    uint8_t sub_with_carry(const uint8_t data)
    {
        return add_with_carry(~data);
    }

    /** \brief Encode all CPU flags into a single byte.
     * \return 8-bit value containing all CPU flags.
     */
    uint8_t flags_as_byte()
    {
        return static_cast<uint8_t>(
            (N << 7) | (V << 6) | (true << 5) | (B << 4) | (D << 3) | (I << 2) | (Z << 1) | (C << 0));
    }

    /** \brief Performs addition of accumulator and data, setting the carry bit as required.
     * \param data A byte of data to be added to the accumulator.
     * \return A byte to be stored in the accumulator.
     */
    uint8_t add_with_carry(const uint8_t data)
    {
        // TODO Not at all confident in this implementation, esp. V = ...
        // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
        uint16_t result = static_cast<uint16_t>(data + A + C);
        Z = (result == 0);
        C = (result > 255);
        N = (result & BIT7);
        V = ((A ^ result) & (data ^ result) & BIT7) != 0;
        return static_cast<uint8_t>(result & 0xFF);
    }

    /** \brief Moves the stack pointer up by one position and returns the value found at that address.
     * \param memory Reference to system memory.
     * \return A byte from the stack.
     */
    uint8_t pop_from_stack()
    {
        stack_pointer++;
        return Bus::read(stack_pointer);
    }

    /** \brief Sets appropriuate flags after performing ORA operation.
     *
     * The negative (N) flag is set if the value of the accumulator is negative, i.e. if bit 7 is set.
     * The zero (Z) flag is set if the value of the accumulator is zero.
     */
    void ORA_set_CPU_flags()
    {
        LDA_set_CPU_flags();
    }

    /** \brief Adds the signed value distance to the IP.
     * \param distance Signed value defining the distance to jump in memory.
     */
    void branch_relative(uint8_t distance)
    {
        if (distance & BIT7)
        {
            // If jump is negative get the two's complement and subtract the result.
            distance = ~distance;
            distance += 1;
            instruction_pointer = instruction_pointer - distance;
        }
        else
        {
            // If the jump is positive, do the jump.
            instruction_pointer = instruction_pointer + distance;
        }
    }

    /** \brief Fetches a byte using relative addressing mode.
     * \param memory A reference to a memory array object.
     * \return A uint8_t from memory.
     */
    uint8_t get_data_relative()
    {
        return get_data_immediate();
    }

    /** \brief Set CPU flags following CPY operation.
     * \param data The setting of the flags depends upon some non-stored calculation result.
     *
     * Flags are set based on the non-stored result of the calculation, which is passed to this function.
     * The carry (C) flag is set if the value is greater than or equal to zero.
     * The zero (Z) flag is set if the value is equal to zero.
     * The negative (N) flag is set if bit 7 of the value is equal to one; this is the sign bit if the value is interpreted as a signed.
     */
    void CPY_set_CPU_flags(const int data)
    {
        CPX_set_CPU_flags(data);
    }

    /** Set CPU flagas following DEY operation.
     *
     *
     * Flags are set based on the current value of the Y register.
     * The negative (N) flag is set if Y is negative, i.e. bit 7 is set.
     * The zero (Z) flag is set if Y is equal to zero.
     */
    void DEY_set_CPU_flags()
    {
        LDY_set_CPU_flags();
    }

    /** \brief Set CPU flags following DEX operation.
     *
     * Flags are set based on the current value of the X register.
     * The negative (N) flag is set if X is negative, i.e. bit 7 is set.
     * The zero (Z) flag is set if X is equal to zero.
     */
    void DEX_set_CPU_flags()
    {
        LDX_set_CPU_flags();
    }

    /** \brief Set CPU flags following INY operation.
     *
     *
     * Flags are set based on the current value of the Y register.
     * The negative (N) flag is set if Y is negative, i.e. bit 7 is set.
     * The zero (Z) flag is set if Y is equal to zero.
     */
    void INY_set_CPU_flags()
    {
        LDY_set_CPU_flags();
    }

    /** \brief Sets appropriate flags after performing TXA operation.
     *
     * Flags are set based on the current value in the accumulator.
     * The negative (N) flag is set if the value of the accumulator is negative, i.e. if bit 7 is set.
     * The zero (Z) flag is set if the value of the accumulator is zero.
     */
    void TXA_set_CPU_flags()
    {
        LDA_set_CPU_flags();
    }

    /** \brief Gets a full word from memory, addressed by the current instruction pointer.
     * \param memory Reference to system memory.
     * \return 16-bit value from memory.
     */
    uint16_t get_word(uint16_t address)
    {
        uint16_t val1 = static_cast<uint16_t>(Bus::read(address));
        uint16_t val2 = static_cast<uint16_t>(Bus::read(address + 1));
        return static_cast<uint16_t>(val2 << 8) | val1;
    }

    /** \brief Sets appropriate flags after performing LDX operations.
     *
     * Flags are set based on the current value of the X register.
     * The negative (N) flag is set if X is negative, i.e. bit 7 is set.
     * The zero (Z) flag is set if X is equal to zero.
     */
    void LDX_set_CPU_flags()
    {
        N = (X & BIT7);
        Z = (X == 0);
    }

    /** \brief Sets appropriate flags after performing TAX operation.
     *
     * Flags are set based on the current value of the X register.
     * The negative (N) flag is set if X is negative, i.e. bit 7 is set.
     * The zero (Z) flag is set if X is equal to zero.
     */
    void TAX_set_CPU_flags()
    {
        LDX_set_CPU_flags();
    }

    /** \brief Sets a value in memory.
     * \param memory Reference to system memory.
     * \param data A byte of data to store in memory.
     * \param index Memory location offset.
     */
    void set_data_indirect_indexed(const uint8_t data, const uint8_t index)
    {
        uint8_t indirect_address = Bus::read(instruction_pointer);
        uint16_t target_address = get_word_zpg_wrap(indirect_address) + index;
        Bus::write(data, target_address);
    }

    /** \brief Set a value in memory using (indirect, x) addressing.
     * \param memory Reference to system memory.
     * \param data uint8_t of data to store in memory.
     * \param index Offset of memory location.
     */
    void set_data_indexed_indirect(uint8_t data, uint8_t index)
    {
        uint8_t indirect_address = Bus::read(instruction_pointer) + index;
        uint16_t target_address = get_word_zpg_wrap(indirect_address);
        Bus::write(data, target_address);
    }

    /** \brief Sets a value in memory.
     * \param data A byte of data to store in the 16-bit address at the current instruction pointer.
     * \param index An offset from the specified memory location.
     */
    void set_data_absolute(uint8_t data, uint8_t index)
    {
        uint16_t address = get_word(instruction_pointer) + index;
        Bus::write(data, address);
    }

    ReturnCode tick(const int cycles_to_add)
    {
        cycles_available += cycles_to_add;

        while (cycles_available > 0)
        {
            // Reset the page crossing flag in case it was left on from the last iteration.
            page_crossed = false;

            // TODO Interrupt handler should go here.

            // Grab an instruction from RAM.
            uint8_t instruction = Bus::read(instruction_pointer);

            // We increment the instruction pointer to point to the next byte in memory.
            instruction_pointer++;

            LOG(
                "N" << N << " " << "V" << V << " " << "B" << B << " " << "D" << D << " " << "I" << I << " " << "Z" << Z
                    << " " << "C" << C << "    " << std::hex << "IP:" << std::setw(4) << (int)instruction_pointer << "   " << "SP:"
                    << std::setw(4) << (int)stack_pointer << "   " << "A:" << std::setw(2) << (int)A << "   " << "X:" << std::setw(2)
                    << (int)X << "   " << "Y:" << std::setw(2) << (int)Y << "   " << instruction_names[instruction]);

            switch (instruction)
            {
            case INSTR_6502_LDA_IMMEDIATE:
                A = Bus::read(instruction_pointer);
                instruction_pointer++;
                LDA_set_CPU_flags();
                cycles_available -= 2;
                break;

            case INSTR_6502_LDA_ZEROPAGE:
                A = get_data_zeropage();
                instruction_pointer++;
                LDA_set_CPU_flags();
                cycles_available -= 3;
                break;

            case INSTR_6502_LDA_ZEROPAGE_X:
                A = get_data_zeropage(X);
                instruction_pointer++;
                LDA_set_CPU_flags();
                cycles_available -= 4;
                break;

            case INSTR_6502_LDA_ABSOLUTE:
                A = get_data_absolute();
                instruction_pointer++;
                instruction_pointer++;
                LDA_set_CPU_flags();
                cycles_available -= 4;
                break;

            case INSTR_6502_LDA_ABSOLUTE_X:
                A = get_data_absolute(X);
                instruction_pointer++;
                instruction_pointer++;
                LDA_set_CPU_flags();
                cycles_available -= 4;

                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_LDA_ABSOLUTE_Y:
                A = get_data_absolute(Y);
                instruction_pointer++;
                instruction_pointer++;
                LDA_set_CPU_flags();
                cycles_available -= 4;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_LDA_INDIRECT_X:
                A = get_data_indexed_indirect(X);
                instruction_pointer++;
                LDA_set_CPU_flags();
                cycles_available -= 6;
                break;

            case INSTR_6502_LDA_INDIRECT_Y:
                A = get_data_indirect_indexed(Y);
                instruction_pointer++;
                LDA_set_CPU_flags();
                cycles_available -= 5;

                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_LDY_IMMEDIATE:
                Y = Bus::read(instruction_pointer);
                instruction_pointer++;
                LDY_set_CPU_flags();
                cycles_available -= 2;
                break;

            case INSTR_6502_LDY_ZEROPAGE:
                Y = get_data_zeropage();
                instruction_pointer++;
                LDY_set_CPU_flags();
                cycles_available -= 3;
                break;

            case INSTR_6502_LDY_ZEROPAGE_X:
                Y = get_data_zeropage(X);
                instruction_pointer++;
                LDY_set_CPU_flags();
                cycles_available -= 4;
                break;

            case INSTR_6502_LDY_ABSOLUTE:
                Y = get_data_absolute();
                instruction_pointer++;
                instruction_pointer++;
                LDY_set_CPU_flags();
                cycles_available -= 4;
                break;

            case INSTR_6502_LDY_ABSOLUTE_X:
                Y = get_data_absolute(X);
                instruction_pointer++;
                instruction_pointer++;
                LDY_set_CPU_flags();
                cycles_available -= 4;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_CMP_IMMEDIATE:
            {
                uint8_t data = get_data_immediate();
                instruction_pointer++;
                CMP_set_CPU_flags(data);
                cycles_available -= 2;
            }
            break;

            case INSTR_6502_CMP_ZEROPAGE:
            {
                uint8_t data = get_data_zeropage();
                instruction_pointer++;
                CMP_set_CPU_flags(data);
                cycles_available -= 3;
            }
            break;

            case INSTR_6502_CMP_ZEROPAGE_X:
            {
                uint8_t data = get_data_zeropage(X);
                instruction_pointer++;
                CMP_set_CPU_flags(data);
                cycles_available -= 4;
            }
            break;

            case INSTR_6502_CMP_ABSOLUTE:
            {
                uint8_t data = get_data_absolute();
                instruction_pointer++;
                instruction_pointer++;
                CMP_set_CPU_flags(data);
                cycles_available -= 4;
            }
            break;

            case INSTR_6502_CMP_ABSOLUTE_X:
            {
                uint8_t data = get_data_absolute(X);
                instruction_pointer++;
                instruction_pointer++;
                CMP_set_CPU_flags(data);
                cycles_available -= 4;
                if (page_crossed)
                {
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_CMP_ABSOLUTE_Y:
            {
                uint8_t data = get_data_absolute(Y);
                instruction_pointer++;
                instruction_pointer++;
                CMP_set_CPU_flags(data);
                cycles_available -= 4;
                if (page_crossed)
                {
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_CMP_INDIRECT_X:
            {
                uint8_t data = get_data_indexed_indirect(X);
                instruction_pointer++;
                CMP_set_CPU_flags(data);
                cycles_available -= 6;
            }
            break;

            case INSTR_6502_CMP_INDIRECT_Y:
            {
                uint8_t data = get_data_indirect_indexed(Y);
                instruction_pointer++;
                CMP_set_CPU_flags(data);
                cycles_available -= 5;
                if (page_crossed)
                {
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_EOR_IMMEDIATE:
                A = A ^ get_data_immediate();
                instruction_pointer++;
                EOR_set_CPU_flags();
                cycles_available -= 2;
                break;

            case INSTR_6502_EOR_ZEROPAGE:
                A = A ^ get_data_zeropage();
                instruction_pointer++;
                EOR_set_CPU_flags();
                cycles_available -= 3;
                break;

            case INSTR_6502_EOR_ZEROPAGE_X:
                A = A ^ get_data_zeropage(X);
                instruction_pointer++;
                EOR_set_CPU_flags();
                cycles_available -= 4;
                break;

            case INSTR_6502_EOR_ABSOLUTE:
                A = A ^ get_data_absolute();
                instruction_pointer++;
                instruction_pointer++;
                EOR_set_CPU_flags();
                cycles_available -= 4;
                break;

            case INSTR_6502_EOR_ABSOLUTE_X:
                A = A ^ get_data_absolute(X);
                instruction_pointer++;
                instruction_pointer++;
                EOR_set_CPU_flags();
                cycles_available -= 4;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_EOR_ABSOLUTE_Y:
                A = A ^ get_data_absolute(Y);
                instruction_pointer++;
                instruction_pointer++;
                EOR_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_EOR_INDIRECT_X:
            {
                A = A ^ get_data_indexed_indirect(X);
                instruction_pointer++;
                EOR_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_EOR_INDIRECT_Y:
                A = A ^ get_data_indirect_indexed(Y);
                instruction_pointer++;
                EOR_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_STA_ZEROPAGE:
                set_data_zeropage(A);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STA_ZEROPAGE_X:
                set_data_zeropage(A, X);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STA_ABSOLUTE:
                set_data_absolute(A);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STA_ABSOLUTE_X:
                set_data_absolute(A, X);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STA_ABSOLUTE_Y:
                set_data_absolute(A, Y);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STA_INDIRECT_X:
                set_data_indexed_indirect(A, X);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STA_INDIRECT_Y:
                set_data_indirect_indexed(A, Y);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_TXS:
                stack_pointer = static_cast<uint16_t>(0x0100 | X);
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_TSX:
                X = static_cast<uint8_t>(stack_pointer & 0x00FF);
                Z = (X == 0);
                N = (X & BIT7);
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_TYA:
                A = Y;
                Z = (A == 0);
                N = (A & BIT7);
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STX_ZEROPAGE:
                set_data_zeropage(X);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STX_ZEROPAGE_Y:
                set_data_absolute(X, Y);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STX_ABSOLUTE:
                set_data_absolute(X);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STY_ZEROPAGE:
                set_data_zeropage(Y);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STY_ZEROPAGE_X:
                set_data_zeropage(Y, X);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_STY_ABSOLUTE:
                set_data_absolute(Y);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_TAX:
                X = A;
                TAX_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_TAY:
                Y = A;
                Z = (Y == 0);
                N = (Y & BIT7);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_TXA:
                A = X;
                TXA_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_INX:
                X++;
                INX_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_INY:
                Y++;
                INY_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_LDX_IMMEDIATE:
                X = get_data_immediate();
                instruction_pointer++;
                LDX_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_LDX_ZEROPAGE:
                X = get_data_zeropage();
                instruction_pointer++;
                LDX_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_LDX_ZEROPAGE_Y:
                X = get_data_zeropage(Y);
                instruction_pointer++;
                LDX_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_LDX_ABSOLUTE:
                X = get_data_absolute();
                instruction_pointer++;
                instruction_pointer++;
                LDX_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_LDX_ABSOLUTE_Y:
                X = get_data_absolute(Y);
                instruction_pointer++;
                instruction_pointer++;
                LDX_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_DEX:
                X--;
                DEX_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_DEY:
                Y--;
                DEY_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_CPX_IMMEDIATE:
            {
                int result = X - get_data_immediate();
                instruction_pointer++;
                CPX_set_CPU_flags(result);
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_CPX_ZEROPAGE:
            {
                int result = X - get_data_zeropage();
                instruction_pointer++;
                CPX_set_CPU_flags(result);
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_CPX_ABSOLUTE:
            {
                int result = X - get_data_absolute();
                instruction_pointer++;
                CPX_set_CPU_flags(result);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_CPY_IMMEDIATE:
            {
                int result = Y - get_data_immediate();
                CPY_set_CPU_flags(result);
                cycles_available--;
                cycles_available--;
                instruction_pointer++;
            }
            break;

            case INSTR_6502_CPY_ZEROPAGE:
            {
                int result = Y - get_data_zeropage();
                CPY_set_CPU_flags(result);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                instruction_pointer++;
            }
            break;

            case INSTR_6502_CPY_ABSOLUTE:
            {
                int result = Y - get_data_absolute();
                CPY_set_CPU_flags(result);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                instruction_pointer++;
            }
            break;

            case INSTR_6502_BNE_RELATIVE:
            {
                // Remember the starting page so we know if we've moved to a new page.
                uint8_t current_page = static_cast<uint8_t>(instruction_pointer >> 8);
                cycles_available--;
                cycles_available--;

                if (!Z)
                {
                    uint8_t distance = get_data_relative();
                    branch_relative(distance);
                    cycles_available--;
                }
                instruction_pointer++;

                // This should take two additional clock cycles if the branch leads to a new page.
                uint8_t new_page = static_cast<uint8_t>(instruction_pointer >> 8);
                if (current_page != new_page)
                {
                    cycles_available--;
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_BEQ_RELATIVE:
            {
                // Remember the starting page so we know if we've moved to a new page.
                uint8_t current_page = static_cast<uint8_t>(instruction_pointer >> 8);
                cycles_available--;
                cycles_available--;

                if (Z)
                {
                    uint8_t dist = get_data_relative();
                    branch_relative(dist);
                    cycles_available--;
                }
                instruction_pointer++;

                // This should take two additional clock cycles if the branch leads to a new page.
                uint8_t new_page = static_cast<uint8_t>(instruction_pointer >> 8);
                if (current_page != new_page)
                {
                    cycles_available--;
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_BMI_RELATIVE:
            {
                uint8_t current_page = static_cast<uint8_t>(instruction_pointer >> 8);
                cycles_available--;
                cycles_available--;

                if (N)
                {
                    uint8_t dist = get_data_relative();
                    branch_relative(dist);
                    cycles_available--;
                }
                instruction_pointer++;

                uint8_t new_page = static_cast<uint8_t>(instruction_pointer >> 8);
                if (current_page != new_page)
                {
                    cycles_available--;
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_BPL_RELATIVE:
            {
                uint8_t current_page = static_cast<uint8_t>(instruction_pointer >> 8);
                cycles_available--;
                cycles_available--;

                if (!N)
                {
                    uint8_t dist = get_data_relative();
                    branch_relative(dist);
                    cycles_available--;
                }
                instruction_pointer++;

                uint8_t new_page = static_cast<uint8_t>(instruction_pointer >> 8);
                if (current_page != new_page)
                {
                    cycles_available--;
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_BVC_RELATIVE:
            {
                uint8_t current_page = static_cast<uint8_t>(instruction_pointer >> 8);
                cycles_available--;
                cycles_available--;

                if (!V)
                {
                    uint8_t dist = get_data_relative();
                    branch_relative(dist);
                    cycles_available--;
                }
                instruction_pointer++;

                uint8_t new_page = static_cast<uint8_t>(instruction_pointer >> 8);
                if (current_page != new_page)
                {
                    cycles_available--;
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_BVS_RELATIVE:
            {
                uint8_t current_page = static_cast<uint8_t>(instruction_pointer >> 8);
                cycles_available--;
                cycles_available--;

                if (V)
                {
                    uint8_t dist = get_data_relative();
                    branch_relative(dist);
                    cycles_available--;
                }
                instruction_pointer++;

                uint8_t new_page = static_cast<uint8_t>(instruction_pointer >> 8);
                if (current_page != new_page)
                {
                    cycles_available--;
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_BCC_RELATIVE:
            {
                uint8_t current_page = static_cast<uint8_t>(instruction_pointer >> 8);
                cycles_available--;
                cycles_available--;

                if (!C)
                {
                    uint8_t dist = get_data_relative();
                    branch_relative(dist);
                    cycles_available--;
                }
                instruction_pointer++;

                uint8_t new_page = static_cast<uint8_t>(instruction_pointer >> 8);
                if (current_page != new_page)
                {
                    cycles_available--;
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_BCS_RELATIVE:
            {
                uint8_t current_page = static_cast<uint8_t>(instruction_pointer >> 8);
                cycles_available--;
                cycles_available--;

                if (C)
                {
                    uint8_t dist = get_data_relative();
                    branch_relative(dist);
                    cycles_available--;
                }
                instruction_pointer++;

                uint8_t new_page = static_cast<uint8_t>(instruction_pointer >> 8);
                if (current_page != new_page)
                {
                    cycles_available--;
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_SED:
                D = true;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_ORA_IMMEDIATE:
                A |= get_data_immediate();
                instruction_pointer++;
                ORA_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_ORA_ZEROPAGE:
                A |= get_data_zeropage();
                instruction_pointer++;
                ORA_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_ORA_ZEROPAGE_X:
                A |= get_data_zeropage(X);
                instruction_pointer++;
                ORA_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_ORA_ABSOLUTE:
                A |= get_data_absolute();
                instruction_pointer++;
                instruction_pointer++;
                ORA_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_ORA_ABSOLUTE_X:
                A |= get_data_absolute(X);
                instruction_pointer++;
                instruction_pointer++;
                ORA_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_ORA_ABSOLUTE_Y:
                A |= get_data_absolute(Y);
                instruction_pointer++;
                instruction_pointer++;
                ORA_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_ORA_INDIRECT_X:
                A |= get_data_indexed_indirect(X);
                instruction_pointer++;
                ORA_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_ORA_INDIRECT_Y:
                A |= get_data_indirect_indexed(Y);
                instruction_pointer++;
                ORA_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_BIT_ZEROPAGE:
            {
                uint8_t result = A & get_data_zeropage();
                instruction_pointer++;
                Z = (result == 0);
                V = (result & BIT6);
                N = (result & BIT7);
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_BIT_ABSOLUTE:
            {
                uint8_t result = A & get_data_absolute();
                instruction_pointer++;
                instruction_pointer++;
                Z = (result == 0);
                V = (result & BIT6);
                N = (result & BIT7);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ASL_ACCUMULATOR:
                C = (A & BIT7);
                A = static_cast<uint8_t>(A << 1);
                Z = (A == 0);
                N = (A & BIT7);
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_ASL_ZEROPAGE:
            {
                uint8_t data = get_data_zeropage();
                C = (data & BIT7);
                data = static_cast<uint8_t>(data << 1);
                Z = (data == 0);
                N = (data & BIT7);
                set_data_zeropage(data);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ASL_ZEROPAGE_X:
            {
                uint8_t data = get_data_zeropage(X);
                C = (data & BIT7);
                data = static_cast<uint8_t>(data << 1);
                Z = (data == 0);
                N = (data & BIT7);
                set_data_zeropage(data, X);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ASL_ABSOLUTE:
            {
                uint8_t data = get_data_absolute();
                C = (data & BIT7);
                data = static_cast<uint8_t>(data << 1);
                Z = (data == 0);
                N = (data & BIT7);
                set_data_absolute(data);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ASL_ABSOLUTE_X:
            {
                uint8_t data = get_data_absolute(X);
                C = (data & BIT7);
                data = static_cast<uint8_t>(data << 1);
                Z = (data == 0);
                N = (data & BIT7);
                set_data_absolute(data, X);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_LSR_ACCUMULATOR:
                C = (A & BIT7);
                A = A >> 1;
                Z = (A == 0);
                N = (A & BIT7);
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_LSR_ZEROPAGE:
            {
                uint8_t data = get_data_zeropage();
                C = (data & BIT7);
                data = data >> 1;
                Z = (data == 0);
                N = (data & BIT7);
                set_data_zeropage(data);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_LSR_ZEROPAGE_X:
            {
                uint8_t data = get_data_zeropage(X);
                C = (data & BIT7);
                data = data >> 1;
                Z = (data == 0);
                N = (data & BIT7);
                set_data_zeropage(data, X);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_LSR_ABSOLUTE:
            {
                uint8_t data = get_data_absolute();
                C = (data & BIT7);
                data = data >> 1;
                Z = (data == 0);
                N = (data & BIT7);
                set_data_absolute(data);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_LSR_ABSOLUTE_X:
            {
                uint8_t data = get_data_absolute(X);
                C = (data & BIT7);
                data = data >> 1;
                Z = (data == 0);
                N = (data & BIT7);
                set_data_absolute(data, X);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ROL_ACCUMULATOR:
            {
                uint8_t tempC = static_cast<uint8_t>(C);
                C = (A & BIT7);
                A = static_cast<uint8_t>((A << 1) | tempC);
                Z = (A == 0);
                N = (A & BIT7);
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ROL_ZEROPAGE:
            {
                uint8_t data = get_data_zeropage();
                uint8_t tempC = static_cast<uint8_t>(C);
                C = (data & BIT7);
                data = static_cast<uint8_t>((data << 1) | tempC);
                Z = (data == 0);
                N = (data & BIT7);
                set_data_zeropage(data);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ROL_ZEROPAGE_X:
            {
                uint8_t data = get_data_zeropage(X);
                uint8_t tempC = static_cast<uint8_t>(C);
                C = (data & BIT7);
                data = static_cast<uint8_t>((data << 1) | tempC);
                Z = (data == 0);
                N = (data & BIT7);
                set_data_zeropage(data, X);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ROL_ABSOLUTE:
            {
                uint8_t data = get_data_absolute();
                uint8_t tempC = static_cast<uint8_t>(C);
                C = (data & BIT7);
                data = static_cast<uint8_t>((data << 1) | tempC);
                Z = (data == 0);
                N = (data & BIT7);
                set_data_absolute(data);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ROL_ABSOLUTE_X:
            {
                uint8_t data = get_data_absolute(X);
                uint8_t tempC = static_cast<uint8_t>(C);
                C = (data & BIT7);
                data = static_cast<uint8_t>((data << 1) | tempC);
                Z = (data == 0);
                N = (data & BIT7);
                set_data_absolute(data, X);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ROR_ACCUMULATOR:
            {
                uint8_t tempC = static_cast<uint8_t>(C << 7);
                C = (A & BIT0);
                A = (A >> 1) | tempC;
                Z = (A == 0);
                N = (A & BIT7);
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ROR_ZEROPAGE:
            {
                uint8_t data = get_data_zeropage();
                uint8_t tempC = static_cast<uint8_t>(C << 7);
                C = (data & BIT0);
                data = (data >> 1) | tempC;
                Z = (data == 0);
                N = (data & BIT7);
                set_data_zeropage(data);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ROR_ZEROPAGE_X:
            {
                uint8_t data = get_data_zeropage(X);
                uint8_t tempC = static_cast<uint8_t>(C << 7);
                C = (data & BIT0);
                data = (data >> 1) | tempC;
                Z = (data == 0);
                N = (data & BIT7);
                set_data_zeropage(data, X);
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ROR_ABSOLUTE:
            {
                uint8_t data = get_data_absolute();
                uint8_t tempC = static_cast<uint8_t>(C << 7);
                C = (data & BIT0);
                data = (data >> 1) | tempC;
                Z = (data == 0);
                N = (data & BIT7);
                set_data_absolute(data);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ROR_ABSOLUTE_X:
            {
                uint8_t data = get_data_absolute(X);
                uint8_t tempC = static_cast<uint8_t>(C << 7);
                C = (data & BIT0);
                data = (data >> 1) | tempC;
                Z = (data == 0);
                N = (data & BIT7);
                set_data_absolute(data, X);
                instruction_pointer++;
                instruction_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_PLP:
            {
                uint8_t flags = pop_from_stack();
                N = (flags >> 7) & BIT1;
                V = (flags >> 6) & BIT1;
                B = (flags >> 4) & BIT1;
                D = (flags >> 3) & BIT1;
                I = (flags >> 2) & BIT1;
                Z = (flags >> 1) & BIT1;
                C = (flags >> 0) & BIT1;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_SEC:
                C = true;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_SEI:
                I = true;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_ADC_IMMEDIATE:
            {
                // TODO: Explain this. What I've done is mostly based on
                // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
                uint8_t data = get_data_immediate();
                instruction_pointer++;
                A = add_with_carry(data);
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ADC_ZEROPAGE:
            {
                uint8_t data = get_data_zeropage();
                instruction_pointer++;
                A = add_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ADC_ZEROPAGE_X:
            {
                uint8_t data = get_data_zeropage(X);
                instruction_pointer++;
                A = add_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ADC_ABSOLUTE:
            {
                uint8_t data = get_data_absolute();
                instruction_pointer++;
                instruction_pointer++;
                A = add_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ADC_ABSOLUTE_X:
            {
                uint8_t data = get_data_absolute(X);
                instruction_pointer++;
                instruction_pointer++;
                A = add_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_ADC_ABSOLUTE_Y:
            {
                uint8_t data = get_data_absolute(Y);
                instruction_pointer++;
                instruction_pointer++;
                A = add_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_ADC_INDIRECT_X:
            {
                uint8_t data = get_data_indexed_indirect(X);
                instruction_pointer++;
                A = add_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_ADC_INDIRECT_Y:
            {
                uint8_t data = get_data_indirect_indexed(Y);
                instruction_pointer++;
                A = add_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_SBC_IMMEDIATE:
            {
                uint8_t data = get_data_immediate();
                instruction_pointer++;
                A = sub_with_carry(data);
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_SBC_ZEROPAGE:
            {
                uint8_t data = get_data_zeropage();
                instruction_pointer++;
                A = sub_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_SBC_ZEROPAGE_X:
            {
                uint8_t data = get_data_zeropage(X);
                instruction_pointer++;
                A = sub_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_SBC_ABSOLUTE:
            {
                uint8_t data = get_data_absolute();
                instruction_pointer++;
                instruction_pointer++;
                A = sub_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_SBC_ABSOLUTE_X:
            {
                uint8_t data = get_data_absolute(X);
                instruction_pointer++;
                instruction_pointer++;
                A = sub_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_SBC_ABSOLUTE_Y:
            {
                uint8_t data = get_data_absolute(Y);
                instruction_pointer++;
                instruction_pointer++;
                A = sub_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_SBC_INDIRECT_X:
            {
                uint8_t data = get_data_indexed_indirect(X);
                instruction_pointer++;
                A = sub_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_SBC_INDIRECT_Y:
            {
                uint8_t data = get_data_indirect_indexed(Y);
                instruction_pointer++;
                A = sub_with_carry(data);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
            }
            break;

            case INSTR_6502_CLD:
                D = false;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_CLI:
                I = false;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_CLC:
                C = false;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_CLV:
                V = false;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_PHA:
                Bus::write(A, stack_pointer);
                stack_pointer--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_PHP:
                Bus::write(flags_as_byte(), stack_pointer);
                stack_pointer--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_PLA:
                A = Bus::read(stack_pointer);
                stack_pointer++;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                LDA_set_CPU_flags();
                break;

            case INSTR_6502_NOP:
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_BRK:
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                B = true;

                Bus::write(static_cast<uint8_t>(instruction_pointer >> 8), stack_pointer);
                stack_pointer--;
                Bus::write(static_cast<uint8_t>(instruction_pointer & 0xFF), stack_pointer);
                stack_pointer--;
                Bus::write(flags_as_byte(), stack_pointer);
                stack_pointer--;

                std::cout << "BRK reached" << std::endl;
                return ReturnCode::BREAK;

            case INSTR_6502_JSR_ABSOLUTE:
            {
                uint16_t target_address = get_word(instruction_pointer);
                instruction_pointer++;
                Bus::write(static_cast<uint8_t>(instruction_pointer >> 8), stack_pointer);
                stack_pointer--;
                Bus::write(static_cast<uint8_t>(instruction_pointer & 0xFF), stack_pointer);
                stack_pointer--;
                instruction_pointer = target_address;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_RTS:
            {
                stack_pointer++;
                uint16_t pointer = get_word(stack_pointer);
                stack_pointer++;

                instruction_pointer = pointer + 1;

                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_JMP_ABSOLUTE:
                // TODO I don't know if I'm supposed to jump to the address in memory at the IP,
                // or the address specified by that memory location.
                instruction_pointer = get_word(instruction_pointer);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_INC_ZEROPAGE:
            {
                uint8_t value = get_data_zeropage();
                value++;
                set_data_zeropage(value);
                instruction_pointer++;
                INC_set_CPU_flags(value);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_INC_ZEROPAGE_X:
            {
                uint8_t value = get_data_zeropage(X);
                value++;
                set_data_zeropage(value, X);
                instruction_pointer++;
                INC_set_CPU_flags(value);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_INC_ABSOLUTE:
            {
                uint8_t value = get_data_absolute();
                value++;
                set_data_absolute(value);
                instruction_pointer++;
                instruction_pointer++;
                INC_set_CPU_flags(value);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_INC_ABSOLUTE_X:
            {
                uint8_t value = get_data_absolute(X);
                value++;
                set_data_absolute(value, X);
                instruction_pointer++;
                instruction_pointer++;
                INC_set_CPU_flags(value);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_DEC_ZEROPAGE:
            {
                uint8_t value = get_data_zeropage();
                value--;
                set_data_zeropage(value);
                instruction_pointer++;
                DEC_set_CPU_flags(value);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_DEC_ZEROPAGE_X:
            {
                uint8_t value = get_data_zeropage(X);
                value--;
                set_data_zeropage(value, X);
                instruction_pointer++;
                DEC_set_CPU_flags(value);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_DEC_ABSOLUTE:
            {
                uint8_t value = get_data_absolute();
                value--;
                set_data_absolute(value);
                instruction_pointer++;
                instruction_pointer++;
                DEC_set_CPU_flags(value);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_DEC_ABSOLUTE_X:
            {
                uint8_t value = get_data_absolute(X);
                value--;
                set_data_absolute(value, X);
                instruction_pointer++;
                instruction_pointer++;
                DEC_set_CPU_flags(value);
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_JMP_INDIRECT:
            {
                uint16_t lookup_address = get_word(instruction_pointer);
                instruction_pointer = get_word(lookup_address);

                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
            }
            break;

            case INSTR_6502_AND_IMMEDIATE:
                A &= get_data_immediate();
                instruction_pointer++;
                AND_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_AND_ZEROPAGE_X:
                A &= get_data_zeropage(X);
                instruction_pointer++;
                AND_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_AND_ZEROPAGE:
                A &= get_data_zeropage();
                instruction_pointer++;
                AND_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_AND_ABSOLUTE:
                A &= get_data_absolute();
                instruction_pointer++;
                instruction_pointer++;
                AND_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_AND_ABSOLUTE_X:
                A &= get_data_absolute(X);
                instruction_pointer++;
                instruction_pointer++;
                AND_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_AND_ABSOLUTE_Y:
                A &= get_data_absolute(Y);
                instruction_pointer++;
                instruction_pointer++;
                AND_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            case INSTR_6502_AND_INDIRECT_X:
                A &= get_data_indexed_indirect(X);
                instruction_pointer++;
                AND_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                break;

            case INSTR_6502_AND_INDIRECT_Y:
                A &= get_data_indirect_indexed(Y);
                instruction_pointer++;
                AND_set_CPU_flags();
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                cycles_available--;
                if (page_crossed)
                {
                    cycles_available--;
                }
                break;

            default:
                std::cout << "Unknown instruction: 0x" << std::hex << std::setw(2) << std::setfill('0');
                std::cout << (int)instruction << "\n";
                return ReturnCode::BREAK;
            }
        }
        return ReturnCode::CONTINUE;
    }
}
