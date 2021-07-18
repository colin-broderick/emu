#include <iostream>
#include <iomanip>

#include "cpu.hpp"
#include "memory.hpp"
#include "semaphore.hpp"

#define DEBUG 1

/** \brief CPU constructor; sets initial configuration including IP, SP, flags, etc. */
CPU::CPU()
{
    IP = 0x0000;
    SP = 0x01FF;
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
    Word difference = (Word)A - (Word)data_from_memory;
    if (A >= data_from_memory)
    {
        C = true;
    }
    if (A == data_from_memory)
    {
        Z = true;
    }
    if (difference & 0x80)
    {
        N = true;
    }
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
void CPU::run(Memory& memory)
{
    while (true)
    {
        #if DEBUG
        std::cout << *this << "\n";
        std::cout << memory << "\n";
        #endif

        // TODO Interrupt handler should go here.

        // Grab an instruction from RAM.
        Byte instruction = get_byte(memory);

        // What we just did costs a single CPU cycle, so consume one cycle.
        // We don't continue from here until another CPU cycle becomes available.
        sem.wait();

        // We increment the instruction pointer to point to the next byte in memory.
        IP++;

        switch (instruction)
        {
            case INSTR_6502_LDA_IMMEDIATE:
                A = get_data_immediate(memory);
                IP++;
                LDA_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_LDA_ZEROPAGE:
                A = get_data_zero_page(memory);
                IP++;
                LDA_set_CPU_flags();
                sem.wait();
                sem.wait();
                break;

            case INSTR_6502_LDA_ZEROPAGE_X:
                A = get_data_zero_page(memory, X);
                IP++;
                LDA_set_CPU_flags();
                sem.wait();
                sem.wait();
                sem.wait();
                break;

            case INSTR_6502_LDA_ABSOLUTE:
                A = get_data_absolute(memory);
                IP++;
                IP++;
                LDA_set_CPU_flags();
                sem.wait();
                sem.wait();
                sem.wait();
                break;

            case INSTR_6502_LDA_ABSOLUTE_X:
                A = get_data_absolute(memory, X);
                IP++;
                IP++;
                LDA_set_CPU_flags();
                sem.wait();
                sem.wait();
                sem.wait();
                if (page_crossed)
                {
                    page_crossed = false;
                    sem.wait();
                }
                break;

            case INSTR_6502_LDA_ABSOLUTE_Y:
                A = get_data_absolute(memory, Y);
                IP++;
                IP++;
                LDA_set_CPU_flags();
                sem.wait();
                sem.wait();
                sem.wait();
                if (page_crossed)
                {
                    page_crossed = false;
                    sem.wait();
                }
                break;
            
            case INSTR_6502_LDA_INDIRECT_X:
                A = get_data_indexed_indirect(memory, X);
                IP++;
                LDA_set_CPU_flags();
                sem.wait();
                sem.wait();
                sem.wait();
                sem.wait();
                sem.wait();
                break;

            case INSTR_6502_LDA_INDIRECT_Y:
                A = get_data_indirect_indexed(memory, Y);
                IP++;
                LDA_set_CPU_flags();
                sem.wait();
                sem.wait();
                sem.wait();
                sem.wait();
                if (page_crossed)
                {
                    page_crossed = false;
                    sem.wait();
                }
                break;

            case INSTR_6502_LDY_IMMEDIATE:
                Y = get_byte(memory);
                IP++;
                LDY_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_CMP_IMMEDIATE:
                {
                    Byte data = get_data_immediate(memory);
                    IP++;
                    CMP_set_CPU_flags(data);
                    sem.wait();
                }
                break;

            case INSTR_6502_CMP_ZERO_PAGE:
                {
                    Byte data = get_data_zero_page(memory);
                    IP++;
                    CMP_set_CPU_flags(data);
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_CMP_ZERO_PAGE_X:
                {
                    Byte data = get_data_zero_page(memory, X);
                    IP++;
                    CMP_set_CPU_flags(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_CMP_ABSOLUTE:
                {
                    Byte data = get_data_absolute(memory);
                    IP++;
                    CMP_set_CPU_flags(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_CMP_ABSOLUTE_X:
                {
                    Byte data = get_data_absolute(memory, X);
                    IP++;
                    CMP_set_CPU_flags(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (page_crossed)
                    {
                        page_crossed = false;
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_CMP_ABSOLUTE_Y:
                {
                    Byte data = get_data_absolute(memory, Y);
                    IP++;
                    CMP_set_CPU_flags(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (page_crossed)
                    {
                        page_crossed = false;
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_CMP_INDIRECT_X:
                {
                    Byte data = get_data_indexed_indirect(memory, X);
                    IP++;
                    CMP_set_CPU_flags(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_CMP_INDIRECT_Y:
                {
                    Byte data = get_data_indirect_indexed(memory, Y);
                    IP++;
                    CMP_set_CPU_flags(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (page_crossed)
                    {
                        page_crossed = false;
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_STA_ABSOLUTE:
                {
                    // Load address
                    Word data_address = get_word(memory);
                    sem.wait();
                    IP++;
                    sem.wait();
                    IP++;

                    // Set value of memory address to A.
                    set_byte(memory, data_address, A);
                    sem.wait();
                }
                break;

            case INSTR_6502_STA_ABSOLUTE_Y:
                {
                    // get address in next two bytes
                    Word target_address = get_word(memory);
                    sem.wait();
                    IP++;
                    sem.wait();
                    IP++;

                    // Add Y to address
                    target_address += Y;
                    sem.wait();

                    // Store A in address
                    memory[target_address] = A;
                    sem.wait();
                }
                break;

            case INSTR_6502_TXS:
                SP = X;
                sem.wait();
                break;

            case INSTR_6502_TSX:
                X = SP;
                Z = (X == 0);
                N = (X & 0x80);
                sem.wait();
                break;

            case INSTR_6502_TYA:
                A = Y;
                Z = (A == 0);
                N = (A & 0x80);
                sem.wait();
                break;

            case INSTR_6502_STA_ZEROPAGE:
                {
                    // get address
                    Byte data_address = get_byte(memory);
                    sem.wait();
                    IP++;
                    
                    // get data from address
                    set_byte(memory, data_address, A);
                    sem.wait();
                }
                break;

            case INSTR_6502_STX_ABSOLUTE:
                {
                    // Load address
                    Word data_address = get_word(memory);
                    sem.wait();
                    IP++;
                    sem.wait();
                    IP++;

                    // Set value of memory address to A.
                    set_byte(memory, data_address, X);
                    sem.wait();
                }
                break;

            case INSTR_6502_STY_ABSOLUTE:
                {
                    // Load address
                    Word data_address = get_word(memory);
                    sem.wait();
                    IP++;
                    sem.wait();
                    IP++;

                    // Set value of memory address to A.
                    set_byte(memory, data_address, Y);
                    sem.wait();
                }
                break;

            case INSTR_6502_TAX:
                // Copy A into X.
                X = A;
                TAX_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_TAY:
                Y = A;
                Z = (Y == 0);
                N = (Y & 0x80);
                sem.wait();

            case INSTR_6502_TXA:
                A = X;
                TXA_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_INX:
                // Increment X.
                X++;
                LDX_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_INY:
                Y++;
                LDY_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_LDX_IMMEDIATE:
                // Load data into X.
                X = get_byte(memory);
                LDX_set_CPU_flags();
                sem.wait();
                IP++;
                break;

            case INSTR_6502_DEX:
                X--;
                LDX_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_DEY:
                Y--;
                LDY_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_CPX_IMMEDIATE:
                {
                    Word result = (Word)X - (Word)get_byte(memory);
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
                    sem.wait();
                    IP++;
                }
                break;

            case INSTR_6502_CPY_IMMEDIATE:
                {
                    Word result = (Word)Y - (Word)get_byte(memory);
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
                    sem.wait();
                    IP++;
                }
                break;

            case INSTR_6502_BNE_RELATIVE:
                {
                    // Remember the starting page so we know if we've moved to a new page.
                    Byte current_page = IP >> 8;
                    sem.wait();

                    if (!Z)
                    {
                        Byte dist = get_byte(memory);
                        branch_relative(dist);
                        sem.wait();
                    }
                    IP++;

                    // This should take two additional clock cycles if the branch leads to a new page.
                    Byte new_page = IP >> 8;
                    if (current_page != new_page)
                    {
                        sem.wait();
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_BEQ_RELATIVE:
                {
                    // Remember the starting page so we know if we've moved to a new page.
                    Byte current_page = IP >> 8;
                    sem.wait();

                    if (Z)
                    {
                        Byte dist = get_byte(memory);
                        branch_relative(dist);
                        sem.wait();
                    }
                    IP++;

                    // This should take two additional clock cycles if the branch leads to a new page.
                    Byte new_page = IP >> 8;
                    if (current_page != new_page)
                    {
                        sem.wait();
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_BMI_RELATIVE:
                {
                    Byte current_page = IP >> 8;
                    sem.wait();

                    if (N)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        sem.wait();
                    }
                    IP++;

                    Byte new_page = IP >> 8;
                    if (current_page != new_page)
                    {
                        sem.wait();
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_BPL_RELATIVE:
                {
                    Byte current_page = IP >> 8;
                    sem.wait();

                    if (!N)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        sem.wait();
                    }
                    IP++;

                    Byte new_page = IP >> 8;
                    if (current_page != new_page)
                    {
                        sem.wait();
                        sem.wait();
                    }
                }
                break;
            
            case INSTR_6502_BVC_RELATIVE:
                {
                    Byte current_page = IP >> 8;
                    sem.wait();

                    if (!V)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        sem.wait();
                    }
                    IP++;

                    Byte new_page = IP >> 8;
                    if (current_page != new_page)
                    {
                        sem.wait();
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_BVS_RELATIVE:
                {
                    Byte current_page = IP >> 8;
                    sem.wait();

                    if (V)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        sem.wait();
                    }
                    IP++;

                    Byte new_page = IP >> 8;
                    if (current_page != new_page)
                    {
                        sem.wait();
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_BCC_RELATIVE:
                {
                    Byte current_page = IP >> 8;
                    sem.wait();

                    if (!C)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        sem.wait();
                    }
                    IP++;

                    Byte new_page = IP >> 8;
                    if (current_page != new_page)
                    {
                        sem.wait();
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_BCS_RELATIVE:
                {
                    Byte current_page = IP >> 8;
                    sem.wait();

                    if (C)
                    {
                        Byte dist = get_data_relative(memory);
                        branch_relative(dist);
                        sem.wait();
                    }
                    IP++;

                    Byte new_page = IP >> 8;
                    if (current_page != new_page)
                    {
                        sem.wait();
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_SED:
                D = true;
                sem.wait();
                break;

            case INSTR_6502_ORA_INDIRECT_X:
                {
                    // Read next byte to get indirect address and jump over it
                    Byte indirect_address = get_byte(memory) + X;
                    sem.wait();
                    IP++;

                    // Get the data from other address
                    Word target_address = get_word(memory, indirect_address);
                    sem.wait();

                    // Get the data to be ORed with A.
                    Byte data = get_byte(memory, target_address);
                    sem.wait();

                    // Do the OR, putting result in A.
                    A |= data;
                    sem.wait();

                    ORA_set_CPU_flags();
                    sem.wait();
                }
                break;

            case INSTR_6502_ADC_IMMEDIATE:
                {
                    // TODO: Explain this. What I've done is mostly based on
                    // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
                    Byte data = get_byte(memory);
                    IP++;
                    A = add_with_carry(data);
                    sem.wait();
                }
                break;

            case INSTR_6502_ADC_ZERO_PAGE:
                {
                    Byte data = get_data_zero_page(memory);
                    IP++;
                    A = add_with_carry(data);
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_ADC_ZERO_PAGE_X:
                {
                    Byte data = get_data_zero_page(memory, X);
                    IP++;
                    A = add_with_carry(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_ADC_ABSOLUTE:
                {
                    Byte data = get_data_absolute(memory);
                    IP++;
                    IP++;
                    A = add_with_carry(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_ADC_ABSOLUTE_X:
                {
                    Byte data = get_data_absolute(memory, X);
                    IP++;
                    IP++;
                    A = add_with_carry(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (page_crossed)
                    {
                        page_crossed = false;
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_ADC_ABSOLUTE_Y:
                {
                    Byte data = get_data_absolute(memory, Y);
                    IP++;
                    IP++;
                    A = add_with_carry(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (page_crossed)
                    {
                        page_crossed = false;
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_ADC_INDIRECT_X:
                {
                    Byte data = get_data_indexed_indirect(memory, X);
                    IP++;
                    A = add_with_carry(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_ADC_INDIRECT_Y:
                {
                    Byte data = get_data_indirect_indexed(memory, Y);
                    IP++;
                    A = add_with_carry(data);
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (page_crossed)
                    {
                        page_crossed = false;
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_CLD:
                D = false;
                sem.wait();
                break;

            case INSTR_6502_CLI:
                I = false;
                sem.wait();
                break;

            case INSTR_6502_CLC:
                C = false;
                sem.wait();
                break;

            case INSTR_6502_CLV:
                V = false;
                sem.wait();
                break;

            case INSTR_6502_PHA:
                memory[SP] = A;
                sem.wait();
                SP--;
                sem.wait();
                break;

            case INSTR_6502_PHP:
                memory[SP] = flags_as_byte();
                sem.wait();
                SP--;
                sem.wait();
                break;

            case INSTR_6502_PLA:
                A = memory[SP];
                sem.wait();
                SP++;
                sem.wait();
                sem.wait();
                LDA_set_CPU_flags();
                break;

            case INSTR_6502_NOP:
                sem.wait();
                break;

            case INSTR_6502_BRK:
                // TODO: Why does this take seven cycles?
                sem.wait();
                sem.wait();
                sem.wait();
                sem.wait();
                sem.wait();
                sem.wait();
                B = true;
                set_byte(memory, SP, IP);
                SP--;
                set_byte(memory, SP, flags_as_byte());
                std::cout << "BRK reached" << std::endl;
                return;

            case INSTR_6502_JSR_ABSOLUTE:
                {
                    // Pushes (address minus one) of the return point onto the stack then sets program counter to target address
                    Word target_address = get_word(memory);
                    IP++;
                    memory[SP] = (IP >> 8);
                    SP--;
                    memory[SP] = (IP & 0xFF);
                    SP--;
                    IP = target_address;
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_RTS:
                {
                    SP++;
                    Word pointer = get_word(memory, SP);
                    SP++;

                    IP = pointer + 1;

                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;
            
            case INSTR_6502_JMP_ABSOLUTE:
                {
                    IP = get_word(memory);
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_INC_ABSOLUTE:
                {
                    Word target_address = get_word(memory);
                    memory[target_address]++;
                    IP++;
                    IP++;
                    N = (memory[target_address] & 0x80);  // Set N on if sign bit of result is set.
                    Z = (memory[target_address] == 0);    // Set Z on if result is zero.
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
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
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
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
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
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
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_JMP_INDIRECT:
                {
                    Word lookup_address = get_word(memory);
                    IP = get_word(memory, lookup_address);

                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;
            
            case INSTR_6502_AND_IMMEDIATE:
                {
                    Byte operand = get_byte(memory);
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    IP++;
                }
                break;

            case INSTR_6502_AND_ZEROPAGE_X:
                {
                    //get address, add X, wrap if necessary
                    Byte lookup_address = (get_byte(memory) + X) & 0xff;
                    Byte operand = get_byte(memory, lookup_address);
                    IP++;
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_AND_ZEROPAGE:
                {
                    Byte lookup_address = get_byte(memory);
                    Byte operand = get_byte(memory, lookup_address);
                    IP++;
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_AND_ABSOLUTE:
                {
                    Word lookup_address = get_word(memory);
                    Byte operand = get_byte(memory, lookup_address);
                    IP++;
                    IP++;
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;
            
            case INSTR_6502_AND_ABSOLUTE_X:
                {
                    Word lookup_address = get_word(memory);
                    Word old_page = lookup_address & 0xFF00;
                    lookup_address += X;
                    Word new_page = lookup_address & 0xFF00;
                    Byte operand = get_byte(memory, lookup_address);
                    IP++;
                    IP++;
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (old_page != new_page) 
                    {
                        sem.wait();
                    }
                }
                break;
            
            case INSTR_6502_AND_ABSOLUTE_Y:
                {
                    Word lookup_address = get_word(memory);
                    Word old_page = lookup_address & 0xFF00;
                    lookup_address += Y;
                    Word new_page = lookup_address & 0xFF00;
                    Byte operand = get_byte(memory, lookup_address);
                    IP++;
                    IP++;
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (old_page != new_page) 
                    {
                        sem.wait();
                    }
                }
                break;
            
            case INSTR_6502_AND_INDIRECT_X:
                {
                    // read next byte and add X without carry
                    Byte indirect_address = get_byte(memory) + X;
                    IP++;

                    //get target address from indirect_address data and next on zero page
                    Word target_address = get_word_zpg_wrap(memory, indirect_address);

                    // get data from target address
                    Byte operand = get_byte(memory, target_address);

                    //complete operation
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;
            
            case INSTR_6502_AND_INDIRECT_Y:
                {
                    Byte indirect_address = get_byte(memory);
                    IP++;
                    Word target_address = get_word_zpg_wrap(memory, indirect_address);
                    Byte page1 = target_address >> 8;
                    target_address += Y;
                    Byte page2 = target_address >> 8;

                    Byte operand = get_byte(memory, target_address);

                    //complete operation
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (page1 != page2)
                    {
                        sem.wait();
                    }
                }
                break;

            default:
                std::cout << "Unknown instruction: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)instruction << "\n";
                return;
        }
    }
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
    Word val1 = (Word)memory[IP];
    Word val2 = (Word)memory[IP+1];
    return (val2 << 8) | val1;
}

/** \brief Gets a full word from the zero page in memory.
 * \param memory Reference to system memory.
 * \param address 8-bit address in memory, from which to fetch data.
 * \return 16-bit value from specified memory location.
 */
Word CPU::get_word(Memory& memory, const Byte address)
{
    Word val1 = (Word)memory[address];
    Word val2 = (Word)memory[(Word)address+1];
    return (val2 << 8) | val1;
}

/** \brief Gets a full word from anywhere in memory.
 * \param memory Reference to system memory.
 * \param address 16-bit address from which to fetch data.
 * \return 16-bit value from memory.
 */
Word CPU::get_word(Memory& memory, const Word address)
{
    Word val1 = (Word)memory[address];
    Word val2 = (Word)memory[address+1];
    return (val2 << 8) | val1;
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
    Byte page1 = address >> 8;
    address += index;
    Byte page2 = address >> 8;
    
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
    Word result = (Word)data + (Word)A + (Word)C;
    Z = (result == 0);
    C = (result > 255);
    N = (result & 0x80);
    V = ((A ^ result) & (data ^ result) & 0x80) != 0;
    return (Byte)(result & 0xFF);
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
Byte CPU::get_data_zero_page(Memory& memory)
{
    Byte data_address = get_byte(memory);
    return get_byte(memory, data_address);
}

/** \brief Fetches a byte using zpg addressing mode with an index, typically X or Y register.
 * \param memory A reference to a memory array object.
 * \param index An index into a memory region.
 * \return A Byte from memory.
 */
Byte CPU::get_data_zero_page(Memory& memory, const Byte index)
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
    Word val1 = (Word)memory[address % 256];
    Word val2 = (Word)memory[(address+1) % 256];   // This wraps automatically since address is a Byte
    return (val2 << 8) | val1;
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

/** \brief Get data from memory using the (indirect),y addressing mode.
 * \param memory Reference to system memory.
 * \param index Index to add to address.
 * \return 8-bit value from memory.
 */
Byte CPU::get_data_indirect_indexed(Memory& memory, const Byte index)
{
    //read next byte and add index without carry
    Byte indirect_address = get_byte(memory);

    //get target address from indirect_address data and next on zero page and add index
    Word target_address = get_word_zpg_wrap(memory, indirect_address) + index;

    // TODO I'm pretty sure this is completely wrong! Consider it a placeholder.
    // Check if page crossed.
    Byte current_page = IP >> 8;
    Byte data_page = target_address >> 8;
    if (current_page != data_page)
    {
        page_crossed = true;
    }

    //get data from target address and return
    return get_byte(memory, target_address);
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
    return (N << 7) | (V << 6) | (true << 5) | (B << 4) | (D << 3) | (I << 2) | (Z << 1) | (C << 1);
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
