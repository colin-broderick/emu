#include <iostream>
#include <iomanip>

#include "cpu.hpp"
#include "memory.hpp"

/** \brief CPU constructor; sets initial configuration.
 * 
 * Sets initial configuration including IP = 0x0000, SP = 0x01FF, all flags = false. */
CPU::CPU()
{
    IP = 0x0000;
    SP = 0x01FF;
    C = Z = I = D = B = V = N = false;
}

/** \brief CPU constructor which allows custom setting of IP and SP.
 * \param ip The starting instruction pointer.
 * \param sp The starting stack pointer.
 * 
 * Sets initial values of the SP and IP to the specified value, and sets all flags = false.
 */
CPU::CPU(const Word ip, const Word sp)
{
    IP = ip;
    SP = sp;
    C = Z = I = D = B = V = N = false;
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
        // std::cout << memory << "\n";
        // #endif

        // Reset the page crossing flag in case it was left on from the last iteration.
        page_crossed = false;

        // TODO Interrupt handler should go here.

        // Grab an instruction from RAM.
        Byte instruction = get_byte(memory);

        // We increment the instruction pointer to point to the next byte in memory.
        IP++;

        LOG("N" << N << " " << "V" << V << " " << "B" << B << " " << "D" << D << " " << "I" << I << " " << "Z" << Z << " " << "C" << C << "    " << std::hex << "IP:" << std::setw(4) << (int)IP << "   " << "SP:" << std::setw(4) << (int)SP << "   " << "A:" << std::setw(2) << (int)A << "   " << "X:" << std::setw(2) << (int)X << "   " << "Y:" << std::setw(2) << (int)Y << "   " << instruction_names[instruction]);

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
                SP = static_cast<Word>(0x0100 | X);
                use_cycles(2);
                break;

            case INSTR_6502_TSX:
                X = static_cast<Byte>(SP & 0x00FF);
                Z = (X == 0);
                N = (X & BIT7);
                use_cycles(2);
                break;

            case INSTR_6502_TYA:
                A = Y;
                Z = (A == 0);
                N = (A & BIT7);
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

            case INSTR_6502_STY_ZEROPAGE:
                set_data_zeropage(memory, Y);
                IP++;
                use_cycles(3);
                break;

            case INSTR_6502_STY_ZEROPAGE_X:
                set_data_zeropage(memory, Y, X);
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
                N = (Y & BIT7);
                use_cycles(4);

            case INSTR_6502_TXA:
                A = X;
                TXA_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_INX:
                // Increment X.
                X++;
                INX_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_INY:
                Y++;
                INY_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_LDX_IMMEDIATE:
                // Load data into X.
                X = get_data_immediate(memory);
                IP++;
                LDX_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_LDX_ZEROPAGE:
                // Load data into X.
                X = get_data_zeropage(memory);
                IP++;
                LDX_set_CPU_flags();
                use_cycles(3);
                break;

            case INSTR_6502_LDX_ZEROPAGE_Y:
                // Load data into X.
                X = get_data_zeropage(memory, Y);
                IP++;
                LDX_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_LDX_ABSOLUTE:
                // Load data into X.
                X = get_data_absolute(memory);
                IP++;
                IP++;
                LDX_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_LDX_ABSOLUTE_Y:
                // Load data into X.
                X = get_data_absolute(memory, Y);
                IP++;
                IP++;
                LDX_set_CPU_flags();
                use_cycles(4);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            case INSTR_6502_DEX:
                X--;
                DEX_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_DEY:
                Y--;
                DEY_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_CPX_IMMEDIATE:
                {
                    int result = X - get_data_immediate(memory);
                    IP++;
                    CPX_set_CPU_flags(result);
                    use_cycles(2);
                }
                break;
                
            case INSTR_6502_CPX_ZEROPAGE:
                {
                    int result = X - get_data_zeropage(memory);
                    IP++;
                    CPX_set_CPU_flags(result);
                    use_cycles(3);
                }
                break;

            case INSTR_6502_CPX_ABSOLUTE:
                {
                    int result = X - get_data_absolute(memory);
                    IP++;
                    CPX_set_CPU_flags(result);
                    use_cycles(4);
                }
                break;

            case INSTR_6502_CPY_IMMEDIATE:
                {
                    int result = Y - get_data_immediate(memory);
                    CPY_set_CPU_flags(result);
                    use_cycles(2);
                    IP++;
                }
                break;

            case INSTR_6502_CPY_ZEROPAGE:
                {
                    int result = Y - get_data_zeropage(memory);
                    CPY_set_CPU_flags(result);
                    use_cycles(3);
                    IP++;
                }
                break;

            case INSTR_6502_CPY_ABSOLUTE:
                {
                    int result = Y - get_data_absolute(memory);
                    CPY_set_CPU_flags(result);
                    use_cycles(4);
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
                        Byte dist = get_data_relative(memory);
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
                        Byte dist = get_data_relative(memory);
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

            case INSTR_6502_ORA_IMMEDIATE:
                A |= get_data_immediate(memory);
                IP++;
                ORA_set_CPU_flags();
                use_cycles(2);
                break;

            case INSTR_6502_ORA_ZEROPAGE:
                A |= get_data_zeropage(memory);
                IP++;
                ORA_set_CPU_flags();
                use_cycles(3);
                break;

            case INSTR_6502_ORA_ZEROPAGE_X:
                A |= get_data_zeropage(memory, X);
                IP++;
                ORA_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_ORA_ABSOLUTE:
                A |= get_data_absolute(memory);
                IP++;
                IP++;
                ORA_set_CPU_flags();
                use_cycles(4);
                break;

            case INSTR_6502_ORA_ABSOLUTE_X:
                A |= get_data_absolute(memory, X);
                IP++;
                IP++;
                ORA_set_CPU_flags();
                use_cycles(4);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            case INSTR_6502_ORA_ABSOLUTE_Y:
                A |= get_data_absolute(memory, Y);
                IP++;
                IP++;
                ORA_set_CPU_flags();
                use_cycles(4);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            case INSTR_6502_ORA_INDIRECT_X:
                A |= get_data_indexed_indirect(memory, X);
                IP++;
                ORA_set_CPU_flags();
                use_cycles(6);
                break;

            case INSTR_6502_ORA_INDIRECT_Y:
                A |= get_data_indirect_indexed(memory, Y);
                IP++;
                ORA_set_CPU_flags();
                use_cycles(5);
                if (page_crossed)
                {
                    use_cycles(1);
                }
                break;

            case INSTR_6502_BIT_ZEROPAGE:
                {
                    Byte result = A & get_data_zeropage(memory);
                    IP++;
                    Z = (result == 0);
                    V = (result & BIT6);
                    N = (result & BIT7);
                    use_cycles(3);
                }
                break;

            case INSTR_6502_BIT_ABSOLUTE:
                {
                    Byte result = A & get_data_absolute(memory);
                    IP++;
                    IP++;
                    Z = (result == 0);
                    V = (result & BIT6);
                    N = (result & BIT7);
                    use_cycles(4);
                }
                break;

            case INSTR_6502_ASL_ACCUMULATOR:
                C = (A & BIT7);
                A = A << 1;
                Z = (A == 0);
                N = (A & BIT7);
                use_cycles(2);
                break;

            case INSTR_6502_ASL_ZEROPAGE:
                {
                    Byte data = get_data_zeropage(memory);
                    C = (data & BIT7);
                    data = data << 1;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_zeropage(memory, data);
                    IP++;
                    use_cycles(5);
                }
                break;

            case INSTR_6502_ASL_ZEROPAGE_X:
                {
                    Byte data = get_data_zeropage(memory, X);
                    C = (data & BIT7);
                    data = data << 1;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_zeropage(memory, data, X);
                    IP++;
                    use_cycles(6);
                }
                break;

            case INSTR_6502_ASL_ABSOLUTE:
                {
                    Byte data = get_data_absolute(memory);
                    C = (data & BIT7);
                    data = data << 1;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_absolute(memory, data);
                    IP++;
                    IP++;
                    use_cycles(6);
                }
                break;

            case INSTR_6502_ASL_ABSOLUTE_X:
                {
                    Byte data = get_data_absolute(memory, X);
                    C = (data & BIT7);
                    data = data << 1;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_absolute(memory, data, X);
                    IP++;
                    IP++;
                    use_cycles(7);
                }
                break;

            case INSTR_6502_LSR_ACCUMULATOR:
                C = (A & BIT7);
                A = A >> 1;
                Z = (A == 0);
                N = (A & BIT7);
                use_cycles(2);
                break;

            case INSTR_6502_LSR_ZEROPAGE:
                {
                    Byte data = get_data_zeropage(memory);
                    C = (data & BIT7);
                    data = data >> 1;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_zeropage(memory, data);
                    IP++;
                    use_cycles(5);
                }
                break;

            case INSTR_6502_LSR_ZEROPAGE_X:
                {
                    Byte data = get_data_zeropage(memory, X);
                    C = (data & BIT7);
                    data = data >> 1;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_zeropage(memory, data, X);
                    IP++;
                    use_cycles(6);
                }
                break;

            case INSTR_6502_LSR_ABSOLUTE:
                {
                    Byte data = get_data_absolute(memory);
                    C = (data & BIT7);
                    data = data >> 1;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_absolute(memory, data);
                    IP++;
                    IP++;
                    use_cycles(6);
                }
                break;

            case INSTR_6502_LSR_ABSOLUTE_X:
                {
                    Byte data = get_data_absolute(memory, X);
                    C = (data & BIT7);
                    data = data >> 1;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_absolute(memory, data, X);
                    IP++;
                    IP++;
                    use_cycles(7);
                }
                break;

            case INSTR_6502_ROL_ACCUMULATOR:
                {
                    Byte tempC = static_cast<Byte>(C);
                    C = (A & BIT7);
                    A = static_cast<Byte>((A << 1) | tempC);
                    Z = (A == 0);
                    N = (A & BIT7);
                    use_cycles(2);
                }
                break;

            case INSTR_6502_ROL_ZEROPAGE:
                {
                    Byte data = get_data_zeropage(memory);
                    Byte tempC = static_cast<Byte>(C);
                    C = (data & BIT7);
                    data = static_cast<Byte>((data << 1) | tempC);
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_zeropage(memory, data);
                    IP++;
                    use_cycles(5);
                }
                break;

            case INSTR_6502_ROL_ZEROPAGE_X:
                {
                    Byte data = get_data_zeropage(memory, X);
                    Byte tempC = static_cast<Byte>(C);
                    C = (data & BIT7);
                    data = static_cast<Byte>((data << 1) | tempC);
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_zeropage(memory, data, X);
                    IP++;
                    use_cycles(6);
                }
                break;

            case INSTR_6502_ROL_ABSOLUTE:
                {
                    Byte data = get_data_absolute(memory);
                    Byte tempC = static_cast<Byte>(C);
                    C = (data & BIT7);
                    data = static_cast<Byte>((data << 1) | tempC);
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_absolute(memory, data);
                    IP++;
                    IP++;
                    use_cycles(6);
                }
                break;

            case INSTR_6502_ROL_ABSOLUTE_X:
                {
                    Byte data = get_data_absolute(memory, X);
                    Byte tempC = static_cast<Byte>(C);
                    C = (data & BIT7);
                    data = static_cast<Byte>((data << 1) | tempC);
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_absolute(memory, data, X);
                    IP++;
                    IP++;
                    use_cycles(7);
                }
                break;

            case INSTR_6502_ROR_ACCUMULATOR:
                {
                    Byte tempC = static_cast<Byte>(C) << 7;
                    C = (A & BIT0);
                    A = (A >> 1) | tempC;
                    Z = (A == 0);
                    N = (A & BIT7);
                    use_cycles(2);
                }
                break;

            case INSTR_6502_ROR_ZEROPAGE:
                {
                    Byte data = get_data_zeropage(memory);
                    Byte tempC = static_cast<Byte>(C) << 7;
                    C = (data & BIT0);
                    data = (data >> 1) | tempC;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_zeropage(memory, data);
                    IP++;
                    use_cycles(5);
                }
                break;

            case INSTR_6502_ROR_ZEROPAGE_X:
                {
                    Byte data = get_data_zeropage(memory, X);
                    Byte tempC = static_cast<Byte>(C) << 7;
                    C = (data & BIT0);
                    data = (data >> 1) | tempC;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_zeropage(memory, data, X);
                    IP++;
                    use_cycles(6);
                }
                break;

            case INSTR_6502_ROR_ABSOLUTE:
                {
                    Byte data = get_data_absolute(memory);
                    Byte tempC = static_cast<Byte>(C) << 7;
                    C = (data & BIT0);
                    data = (data >> 1) | tempC;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_absolute(memory, data);
                    IP++;
                    IP++;
                    use_cycles(6);
                }
                break;

            case INSTR_6502_ROR_ABSOLUTE_X:
                {
                    Byte data = get_data_absolute(memory, X);
                    Byte tempC = static_cast<Byte>(C) << 7;
                    C = (data & BIT0);
                    data = (data >> 1) | tempC;
                    Z = (data == 0);
                    N = (data & BIT7);
                    set_data_absolute(memory, data, X);
                    IP++;
                    IP++;
                    use_cycles(7);
                }
                break;

            case INSTR_6502_PLP:
                {
                    Byte flags = pop_from_stack(memory);
                    N = (flags >> 7) & BIT1;
                    V = (flags >> 6) & BIT1;
                    B = (flags >> 4) & BIT1;
                    D = (flags >> 3) & BIT1;
                    I = (flags >> 2) & BIT1;
                    Z = (flags >> 1) & BIT1;
                    C = (flags >> 0) & BIT1;
                    use_cycles(2);
                }
                break;

            case INSTR_6502_SEC:
                C = true;
                use_cycles(2);
                break;

            case INSTR_6502_SEI:
                I = true;
                use_cycles(2);
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

            case INSTR_6502_SBC_IMMEDIATE:
                {
                    Byte data = get_data_immediate(memory);
                    IP++;
                    A = sub_with_carry(data);
                    use_cycles(2);
                }
                break;

            case INSTR_6502_SBC_ZEROPAGE:
                {
                    Byte data = get_data_zeropage(memory);
                    IP++;
                    A = sub_with_carry(data);
                    use_cycles(3);
                }
                break;

            case INSTR_6502_SBC_ZEROPAGE_X:
                {
                    Byte data = get_data_zeropage(memory, X);
                    IP++;
                    A = sub_with_carry(data);
                    use_cycles(4);
                }
                break;

            case INSTR_6502_SBC_ABSOLUTE:
                {
                    Byte data = get_data_absolute(memory);
                    IP++;
                    IP++;
                    A = sub_with_carry(data);
                    use_cycles(4);
                }
                break;

            case INSTR_6502_SBC_ABSOLUTE_X:
                {
                    Byte data = get_data_absolute(memory, X);
                    IP++;
                    IP++;
                    A = sub_with_carry(data);
                    use_cycles(4);
                    if (page_crossed)
                    {
                        use_cycles(1);
                    }
                }
                break;

            case INSTR_6502_SBC_ABSOLUTE_Y:
                {
                    Byte data = get_data_absolute(memory, Y);
                    IP++;
                    IP++;
                    A = sub_with_carry(data);
                    use_cycles(4);
                    if (page_crossed)
                    {
                        use_cycles(1);
                    }
                }
                break;

            case INSTR_6502_SBC_INDIRECT_X:
                {
                    Byte data = get_data_indexed_indirect(memory, X);
                    IP++;
                    A = sub_with_carry(data);
                    use_cycles(6);
                }
                break;

            case INSTR_6502_SBC_INDIRECT_Y:
                {
                    Byte data = get_data_indirect_indexed(memory, Y);
                    IP++;
                    A = sub_with_carry(data);
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

            case INSTR_6502_INC_ZEROPAGE:
                {
                    Byte value = get_data_zeropage(memory);
                    value++;
                    set_data_zeropage(memory, value);
                    IP++;
                    INC_set_CPU_flags(value);
                    use_cycles(5);
                }
                break;

            case INSTR_6502_INC_ZEROPAGE_X:
                {
                    Byte value = get_data_zeropage(memory, X);
                    value++;
                    set_data_zeropage(memory, value, X);
                    IP++;
                    INC_set_CPU_flags(value);
                    use_cycles(6);
                }
                break;

            case INSTR_6502_INC_ABSOLUTE:
                {
                    Byte value = get_data_absolute(memory);
                    value++;
                    set_data_absolute(memory, value);
                    IP++;
                    IP++;
                    INC_set_CPU_flags(value);
                    use_cycles(6);
                }
                break;

            case INSTR_6502_INC_ABSOLUTE_X:
                {
                    Byte value = get_data_absolute(memory, X);
                    value++;
                    set_data_absolute(memory, value, X);
                    IP++;
                    IP++;
                    INC_set_CPU_flags(value);
                    use_cycles(7);
                }
                break;

            case INSTR_6502_DEC_ZEROPAGE:
                {
                    Byte value = get_data_zeropage(memory);
                    value--;
                    set_data_zeropage(memory, value);
                    IP++;
                    DEC_set_CPU_flags(value);
                    use_cycles(5);
                }
                break;

            case INSTR_6502_DEC_ZEROPAGE_X:
                {
                    Byte value = get_data_zeropage(memory, X);
                    value--;
                    set_data_zeropage(memory, value, X);
                    IP++;
                    DEC_set_CPU_flags(value);
                    use_cycles(6);
                }
                break;

            case INSTR_6502_DEC_ABSOLUTE:
                {
                    Byte value = get_data_absolute(memory);
                    value--;
                    set_data_absolute(memory, value);
                    IP++;
                    IP++;
                    DEC_set_CPU_flags(value);
                    use_cycles(6);
                }
                break;

            case INSTR_6502_DEC_ABSOLUTE_X:
                {
                    Byte value = get_data_absolute(memory, X);
                    value--;
                    set_data_absolute(memory, value, X);
                    IP++;
                    IP++;
                    DEC_set_CPU_flags(value);
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
    return CONTINUE;
}

/** \brief Sets the value of a byte in memory, addressed by a full word.
 * \param memory Reference to system memory.
 * \param address The 16-bit address of the memory location to write to.
 * \param value The 8-bit value to write into memory.
 */
void CPU::set_byte(Memory& memory, const Word address, Byte const value)
{
    memory[address] = value;
}

/** \brief Gets a byte from memory, addressed by the current instruction pointer.
 * \param memory Reference to system memory.
 * \return A byte from memory.
 */
Byte CPU::get_byte(Memory& memory) const
{
    return memory[IP];
}

/** \brief Gets a byte from the zero page in memory, from a specified 8-bit address.
 * \param memory Reference to system memory.
 * \param address The address in the zero page from which to fetch data.
 * \return 8-bit value from specified memory location.
 */
Byte CPU::get_byte(Memory& memory, const Byte address) const
{
    return memory[address];
}

/** \brief Gets a byte from system memory.
 * \param memory Reference to system memory.
 * \param address A full 16-bit address from which to getch data.
 * \return 8-bit value from memory.
 */
Byte CPU::get_byte(Memory& memory, const Word address) const
{
    return memory[address];
}

/** \brief Gets a full word from memory, addressed by the current instruction pointer.
 * \param memory Reference to system memory.
 * \return 16-bit value from memory.
 */
Word CPU::get_word(Memory& memory) const
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
Word CPU::get_word(Memory& memory, const Byte address) const
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
Word CPU::get_word(Memory& memory, const Word address) const
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
Byte CPU::get_data_absolute(Memory& memory) const
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
void CPU::set_data_absolute(Memory& memory, const Byte data)
{
    Word address = get_word(memory);
    memory[address] = data;
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in the 16-bit address at the current instruction pointer.
 * \param index An offset from the specified memory location.
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

/** \brief Performs subtracton of accumulator and data, setting the carry bit as required.
 * \param data A byte of data to be subtracted from the accumulator.
 * \return A byte to be stored in the accumulator.
 */
Byte CPU::sub_with_carry(const Byte data)
{
    return add_with_carry(~data);
}

/** \brief Performs addition of accumulator and data, setting the carry bit as required.
 * \param data A byte of data to be added to the accumulator.
 * \return A byte to be stored in the accumulator.
 */
Byte CPU::add_with_carry(const Byte data)
{
    // TODO Not at all confident in this implementation, esp. V = ...
    // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
    Word result = static_cast<Word>(data + A + C);
    Z = (result == 0);
    C = (result > 255);
    N = (result & BIT7);
    V = ((A ^ result) & (data ^ result) & BIT7) != 0;
    return static_cast<Byte>(result & 0xFF);
}

/** \brief Fetches a byte using relative addressing mode.
 * \param memory A reference to a memory array object.
 * \return A Byte from memory.
 */
Byte CPU::get_data_relative(Memory& memory) const
{
    return get_data_immediate(memory);
}

/** \brief Fetches a byte using immediate addressing mode.
 * \param memory A reference to a memory array object.
 * \return A Byte from memory.
 */
Byte CPU::get_data_immediate(Memory& memory) const
{
    return get_byte(memory);
}

/** \brief Fetches a byte using zpg addressing mode.
 * \param memory A reference to a memory array object.
 * \return A Byte from memory.
 */
Byte CPU::get_data_zeropage(Memory& memory) const
{
    Byte data_address = get_byte(memory);
    return get_byte(memory, data_address);
}

/** \brief Fetches a byte using zpg addressing mode with an index, typically X or Y register.
 * \param memory A reference to a memory array object.
 * \param index An index into a memory region.
 * \return A Byte from memory.
 */
Byte CPU::get_data_zeropage(Memory& memory, const Byte index) const
{
    Byte data_address = get_byte(memory) + index;
    return get_byte(memory, data_address);
}

/** \brief Get a word from the zero page with full wrapping.
 * \param memory Reference to system memory.
 * \param address The zero page address of the first byte to be read.
 * \return 16-bit value from the zero page.
 */
Word CPU::get_word_zpg_wrap(Memory& memory, const Byte address) const
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
Byte CPU::get_data_indexed_indirect(Memory& memory, const Byte index) const
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
    if (distance & BIT7)
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

/** \brief Moves the stack pointer up by one position and returns the value found at that address.
 * \param memory Reference to system memory.
 * \return A byte from the stack.
 */
Byte CPU::pop_from_stack(Memory& memory)
{
    SP++;
    return memory[SP];
}

/** \brief Encode all CPU flags into a single byte.
 * \return 8-bit value containing all CPU flags.
 */
Byte CPU::flags_as_byte() const
{
    return static_cast<Byte>(
          (N << 7) 
        | (V << 6) 
        | (true << 5) 
        | (B << 4) 
        | (D << 3) 
        | (I << 2) 
        | (Z << 1) 
        | (C << 0)
    );
}

/** \brief Increases the number of available CPU cycles.
 * \param cycles_to_add How much to increase the count of available cycles.
 */
void CPU::add_cycles(int cycles_to_add)
{
    cycles_available += cycles_to_add;
}

/** \brief Reduce the number of available CPU cycles.
 * \param cycles_to_use How much to reduce the count of available cycles.
 */
void CPU::use_cycles(int cycles_to_use)
{
    cycles_available -= cycles_to_use;
}

/** \brief Set the instruction pointer of the CPU.
 * \param newIP The new value of the instruction pointer.
 */
void CPU::setIP(const Word newIP)
{
    // Bounds checking on IP value.
    this->IP = newIP;
}

/** \brief Set the stack pointer of the CPU.
 * \param newSP The new value for the stack pointer.
 */
void CPU::setSP(const Word newSP)
{
    // Bounds checking on SP value.
    this->SP = newSP;
}

/** \brief Get the current value of the CPU instruction pointer.
 * \return Current IP value.
 */
Word CPU::getIP() const
{
    return this->IP;
}

/** \brief Get the current value of the CPU stack pointer.
 * \return Current SP value.
 */
Word CPU::getSP() const
{
    return this->SP;
}
