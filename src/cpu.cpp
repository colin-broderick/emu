#include <iostream>
#include <iomanip>

#include "cpu.hpp"
#include "memory.hpp"

#define DEBUG 1

#if DEBUG
#define LOG(x) std::cout << x << std::endl
#else

#endif

/** \brief CPU constructor; sets initial configuration including IP, SP, flags, etc. */
CPU::CPU()
{
    IP = 0x0000;
    SP = 0x01FF;
    C = Z = I = D = B = V = N = false;
}

/** \brief CPU constructor which allows custom setting of IP and SP.
 * \param ip The starting instruction pointer.
 * \param sp The starting stack pointer.
 */
CPU::CPU(const unsigned int ip, const unsigned int sp)
{
    IP = static_cast<Word>(ip);
    SP = static_cast<Word>(sp);
    C = Z = I = D = B = V = N = false;
}

/** \brief Sets appropriate flags after performing LDA operations. */
void CPU::LDA_set_CPU_flags()
{
    N = (A & 0b10000000);
    Z = (A == 0);
}

/** \brief Sets appropriate flags after performing a CMP operation.
 * \param data_from_memory The flags to set depend on the data read from memory to do the comparison.
 */
void CPU::CMP_set_CPU_flags(Byte data_from_memory)
{
    Word difference = static_cast<Word>(A - data_from_memory);
    C = (A >= data_from_memory);
    Z = (A == data_from_memory);
    N = (difference & 0x80);
}

/** \brief Sets appropriate CPU flags following an EOR operation. */
void CPU::EOR_set_CPU_flags()
{
    LDA_set_CPU_flags();
}

/** \brief Sets appropriuate flags after performing ORA operation. */
void CPU::ORA_set_CPU_flags()
{
    LDA_set_CPU_flags();
}

/** \brief Sets appropriate flags after performing LDX operations.. */
void CPU::LDX_set_CPU_flags()
{
    N = (X & 0b10000000);
    Z = (X == 0);
}

/** \brief Sets appropriate flags after performing LDY operations. */
void CPU::LDY_set_CPU_flags()
{
    N = (Y & 0b10000000);
    Z = (Y == 0);
}

/** \brief Sets appropriate flags after performing TAX operation. */
void CPU::TAX_set_CPU_flags()
{
    LDX_set_CPU_flags();
}

/** \brief Sets appropriate flags after performing TXA operation. */
void CPU::TXA_set_CPU_flags()
{
    LDA_set_CPU_flags();
}

/** \brief Runs the loaded program while CPU cycles are available to spend.
 * \param memory A reference to the main memory of the system. The CPU reads and writes this memory.
 */
int CPU::run(Memory& memory, const int cycles)
{
    add_cycles(cycles);
    
    while (cycles_available > 0)
    {
        // #if DEBUG
        // std::cout << *this << "\n";
        // std::cout << memory << "\n";
        // #endif

        // Reset the page crossing flag in case it was left on from the last iteration.
        page_crossed = false;

        // TODO Interrupt handler should go here.

        // Grab an instruction from RAM.
        Byte instruction = get_byte(memory);

        // We increment the instruction pointer to point to the next byte in memory.
        IP++;

        LOG(instruction_names[instruction] << " " << IP);

        switch (instruction)
        {
            case INSTR_6502_LDA_IMMEDIATE:
                A = get_data_immediate(memory);
                IP++;
                LDA_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_LDA_ZEROPAGE:
                A = get_data_zeropage(memory);
                IP++;
                LDA_set_CPU_flags();
                use_cycles(3);
                break;

            case INSTR_6502_LDA_ZEROPAGE_X:
                A = get_data_zeropage(memory, X);
                IP++;
                LDA_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_LDA_ABSOLUTE:
                A = get_data_absolute(memory);
                IP++;
                IP++;
                LDA_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_LDA_ABSOLUTE_X:
                A = get_data_absolute(memory, X);
                IP++;
                IP++;
                LDA_set_CPU_flags();
                use_cycles(4);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            case INSTR_6502_LDA_ABSOLUTE_Y:
                A = get_data_absolute(memory, Y);
                IP++;
                IP++;
                LDA_set_CPU_flags();
                use_cycles(4);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;
            
            case INSTR_6502_LDA_INDIRECT_X:
                A = get_data_indexed_indirect(memory, X);
                IP++;
                LDA_set_CPU_flags();
                use_cycles(6);
                break;

            case INSTR_6502_LDA_INDIRECT_Y:
                A = get_data_indirect_indexed(memory, Y);
                IP++;
                LDA_set_CPU_flags();
                use_cycles(5);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            case INSTR_6502_LDY_IMMEDIATE:
                Y = get_data_immediate(memory);
                IP++;
                LDY_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_LDY_ZEROPAGE:
                Y = get_data_zeropage(memory);
                IP++;
                LDY_set_CPU_flags();
                use_cycles(3);
                break;

            case INSTR_6502_LDY_ZEROPAGE_X:
                Y = get_data_zeropage(memory, X);
                IP++;
                LDY_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_LDY_ABSOLUTE:
                Y = get_data_absolute(memory);
                IP++;
                IP++;
                LDY_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_LDY_ABSOLUTE_X:
                Y = get_data_absolute(memory, X);
                IP++;
                IP++;
                LDY_set_CPU_flags();
                use_cycles(4);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            case INSTR_6502_CMP_IMMEDIATE:
                {
                    Byte data = get_data_immediate(memory);
                    IP++;
                    CMP_set_CPU_flags(data);
                    use_cycles(2);
                }
                break;

            case INSTR_6502_CMP_ZEROPAGE:
                {
                    Byte data = get_data_zeropage(memory);
                    IP++;
                    CMP_set_CPU_flags(data);
                    use_cycles(3);
                }
                break;

            case INSTR_6502_CMP_ZEROPAGE_X:
                {
                    Byte data = get_data_zeropage(memory, X);
                    IP++;
                    CMP_set_CPU_flags(data);
                    use_cycles(4);
                }
                break;

            case INSTR_6502_CMP_ABSOLUTE:
                {
                    Byte data = get_data_absolute(memory);
                    IP++;
                    IP++;
                    CMP_set_CPU_flags(data);
                    use_cycles(4);
                }
                break;

            case INSTR_6502_CMP_ABSOLUTE_X:
                {
                    Byte data = get_data_absolute(memory, X);
                    IP++;
                    IP++;
                    CMP_set_CPU_flags(data);
                    use_cycles(4);
                    if (page_crossed)
                    {
                        use_cycles(1);
                    }
                }
                break;

            case INSTR_6502_CMP_ABSOLUTE_Y:
                {
                    Byte data = get_data_absolute(memory, Y);
                    IP++;
                    IP++;
                    CMP_set_CPU_flags(data);
                    use_cycles(4);
                    if (page_crossed)
                    {
                        use_cycles(1);
                    }
                }
                break;

            case INSTR_6502_CMP_INDIRECT_X:
                {
                    Byte data = get_data_indexed_indirect(memory, X);
                    IP++;
                    CMP_set_CPU_flags(data);
                    use_cycles(6);
                }
                break;

            case INSTR_6502_CMP_INDIRECT_Y:
                {
                    Byte data = get_data_indirect_indexed(memory, Y);
                    IP++;
                    CMP_set_CPU_flags(data);
                    use_cycles(5);
                    if (page_crossed)
                    {
                        use_cycles(1);
                    }
                }
                break;

            case INSTR_6502_EOR_IMMEDIATE:
                A = A ^ get_data_immediate(memory);
                IP++;
                EOR_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_EOR_ZEROPAGE:
                A = A ^ get_data_zeropage(memory);
                IP++;
                EOR_set_CPU_flags();
                use_cycles(3);
                break;

            case INSTR_6502_EOR_ZEROPAGE_X:
                A = A ^ get_data_zeropage(memory, X);
                IP++;
                EOR_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_EOR_ABSOLUTE:
                A = A ^ get_data_absolute(memory);
                IP++;
                IP++;
                EOR_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_EOR_ABSOLUTE_X:
                A = A ^ get_data_absolute(memory, X);
                IP++;
                IP++;
                EOR_set_CPU_flags();
                use_cycles(4);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            case INSTR_6502_EOR_ABSOLUTE_Y:
                A = A ^ get_data_absolute(memory, Y);
                IP++;
                IP++;
                EOR_set_CPU_flags();
                use_cycles(4);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            case INSTR_6502_EOR_INDIRECT_X:
                {
                    A = A ^ get_data_indexed_indirect(memory, X);
                    IP++;
                    EOR_set_CPU_flags();
                    use_cycles(6);
                }
                break;

            case INSTR_6502_EOR_INDIRECT_Y:
                A = A ^ get_data_indirect_indexed(memory, Y);
                IP++;
                EOR_set_CPU_flags();
                use_cycles(5);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            case INSTR_6502_STA_ZEROPAGE:
                set_data_zeropage(memory, A);
                IP++;
                use_cycles(3);
                break;

            case INSTR_6502_STA_ZEROPAGE_X:
                set_data_zeropage(memory, A, X);
                IP++;
                use_cycles(4);
                break;

            case INSTR_6502_STA_ABSOLUTE:
                set_data_absolute(memory, A);
                IP++;
                IP++;
                use_cycles(4);
                break;

            case INSTR_6502_STA_ABSOLUTE_X:
                set_data_absolute(memory, A, X);
                IP++;
                IP++;
                use_cycles(5);
                break;

            case INSTR_6502_STA_ABSOLUTE_Y:
                set_data_absolute(memory, A, Y);
                IP++;
                IP++;
                use_cycles(5);
                break;

            case INSTR_6502_STA_INDIRECT_X:
                set_data_indexed_indirect(memory, A, X);
                IP++;
                IP++;
                use_cycles(6);
                break;

            case INSTR_6502_STA_INDIRECT_Y:
                set_data_indirect_indexed(memory, A, Y);
                IP++;
                IP++;
                use_cycles(6);
                break;

            case INSTR_6502_TXS:
                SP = X;
                use_cycles(2);
                break;

            case INSTR_6502_TSX:
                // TODO This doesn't seem right. Is says "stack register"; does that mean contents of current stack pointer address, or stack pointer itself?
                X = static_cast<Byte>(SP & 0x00FF);
                Z = (X == 0);
                N = (X & 0x80);
                use_cycles(2);
                break;

            case INSTR_6502_TYA:
                A = Y;
                Z = (A == 0);
                N = (A & 0x80);
                use_cycles(2);
                break;

            case INSTR_6502_STX_ZEROPAGE:
                set_data_zeropage(memory, X);
                IP++;
                use_cycles(3);
                break;

            case INSTR_6502_STX_ZEROPAGE_Y:
                set_data_absolute(memory, X, Y);
                IP++;
                use_cycles(4);
                break;

            case INSTR_6502_STX_ABSOLUTE:
                set_data_absolute(memory, X);
                IP++;
                IP++;
                use_cycles(4);
                break;

            case INSTR_6502_STY_ABSOLUTE:
                set_data_absolute(memory, Y);
                IP++;
                IP++;
                use_cycles(4);
                break;

            case INSTR_6502_TAX:
                // Copy A into X.
                X = A;
                TAX_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_TAY:
                Y = A;
                Z = (Y == 0);
                N = (Y & 0x80);
                use_cycles(4);

            case INSTR_6502_TXA:
                A = X;
                TXA_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_INX:
                // Increment X.
                X++;
                LDX_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_INY:
                Y++;
                LDY_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_LDX_IMMEDIATE:
                // Load data into X.
                X = get_byte(memory);
                LDX_set_CPU_flags();
                use_cycles(2);
                IP++;
                break;

            case INSTR_6502_DEX:
                X--;
                LDX_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_DEY:
                Y--;
                LDY_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_CPX_IMMEDIATE:
                {
                    Word result = static_cast<Word>(X - get_byte(memory));
                    if (result >= 0)
                    {
                        C = true;
                        if (result == 0)
                        {
                            Z = true;
                        }
                        if (result & 0b10000000)
                        {
                            N = true;
                        }
                    }
                    use_cycles(2);
                    IP++;
                }
                break;

            case INSTR_6502_CPY_IMMEDIATE:
                {
                    Word result = static_cast<Word>(Y - get_byte(memory));
                    if (result >= 0)
                    {
                        C = true;
                        if (result == 0)
                        {
                            Z = true;
                        }
                        if (result & 0b10000000)
                        {
                            N = true;
                        }
                    }
                    use_cycles(2);
                    IP++;
                }
                break;

            case INSTR_6502_BNE_RELATIVE:
                {
                    // Remember the starting page so we know if we've moved to a new page.
                    Byte current_page = static_cast<Byte>(IP >> 8);
                    use_cycles(2);

                    if (!Z)
                    {
                        Byte dist = get_byte(memory);
                        branch_relative(dist);
                        use_cycles(1);
                    }
                    IP++;

                    // This should take two additional clock cycles if the branch leads to a new page.
                    Byte new_page = static_cast<Byte>(IP >> 8);
                    if (current_page != new_page)
                    {
                        use_cycles(2);
                    }
                }
                break;

            case INSTR_6502_BEQ_RELATIVE:
                {
                    // Remember the starting page so we know if we've moved to a new page.
                    Byte current_page = static_cast<Byte>(IP >> 8);
                    use_cycles(2);

                    if (Z)
                    {
                        Byte dist = get_byte(memory);
                        branch_relative(dist);
                        use_cycles(1);
                    }
                    IP++;

                    // This should take two additional clock cycles if the branch leads to a new page.
                    Byte new_page = static_cast<Byte>(IP >> 8);
                    if (current_page != new_page)
                    {
                        use_cycles(2);
                    }
                }
                break;

            case INSTR_6502_BMI_RELATIVE:
                {
                    Byte current_page = static_cast<Byte>(IP >> 8);
                    use_cycles(2);

                    if (N)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        use_cycles(1);
                    }
                    IP++;

                    Byte new_page = static_cast<Byte>(IP >> 8);
                    if (current_page != new_page)
                    {
                        use_cycles(2);
                    }
                }
                break;

            case INSTR_6502_BPL_RELATIVE:
                {
                    Byte current_page = static_cast<Byte>(IP >> 8);
                    use_cycles(2);

                    if (!N)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        use_cycles(1);
                    }
                    IP++;

                    Byte new_page = static_cast<Byte>(IP >> 8);
                    if (current_page != new_page)
                    {
                        use_cycles(2);
                    }
                }
                break;
            
            case INSTR_6502_BVC_RELATIVE:
                {
                    Byte current_page = static_cast<Byte>(IP >> 8);
                    use_cycles(2);

                    if (!V)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        use_cycles(1);
                    }
                    IP++;

                    Byte new_page = static_cast<Byte>(IP >> 8);
                    if (current_page != new_page)
                    {
                        use_cycles(2);
                    }
                }
                break;

            case INSTR_6502_BVS_RELATIVE:
                {
                    Byte current_page = static_cast<Byte>(IP >> 8);
                    use_cycles(2);

                    if (V)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        use_cycles(1);
                    }
                    IP++;

                    Byte new_page = static_cast<Byte>(IP >> 8);
                    if (current_page != new_page)
                    {
                        use_cycles(2);
                    }
                }
                break;

            case INSTR_6502_BCC_RELATIVE:
                {
                    Byte current_page = static_cast<Byte>(IP >> 8);
                    use_cycles(2);

                    if (!C)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        use_cycles(1);
                    }
                    IP++;

                    Byte new_page = static_cast<Byte>(IP >> 8);
                    if (current_page != new_page)
                    {
                        use_cycles(2);
                    }
                }
                break;

            case INSTR_6502_BCS_RELATIVE:
                {
                    Byte current_page = static_cast<Byte>(IP >> 8);
                    use_cycles(2);

                    if (C)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        use_cycles(1);
                    }
                    IP++;

                    Byte new_page = static_cast<Byte>(IP >> 8);
                    if (current_page != new_page)
                    {
                        use_cycles(2);
                    }
                }
                break;

            case INSTR_6502_SED:
                D = true;
                use_cycles(2);
                break;

            case INSTR_6502_ORA_INDIRECT_X:
                A |= get_data_indexed_indirect(memory, X);
                IP++;
                ORA_set_CPU_flags();
                use_cycles(6);
                break;

            case INSTR_6502_ADC_IMMEDIATE:
                {
                    // TODO: Explain this. What I've done is mostly based on
                    // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
                    Byte data = get_data_immediate(memory);
                    IP++;
                    A = add_with_carry(data);
                    use_cycles(2);
                }
                break;

            case INSTR_6502_ADC_ZEROPAGE:
                {
                    Byte data = get_data_zeropage(memory);
                    IP++;
                    A = add_with_carry(data);
                    use_cycles(3);
                }
                break;

            case INSTR_6502_ADC_ZEROPAGE_X:
                {
                    Byte data = get_data_zeropage(memory, X);
                    IP++;
                    A = add_with_carry(data);
                    use_cycles(4);
                }
                break;

            case INSTR_6502_ADC_ABSOLUTE:
                {
                    Byte data = get_data_absolute(memory);
                    IP++;
                    IP++;
                    A = add_with_carry(data);
                    use_cycles(4);
                }
                break;

            case INSTR_6502_ADC_ABSOLUTE_X:
                {
                    Byte data = get_data_absolute(memory, X);
                    IP++;
                    IP++;
                    A = add_with_carry(data);
                    use_cycles(4);
                    if (page_crossed)
                    {
                        use_cycles(1);
                    }
                }
                break;

            case INSTR_6502_ADC_ABSOLUTE_Y:
                {
                    Byte data = get_data_absolute(memory, Y);
                    IP++;
                    IP++;
                    A = add_with_carry(data);
                    use_cycles(4);
                    if (page_crossed)
                    {
                        use_cycles(1);
                    }
                }
                break;

            case INSTR_6502_ADC_INDIRECT_X:
                {
                    Byte data = get_data_indexed_indirect(memory, X);
                    IP++;
                    A = add_with_carry(data);
                    use_cycles(6);
                }
                break;

            case INSTR_6502_ADC_INDIRECT_Y:
                {
                    Byte data = get_data_indirect_indexed(memory, Y);
                    IP++;
                    A = add_with_carry(data);
                    use_cycles(5);
                    if (page_crossed)
                    {
                        use_cycles(1);
                    }
                }
                break;

            case INSTR_6502_CLD:
                D = false;
                use_cycles(2);
                break;

            case INSTR_6502_CLI:
                I = false;
                use_cycles(2);
                break;

            case INSTR_6502_CLC:
                C = false;
                use_cycles(2);
                break;

            case INSTR_6502_CLV:
                V = false;
                use_cycles(2);
                break;

            case INSTR_6502_PHA:
                memory[SP] = A;
                SP--;
                use_cycles(3);
                break;

            case INSTR_6502_PHP:
                memory[SP] = flags_as_byte();
                SP--;
                use_cycles(3);
                break;

            case INSTR_6502_PLA:
                A = memory[SP];
                SP++;
                use_cycles(4);
                LDA_set_CPU_flags();
                break;

            case INSTR_6502_NOP:
                use_cycles(2);
                break;

            case INSTR_6502_BRK:
                // TODO: Why does this take seven cycles?
                use_cycles(7);
                B = true;

                memory[SP] = static_cast<Byte>(IP >> 8);
                SP--;
                memory[SP] = static_cast<Byte>(IP & 0xFF);
                SP--;
                memory[SP] = flags_as_byte();
                SP--;

                std::cout << "BRK reached" << std::endl;
                return BREAK;

            case INSTR_6502_JSR_ABSOLUTE:
                {
                    // Pushes (address minus one) of the return point onto the stack then sets program counter to target address
                    Word target_address = get_word(memory);
                    IP++;
                    memory[SP] = static_cast<Byte>(IP >> 8);
                    SP--;
                    memory[SP] = static_cast<Byte>(IP & 0xFF);
                    SP--;
                    IP = target_address;
                    use_cycles(6);
                }
                break;

            case INSTR_6502_RTS:
                {
                    SP++;
                    Word pointer = get_word(memory, SP);
                    SP++;

                    IP = pointer + 1;

                    use_cycles(6);
                }
                break;
            
            case INSTR_6502_JMP_ABSOLUTE:
                // TODO I don't know if I'm supposed to jump to the address in memory at the IP,
                // or the address specified by that memory location.
                IP = get_word(memory);
                use_cycles(3);
                break;

            case INSTR_6502_INC_ABSOLUTE:
                {
                    Word target_address = get_word(memory);
                    memory[target_address]++;
                    IP++;
                    IP++;
                    N = (memory[target_address] & 0x80);  // Set N on if sign bit of result is set.
                    Z = (memory[target_address] == 0);    // Set Z on if result is zero.
                    use_cycles(6);
                }
                break;

            case INSTR_6502_INC_ABSOLUTE_X:
                {
                    Word target_address = get_word(memory);
                    target_address += X;
                    memory[target_address]++;
                    IP++;
                    IP++;
                    N = (memory[target_address] & 0x80);  // Set N on if sign bit of result is set.
                    Z = (memory[target_address] == 0);    // Set Z on if result is zero.
                    use_cycles(7);
                }
                break;

            case INSTR_6502_DEC_ABSOLUTE:
                {
                    Word target_address = get_word(memory);
                    memory[target_address]--;
                    IP++;
                    IP++;
                    N = (memory[target_address] & 0x80);  // Set N on if sign bit of result is set.
                    Z = (memory[target_address] == 0);    // Set Z on if result is zero.
                    use_cycles(6);
                }
                break;

            case INSTR_6502_DEC_ABSOLUTE_X:
                {
                    Word target_address = get_word(memory);
                    target_address += X;
                    memory[target_address]--;
                    IP++;
                    IP++;
                    N = (memory[target_address] & 0x80);  // Set N on if sign bit of result is set.
                    Z = (memory[target_address] == 0);    // Set Z on if result is zero.
                    use_cycles(7);
                }
                break;

            case INSTR_6502_JMP_INDIRECT:
                {
                    Word lookup_address = get_word(memory);
                    IP = get_word(memory, lookup_address);

                    use_cycles(5);
                }
                break;
            
            case INSTR_6502_AND_IMMEDIATE:
                A &= get_data_immediate(memory);
                IP++;
                AND_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_AND_ZEROPAGE_X:
                A &= get_data_zeropage(memory, X);
                IP++;
                AND_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_AND_ZEROPAGE:
                A &= get_data_zeropage(memory);
                IP++;
                AND_set_CPU_flags();
                use_cycles(3);
                break;

            case INSTR_6502_AND_ABSOLUTE:
                A &= get_data_absolute(memory);
                IP++;
                IP++;
                AND_set_CPU_flags();
                use_cycles(4);
                break;
            
            case INSTR_6502_AND_ABSOLUTE_X:
                A &= get_data_absolute(memory, X);
                IP++;
                IP++;
                AND_set_CPU_flags();
                use_cycles(4);
                if (page_crossed) 
                {
                    use_cycles(1);
                }
                break;
            
            case INSTR_6502_AND_ABSOLUTE_Y:
                A &= get_data_absolute(memory, Y);
                IP++;
                IP++;
                AND_set_CPU_flags();
                use_cycles(4);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;
            
            case INSTR_6502_AND_INDIRECT_X:
                A &= get_data_indexed_indirect(memory, X);
                IP++;
                AND_set_CPU_flags();
                use_cycles(6);
                break;
            
            case INSTR_6502_AND_INDIRECT_Y:
                A &= get_data_indirect_indexed(memory, Y);
                IP++;
                AND_set_CPU_flags();
                use_cycles(5);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            default:
                std::cout << "Unknown instruction: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)instruction << "\n";
                return BREAK;
        }
    }
    return cycles_available;
}

/** \brief Sets the value of a byte in memory, addressed by a full word.
 * \param memory Reference to system memory.
 * \param address The 16-bit address of the memory location to write to.
 * \param value The 8-bit value to write into memory.
 */
void CPU::set_byte(Memory& memory, Word address, Byte value)
{
    memory[address] = value;
}

/** \brief Gets a byte from memory, addressed by the current instruction pointer.
 * \param memory Reference to system memory.
 * \return A byte from memory.
 */
Byte CPU::get_byte(Memory& memory)
{
    return memory[IP];
}

/** \brief Gets a byte from the zero page in memory, from a specified 8-bit address.
 * \param memory Reference to system memory.
 * \param address The address in the zero page from which to fetch data.
 * \return 8-bit value from specified memory location.
 */
Byte CPU::get_byte(Memory& memory, const Byte address)
{
    return memory[address];
}

/** \brief Gets a byte from system memory.
 * \param memory Reference to system memory.
 * \param address A full 16-bit address from which to getch data.
 * \return 8-bit value from memory.
 */
Byte CPU::get_byte(Memory& memory, const Word address)
{
    return memory[address];
}

/** \brief Gets a full word from memory, addressed by the current instruction pointer.
 * \param memory Reference to system memory.
 * \return 16-bit value from memory.
 */
Word CPU::get_word(Memory& memory)
{
    Word val1 = static_cast<Word>(memory[IP]);
    Word val2 = static_cast<Word>(memory[IP+1]);
    return static_cast<Word>(val2 << 8) | val1;
}

/** \brief Gets a full word from the zero page in memory.
 * \param memory Reference to system memory.
 * \param address 8-bit address in memory, from which to fetch data.
 * \return 16-bit value from specified memory location.
 */
Word CPU::get_word(Memory& memory, const Byte address)
{
    Word val1 = static_cast<Word>(memory[address]);
    Word val2 = static_cast<Word>(memory[address+1]);
    return static_cast<Word>(val2 << 8) | val1;
}

/** \brief Gets a full word from anywhere in memory.
 * \param memory Reference to system memory.
 * \param address 16-bit address from which to fetch data.
 * \return 16-bit value from memory.
 */
Word CPU::get_word(Memory& memory, const Word address)
{
    Word val1 = static_cast<Word>(memory[address]);
    Word val2 = static_cast<Word>(memory[address+1]);
    return static_cast<Word>(val2 << 8) | val1;
}

/** \brief Get data byte from memory using absolute addressing, with data addressed by 
 * current instruction pointer.
 * \param memory Reference to system memory.
 * \return 8-bit value from memory.
 */
Byte CPU::get_data_absolute(Memory& memory)
{
    //get address from next two bytes and add index
    Word address = get_word(memory);
    //return data at address
    return get_byte(memory, address);
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in the 16-bit address at the current instruction pointer.
 */
void CPU::set_data_absolute(Memory& memory, Byte data)
{
    Word address = get_word(memory);
    memory[address] = data;
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in the 16-bit address at the current instruction pointer.
 */
void CPU::set_data_absolute(Memory& memory, Byte data, Byte index)
{
    Word address = get_word(memory) + index;
    memory[address] = data;
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in the 8-bit (zero page) address at the current instruction pointer.
 */
void CPU::set_data_zeropage(Memory& memory, Byte data)
{
    Byte data_address = get_byte(memory);
    memory[data_address] = data;
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in the 8-bit (zero page) address at the current instruction pointer.
 * \param index Offset from the memory location read by the instruction pointer.
 */
void CPU::set_data_zeropage(Memory& memory, Byte data, Byte index)
{
    Byte data_address = get_byte(memory) + index;
    memory[data_address] = data;
}

/** \brief Get data byte from memory using absolute addressing, with data addressed by 
 * current instruction pointer and an index.
 * \param memory Reference to system memory.
 * \param index A byte to add to the address to be read from.
 * \return 8-bit value from memory.
 */
Byte CPU::get_data_absolute(Memory& memory, const Byte index)
{
    //get address from next two bytes and add index
    Word address = get_word(memory);
    Byte page1 = static_cast<Byte>(address >> 8);
    address += index;
    Byte page2 = static_cast<Byte>(address >> 8);
    
    // Check for page crossing for extra cycle.
    if (page1 != page2)
    {
        this->page_crossed = true;
    }
    
    //return data at address
    return get_byte(memory, address);
}

/** \brief Performs addition of accumulator and data, setting the carry bit as required.
 * \param data A byte of data to be added to the accumulator.
 * \return A byte to be stored in the accumulator.
 */
Byte CPU::add_with_carry(Byte data)
{
    Word result = static_cast<Word>(data + A + C);
    Z = (result == 0);
    C = (result > 255);
    N = (result & 0x80);
    V = ((A ^ result) & (data ^ result) & 0x80) != 0;
    return static_cast<Byte>(result & 0xFF);
}

/** \brief Fetches a byte using relative addressing mode.
 * \param memory A reference to a memory array object.
 * \return A Byte from memory.
 */
Byte CPU::get_data_relative(Memory& memory)
{
    return get_data_immediate(memory);
}

/** \brief Fetches a byte using immediate addressing mode.
 * \param memory A reference to a memory array object.
 * \return A Byte from memory.
 */
Byte CPU::get_data_immediate(Memory& memory)
{
    return get_byte(memory);
}

/** \brief Fetches a byte using zpg addressing mode.
 * \param memory A reference to a memory array object.
 * \return A Byte from memory.
 */
Byte CPU::get_data_zeropage(Memory& memory)
{
    Byte data_address = get_byte(memory);
    return get_byte(memory, data_address);
}

/** \brief Fetches a byte using zpg addressing mode with an index, typically X or Y register.
 * \param memory A reference to a memory array object.
 * \param index An index into a memory region.
 * \return A Byte from memory.
 */
Byte CPU::get_data_zeropage(Memory& memory, const Byte index)
{
    Byte data_address = get_byte(memory) + index;
    return get_byte(memory, data_address);
}

/** \brief Get a word from the zero page with full wrapping.
 * \param memory Reference to system memory.
 * \param address The zero page address of the first byte to be read.
 * \return 16-bit value from the zero page.
 */
Word CPU::get_word_zpg_wrap(Memory& memory, const Byte address)
{
    Word val1 = static_cast<Word>(memory[address % 256]);
    Word val2 = static_cast<Word>(memory[(address+1) % 256]);   // This wraps automatically since address is a Byte
    return static_cast<Word>(val2 << 8) | val1;
}

/** \brief Get data from memory using the (indirect,x) addressing mode.
 * \param memory Reference to system memory.
 * \param index Index added to address.
 * \return 8-bit value from memory.
 */
Byte CPU::get_data_indexed_indirect(Memory& memory, const Byte index)
{
    // read next byte and add index without carry
    Byte indirect_address = get_byte(memory) + index;

    //get target address from indirect_address data and next on zero page
    Word target_address = get_word_zpg_wrap(memory, indirect_address);

    // get data from target address and return
    return get_byte(memory, target_address);
}

/** \brief Set a value in memory using (indirect, x) addressing.
 * \param memory Reference to system memory.
 * \param data Byte of data to store in memory.
 * \param index Offset of memory location.
 */
void CPU::set_data_indexed_indirect(Memory& memory, Byte data, Byte index)
{
    Byte indirect_address = get_byte(memory) + index;
    Word target_address = get_word_zpg_wrap(memory, indirect_address);
    memory[target_address] = data;
}

/** \brief Get data from memory using the (indirect),y addressing mode.
 * Will set the page_crossed flag if a page is crossed.
 * \param memory Reference to system memory.
 * \param index Index to add to address.
 * \return 8-bit value from memory.
 */
Byte CPU::get_data_indirect_indexed(Memory& memory, const Byte index)
{
    //read next byte and add index without carry
    Byte indirect_address = get_byte(memory);

    //get target address from indirect_address data and next on zero page and add index
    Word target_address = get_word_zpg_wrap(memory, indirect_address);
    Byte page1 = static_cast<Byte>(target_address >> 8);
    target_address += index;
    Byte page2 = static_cast<Byte>(target_address >> 8);

    // Check if page crossed.
    if (page1 != page2)
    {
        page_crossed = true;
    }

    //get data from target address and return
    return get_byte(memory, target_address);
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in memory.
 * \param index Memory location offset.
 */
void CPU::set_data_indirect_indexed(Memory& memory, const Byte data, const Byte index)
{
    Byte indirect_address = get_byte(memory);
    Word target_address = get_word_zpg_wrap(memory, indirect_address) + index;
    memory[target_address] = data;
}

/** \brief Adds the signed value distance to the IP.
 * \param distance Signed value defining the distance to jump in memory.
 */
void CPU::branch_relative(Byte distance)
{
    if (distance & 0x80)
    {
        // If jump is negative get the two's complement and subtract the result.
        distance = ~distance;
        distance += 1;
        IP = IP - distance;
    }
    else
    {
        // If the jump is positive, do the jump.
        IP = IP + distance;
    }
}

/** \brief Encode all CPU flags into a single byte.
 * \return 8-bit value containing all CPU flags.
 */
Byte CPU::flags_as_byte()
{
    return static_cast<Byte>(
          (N << 7) 
        | (V << 6) 
        | (true << 5) 
        | (B << 4) 
        | (D << 3) 
        | (I << 2) 
        | (Z << 1) 
        | (C << 1)
    );
}

/** \brief Print a summary of the CPU state to an iostream.
 * \param stream Reference to stream to write to.
 * \param cpu Const reference to the CPU to be printed.
 * \return Reference to the stream being written to.
 */
std::ostream& operator<<(std::ostream& stream, const CPU& cpu)
{
    stream << "A: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.A;
    stream << "   X: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.X;
    stream << "   Y: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.Y;
    stream << "   IP: 0x" << std::hex << std::setw(4) << std::setfill('0') << (int)cpu.IP;
    stream << "   SP: 0x" << std::hex << std::setw(4) << std::setfill('0') << (int)cpu.SP;
    stream << "\nFlags: ";
    stream << (int)cpu.N;
    stream << (int)cpu.V;
    stream << "-";
    stream << (int)cpu.B;
    stream << (int)cpu.D;
    stream << (int)cpu.I;
    stream << (int)cpu.Z;
    stream << (int)cpu.C;
    return stream;
}

/** \brief Set CPU flags following an AND operation. */
void CPU::AND_set_CPU_flags()
{
    LDA_set_CPU_flags();
}

void CPU::add_cycles(int cycles_to_add)
{
    cycles_available += cycles_to_add;
}

void CPU::use_cycles(int cycles_to_use)
{
    cycles_available -= cycles_to_use;
}
