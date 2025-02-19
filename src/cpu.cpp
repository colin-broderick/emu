#include <iostream>
#include <iomanip>

#include "cpu.hpp"
#include "memory.hpp"

/** \brief CPU constructor; sets initial configuration.
 *
 * Sets initial configuration including IP = 0x0000, SP = 0x01FF, all flags = false. */
Cpu6502::Cpu6502()
{
    m_instruction_pointer = 0x0000;
    m_stack_pointer = 0x01FF;
    C = Z = I = D = B = V = N = false;
}

/** \brief CPU constructor which allows custom setting of IP and SP.
 * \param ip The starting instruction pointer.
 * \param sp The starting stack pointer.
 *
 * Sets initial values of the SP and IP to the specified value, and sets all flags = false.
 */
Cpu6502::Cpu6502(const Word ip, const Word sp)
{
    m_instruction_pointer = ip;
    m_stack_pointer = sp;
    C = Z = I = D = B = V = N = false;
}

/** \brief Runs the loaded program while CPU cycles are available to spend.
 * \param memory A reference to the main memory of the system. The CPU reads and writes this memory.
 * \param cycles The new supply of cycles.
 * \return Code indicating whether to continue or stop running the CPU.
 *
 * The CPU stores a count of available cycles. When the run function is called, the
 * supplied cycles will be added to this internal count, and the CPU will then execute
 * instructions until the internal supply is exhausted.
 *
 * The internal supply can become slightly negative, since instructions take different
 * numbers of cycles to execute, and this cannot be predicted ahead of time. This
 * will be reflected in the next call to run(), since the negative supply is remembered.
 */
Cpu6502::ReturnCode Cpu6502::run(Memory &memory, const int cycles)
{
    add_cycles(cycles);

    while (cycles_available > 0)
    {
        // Reset the page crossing flag in case it was left on from the last iteration.
        page_crossed = false;

        // TODO Interrupt handler should go here.

        // Grab an instruction from RAM.
        Byte instruction = get_data_immediate(memory);

        // We increment the instruction pointer to point to the next byte in memory.
        m_instruction_pointer++;

        LOG(
            "N" << N << " " << "V" << V << " " << "B" << B << " " << "D" << D << " " << "I" << I << " " << "Z" << Z
                << " " << "C" << C << "    " << std::hex << "IP:" << std::setw(4) << (int)m_instruction_pointer << "   " << "SP:"
                << std::setw(4) << (int)m_stack_pointer << "   " << "A:" << std::setw(2) << (int)A << "   " << "X:" << std::setw(2)
                << (int)X << "   " << "Y:" << std::setw(2) << (int)Y << "   " << instruction_names[instruction]);

        switch (instruction)
        {
        case INSTR_6502_LDA_IMMEDIATE:
            A = get_data_immediate(memory);
            m_instruction_pointer++;
            LDA_set_CPU_flags();
            use_cycles(2);
            break;

        case INSTR_6502_LDA_ZEROPAGE:
            A = get_data_zeropage(memory);
            m_instruction_pointer++;
            LDA_set_CPU_flags();
            use_cycles(3);
            break;

        case INSTR_6502_LDA_ZEROPAGE_X:
            A = get_data_zeropage(memory, X);
            m_instruction_pointer++;
            LDA_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_LDA_ABSOLUTE:
            A = get_data_absolute(memory);
            m_instruction_pointer++;
            m_instruction_pointer++;
            LDA_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_LDA_ABSOLUTE_X:
            A = get_data_absolute(memory, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
            LDA_set_CPU_flags();
            use_cycles(4);
            if (page_crossed)
            {
                use_cycles(1);
            }
            break;

        case INSTR_6502_LDA_ABSOLUTE_Y:
            A = get_data_absolute(memory, Y);
            m_instruction_pointer++;
            m_instruction_pointer++;
            LDA_set_CPU_flags();
            use_cycles(4);
            if (page_crossed)
            {
                use_cycles(1);
            }
            break;

        case INSTR_6502_LDA_INDIRECT_X:
            A = get_data_indexed_indirect(memory, X);
            m_instruction_pointer++;
            LDA_set_CPU_flags();
            use_cycles(6);
            break;

        case INSTR_6502_LDA_INDIRECT_Y:
            A = get_data_indirect_indexed(memory, Y);
            m_instruction_pointer++;
            LDA_set_CPU_flags();
            use_cycles(5);
            if (page_crossed)
            {
                use_cycles(1);
            }
            break;

        case INSTR_6502_LDY_IMMEDIATE:
            Y = get_data_immediate(memory);
            m_instruction_pointer++;
            LDY_set_CPU_flags();
            use_cycles(2);
            break;

        case INSTR_6502_LDY_ZEROPAGE:
            Y = get_data_zeropage(memory);
            m_instruction_pointer++;
            LDY_set_CPU_flags();
            use_cycles(3);
            break;

        case INSTR_6502_LDY_ZEROPAGE_X:
            Y = get_data_zeropage(memory, X);
            m_instruction_pointer++;
            LDY_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_LDY_ABSOLUTE:
            Y = get_data_absolute(memory);
            m_instruction_pointer++;
            m_instruction_pointer++;
            LDY_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_LDY_ABSOLUTE_X:
            Y = get_data_absolute(memory, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            CMP_set_CPU_flags(data);
            use_cycles(2);
        }
        break;

        case INSTR_6502_CMP_ZEROPAGE:
        {
            Byte data = get_data_zeropage(memory);
            m_instruction_pointer++;
            CMP_set_CPU_flags(data);
            use_cycles(3);
        }
        break;

        case INSTR_6502_CMP_ZEROPAGE_X:
        {
            Byte data = get_data_zeropage(memory, X);
            m_instruction_pointer++;
            CMP_set_CPU_flags(data);
            use_cycles(4);
        }
        break;

        case INSTR_6502_CMP_ABSOLUTE:
        {
            Byte data = get_data_absolute(memory);
            m_instruction_pointer++;
            m_instruction_pointer++;
            CMP_set_CPU_flags(data);
            use_cycles(4);
        }
        break;

        case INSTR_6502_CMP_ABSOLUTE_X:
        {
            Byte data = get_data_absolute(memory, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            CMP_set_CPU_flags(data);
            use_cycles(6);
        }
        break;

        case INSTR_6502_CMP_INDIRECT_Y:
        {
            Byte data = get_data_indirect_indexed(memory, Y);
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            EOR_set_CPU_flags();
            use_cycles(2);
            break;

        case INSTR_6502_EOR_ZEROPAGE:
            A = A ^ get_data_zeropage(memory);
            m_instruction_pointer++;
            EOR_set_CPU_flags();
            use_cycles(3);
            break;

        case INSTR_6502_EOR_ZEROPAGE_X:
            A = A ^ get_data_zeropage(memory, X);
            m_instruction_pointer++;
            EOR_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_EOR_ABSOLUTE:
            A = A ^ get_data_absolute(memory);
            m_instruction_pointer++;
            m_instruction_pointer++;
            EOR_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_EOR_ABSOLUTE_X:
            A = A ^ get_data_absolute(memory, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
            EOR_set_CPU_flags();
            use_cycles(4);
            if (page_crossed)
            {
                use_cycles(1);
            }
            break;

        case INSTR_6502_EOR_ABSOLUTE_Y:
            A = A ^ get_data_absolute(memory, Y);
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            EOR_set_CPU_flags();
            use_cycles(6);
        }
        break;

        case INSTR_6502_EOR_INDIRECT_Y:
            A = A ^ get_data_indirect_indexed(memory, Y);
            m_instruction_pointer++;
            EOR_set_CPU_flags();
            use_cycles(5);
            if (page_crossed)
            {
                use_cycles(1);
            }
            break;

        case INSTR_6502_STA_ZEROPAGE:
            set_data_zeropage(memory, A);
            m_instruction_pointer++;
            use_cycles(3);
            break;

        case INSTR_6502_STA_ZEROPAGE_X:
            set_data_zeropage(memory, A, X);
            m_instruction_pointer++;
            use_cycles(4);
            break;

        case INSTR_6502_STA_ABSOLUTE:
            set_data_absolute(memory, A);
            m_instruction_pointer++;
            m_instruction_pointer++;
            use_cycles(4);
            break;

        case INSTR_6502_STA_ABSOLUTE_X:
            set_data_absolute(memory, A, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
            use_cycles(5);
            break;

        case INSTR_6502_STA_ABSOLUTE_Y:
            set_data_absolute(memory, A, Y);
            m_instruction_pointer++;
            m_instruction_pointer++;
            use_cycles(5);
            break;

        case INSTR_6502_STA_INDIRECT_X:
            set_data_indexed_indirect(memory, A, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
            use_cycles(6);
            break;

        case INSTR_6502_STA_INDIRECT_Y:
            set_data_indirect_indexed(memory, A, Y);
            m_instruction_pointer++;
            m_instruction_pointer++;
            use_cycles(6);
            break;

        case INSTR_6502_TXS:
            m_stack_pointer = static_cast<Word>(0x0100 | X);
            use_cycles(2);
            break;

        case INSTR_6502_TSX:
            X = static_cast<Byte>(m_stack_pointer & 0x00FF);
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
            m_instruction_pointer++;
            use_cycles(3);
            break;

        case INSTR_6502_STX_ZEROPAGE_Y:
            set_data_absolute(memory, X, Y);
            m_instruction_pointer++;
            use_cycles(4);
            break;

        case INSTR_6502_STX_ABSOLUTE:
            set_data_absolute(memory, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
            use_cycles(4);
            break;

        case INSTR_6502_STY_ZEROPAGE:
            set_data_zeropage(memory, Y);
            m_instruction_pointer++;
            use_cycles(3);
            break;

        case INSTR_6502_STY_ZEROPAGE_X:
            set_data_zeropage(memory, Y, X);
            m_instruction_pointer++;
            use_cycles(4);
            break;

        case INSTR_6502_STY_ABSOLUTE:
            set_data_absolute(memory, Y);
            m_instruction_pointer++;
            m_instruction_pointer++;
            use_cycles(4);
            break;

        case INSTR_6502_TAX:
            X = A;
            TAX_set_CPU_flags();
            use_cycles(2);
            break;

        case INSTR_6502_TAY:
            Y = A;
            Z = (Y == 0);
            N = (Y & BIT7);
            use_cycles(4);
            break;

        case INSTR_6502_TXA:
            A = X;
            TXA_set_CPU_flags();
            use_cycles(2);
            break;

        case INSTR_6502_INX:
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
            X = get_data_immediate(memory);
            m_instruction_pointer++;
            LDX_set_CPU_flags();
            use_cycles(2);
            break;

        case INSTR_6502_LDX_ZEROPAGE:
            X = get_data_zeropage(memory);
            m_instruction_pointer++;
            LDX_set_CPU_flags();
            use_cycles(3);
            break;

        case INSTR_6502_LDX_ZEROPAGE_Y:
            X = get_data_zeropage(memory, Y);
            m_instruction_pointer++;
            LDX_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_LDX_ABSOLUTE:
            X = get_data_absolute(memory);
            m_instruction_pointer++;
            m_instruction_pointer++;
            LDX_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_LDX_ABSOLUTE_Y:
            X = get_data_absolute(memory, Y);
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            CPX_set_CPU_flags(result);
            use_cycles(2);
        }
        break;

        case INSTR_6502_CPX_ZEROPAGE:
        {
            int result = X - get_data_zeropage(memory);
            m_instruction_pointer++;
            CPX_set_CPU_flags(result);
            use_cycles(3);
        }
        break;

        case INSTR_6502_CPX_ABSOLUTE:
        {
            int result = X - get_data_absolute(memory);
            m_instruction_pointer++;
            CPX_set_CPU_flags(result);
            use_cycles(4);
        }
        break;

        case INSTR_6502_CPY_IMMEDIATE:
        {
            int result = Y - get_data_immediate(memory);
            CPY_set_CPU_flags(result);
            use_cycles(2);
            m_instruction_pointer++;
        }
        break;

        case INSTR_6502_CPY_ZEROPAGE:
        {
            int result = Y - get_data_zeropage(memory);
            CPY_set_CPU_flags(result);
            use_cycles(3);
            m_instruction_pointer++;
        }
        break;

        case INSTR_6502_CPY_ABSOLUTE:
        {
            int result = Y - get_data_absolute(memory);
            CPY_set_CPU_flags(result);
            use_cycles(4);
            m_instruction_pointer++;
        }
        break;

        case INSTR_6502_BNE_RELATIVE:
        {
            // Remember the starting page so we know if we've moved to a new page.
            Byte current_page = static_cast<Byte>(m_instruction_pointer >> 8);
            use_cycles(2);

            if (!Z)
            {
                Byte dist = get_data_relative(memory);
                branch_relative(dist);
                use_cycles(1);
            }
            m_instruction_pointer++;

            // This should take two additional clock cycles if the branch leads to a new page.
            Byte new_page = static_cast<Byte>(m_instruction_pointer >> 8);
            if (current_page != new_page)
            {
                use_cycles(2);
            }
        }
        break;

        case INSTR_6502_BEQ_RELATIVE:
        {
            // Remember the starting page so we know if we've moved to a new page.
            Byte current_page = static_cast<Byte>(m_instruction_pointer >> 8);
            use_cycles(2);

            if (Z)
            {
                Byte dist = get_data_relative(memory);
                branch_relative(dist);
                use_cycles(1);
            }
            m_instruction_pointer++;

            // This should take two additional clock cycles if the branch leads to a new page.
            Byte new_page = static_cast<Byte>(m_instruction_pointer >> 8);
            if (current_page != new_page)
            {
                use_cycles(2);
            }
        }
        break;

        case INSTR_6502_BMI_RELATIVE:
        {
            Byte current_page = static_cast<Byte>(m_instruction_pointer >> 8);
            use_cycles(2);

            if (N)
            {
                Byte dist = get_data_relative(memory);
                branch_relative(dist);
                use_cycles(1);
            }
            m_instruction_pointer++;

            Byte new_page = static_cast<Byte>(m_instruction_pointer >> 8);
            if (current_page != new_page)
            {
                use_cycles(2);
            }
        }
        break;

        case INSTR_6502_BPL_RELATIVE:
        {
            Byte current_page = static_cast<Byte>(m_instruction_pointer >> 8);
            use_cycles(2);

            if (!N)
            {
                Byte dist = get_data_relative(memory);
                branch_relative(dist);
                use_cycles(1);
            }
            m_instruction_pointer++;

            Byte new_page = static_cast<Byte>(m_instruction_pointer >> 8);
            if (current_page != new_page)
            {
                use_cycles(2);
            }
        }
        break;

        case INSTR_6502_BVC_RELATIVE:
        {
            Byte current_page = static_cast<Byte>(m_instruction_pointer >> 8);
            use_cycles(2);

            if (!V)
            {
                Byte dist = get_data_relative(memory);
                branch_relative(dist);
                use_cycles(1);
            }
            m_instruction_pointer++;

            Byte new_page = static_cast<Byte>(m_instruction_pointer >> 8);
            if (current_page != new_page)
            {
                use_cycles(2);
            }
        }
        break;

        case INSTR_6502_BVS_RELATIVE:
        {
            Byte current_page = static_cast<Byte>(m_instruction_pointer >> 8);
            use_cycles(2);

            if (V)
            {
                Byte dist = get_data_relative(memory);
                branch_relative(dist);
                use_cycles(1);
            }
            m_instruction_pointer++;

            Byte new_page = static_cast<Byte>(m_instruction_pointer >> 8);
            if (current_page != new_page)
            {
                use_cycles(2);
            }
        }
        break;

        case INSTR_6502_BCC_RELATIVE:
        {
            Byte current_page = static_cast<Byte>(m_instruction_pointer >> 8);
            use_cycles(2);

            if (!C)
            {
                Byte dist = get_data_relative(memory);
                branch_relative(dist);
                use_cycles(1);
            }
            m_instruction_pointer++;

            Byte new_page = static_cast<Byte>(m_instruction_pointer >> 8);
            if (current_page != new_page)
            {
                use_cycles(2);
            }
        }
        break;

        case INSTR_6502_BCS_RELATIVE:
        {
            Byte current_page = static_cast<Byte>(m_instruction_pointer >> 8);
            use_cycles(2);

            if (C)
            {
                Byte dist = get_data_relative(memory);
                branch_relative(dist);
                use_cycles(1);
            }
            m_instruction_pointer++;

            Byte new_page = static_cast<Byte>(m_instruction_pointer >> 8);
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
            m_instruction_pointer++;
            ORA_set_CPU_flags();
            use_cycles(2);
            break;

        case INSTR_6502_ORA_ZEROPAGE:
            A |= get_data_zeropage(memory);
            m_instruction_pointer++;
            ORA_set_CPU_flags();
            use_cycles(3);
            break;

        case INSTR_6502_ORA_ZEROPAGE_X:
            A |= get_data_zeropage(memory, X);
            m_instruction_pointer++;
            ORA_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_ORA_ABSOLUTE:
            A |= get_data_absolute(memory);
            m_instruction_pointer++;
            m_instruction_pointer++;
            ORA_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_ORA_ABSOLUTE_X:
            A |= get_data_absolute(memory, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
            ORA_set_CPU_flags();
            use_cycles(4);
            if (page_crossed)
            {
                use_cycles(1);
            }
            break;

        case INSTR_6502_ORA_ABSOLUTE_Y:
            A |= get_data_absolute(memory, Y);
            m_instruction_pointer++;
            m_instruction_pointer++;
            ORA_set_CPU_flags();
            use_cycles(4);
            if (page_crossed)
            {
                use_cycles(1);
            }
            break;

        case INSTR_6502_ORA_INDIRECT_X:
            A |= get_data_indexed_indirect(memory, X);
            m_instruction_pointer++;
            ORA_set_CPU_flags();
            use_cycles(6);
            break;

        case INSTR_6502_ORA_INDIRECT_Y:
            A |= get_data_indirect_indexed(memory, Y);
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            Z = (result == 0);
            V = (result & BIT6);
            N = (result & BIT7);
            use_cycles(3);
        }
        break;

        case INSTR_6502_BIT_ABSOLUTE:
        {
            Byte result = A & get_data_absolute(memory);
            m_instruction_pointer++;
            m_instruction_pointer++;
            Z = (result == 0);
            V = (result & BIT6);
            N = (result & BIT7);
            use_cycles(4);
        }
        break;

        case INSTR_6502_ASL_ACCUMULATOR:
            C = (A & BIT7);
            A = static_cast<Byte>(A << 1);
            Z = (A == 0);
            N = (A & BIT7);
            use_cycles(2);
            break;

        case INSTR_6502_ASL_ZEROPAGE:
        {
            Byte data = get_data_zeropage(memory);
            C = (data & BIT7);
            data = static_cast<Byte>(data << 1);
            Z = (data == 0);
            N = (data & BIT7);
            set_data_zeropage(memory, data);
            m_instruction_pointer++;
            use_cycles(5);
        }
        break;

        case INSTR_6502_ASL_ZEROPAGE_X:
        {
            Byte data = get_data_zeropage(memory, X);
            C = (data & BIT7);
            data = static_cast<Byte>(data << 1);
            Z = (data == 0);
            N = (data & BIT7);
            set_data_zeropage(memory, data, X);
            m_instruction_pointer++;
            use_cycles(6);
        }
        break;

        case INSTR_6502_ASL_ABSOLUTE:
        {
            Byte data = get_data_absolute(memory);
            C = (data & BIT7);
            data = static_cast<Byte>(data << 1);
            Z = (data == 0);
            N = (data & BIT7);
            set_data_absolute(memory, data);
            m_instruction_pointer++;
            m_instruction_pointer++;
            use_cycles(6);
        }
        break;

        case INSTR_6502_ASL_ABSOLUTE_X:
        {
            Byte data = get_data_absolute(memory, X);
            C = (data & BIT7);
            data = static_cast<Byte>(data << 1);
            Z = (data == 0);
            N = (data & BIT7);
            set_data_absolute(memory, data, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
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
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
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
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            m_instruction_pointer++;
            use_cycles(7);
        }
        break;

        case INSTR_6502_ROR_ACCUMULATOR:
        {
            Byte tempC = static_cast<Byte>(C << 7);
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
            Byte tempC = static_cast<Byte>(C << 7);
            C = (data & BIT0);
            data = (data >> 1) | tempC;
            Z = (data == 0);
            N = (data & BIT7);
            set_data_zeropage(memory, data);
            m_instruction_pointer++;
            use_cycles(5);
        }
        break;

        case INSTR_6502_ROR_ZEROPAGE_X:
        {
            Byte data = get_data_zeropage(memory, X);
            Byte tempC = static_cast<Byte>(C << 7);
            C = (data & BIT0);
            data = (data >> 1) | tempC;
            Z = (data == 0);
            N = (data & BIT7);
            set_data_zeropage(memory, data, X);
            m_instruction_pointer++;
            use_cycles(6);
        }
        break;

        case INSTR_6502_ROR_ABSOLUTE:
        {
            Byte data = get_data_absolute(memory);
            Byte tempC = static_cast<Byte>(C << 7);
            C = (data & BIT0);
            data = (data >> 1) | tempC;
            Z = (data == 0);
            N = (data & BIT7);
            set_data_absolute(memory, data);
            m_instruction_pointer++;
            m_instruction_pointer++;
            use_cycles(6);
        }
        break;

        case INSTR_6502_ROR_ABSOLUTE_X:
        {
            Byte data = get_data_absolute(memory, X);
            Byte tempC = static_cast<Byte>(C << 7);
            C = (data & BIT0);
            data = (data >> 1) | tempC;
            Z = (data == 0);
            N = (data & BIT7);
            set_data_absolute(memory, data, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            A = add_with_carry(data);
            use_cycles(2);
        }
        break;

        case INSTR_6502_ADC_ZEROPAGE:
        {
            Byte data = get_data_zeropage(memory);
            m_instruction_pointer++;
            A = add_with_carry(data);
            use_cycles(3);
        }
        break;

        case INSTR_6502_ADC_ZEROPAGE_X:
        {
            Byte data = get_data_zeropage(memory, X);
            m_instruction_pointer++;
            A = add_with_carry(data);
            use_cycles(4);
        }
        break;

        case INSTR_6502_ADC_ABSOLUTE:
        {
            Byte data = get_data_absolute(memory);
            m_instruction_pointer++;
            m_instruction_pointer++;
            A = add_with_carry(data);
            use_cycles(4);
        }
        break;

        case INSTR_6502_ADC_ABSOLUTE_X:
        {
            Byte data = get_data_absolute(memory, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            A = add_with_carry(data);
            use_cycles(6);
        }
        break;

        case INSTR_6502_ADC_INDIRECT_Y:
        {
            Byte data = get_data_indirect_indexed(memory, Y);
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            A = sub_with_carry(data);
            use_cycles(2);
        }
        break;

        case INSTR_6502_SBC_ZEROPAGE:
        {
            Byte data = get_data_zeropage(memory);
            m_instruction_pointer++;
            A = sub_with_carry(data);
            use_cycles(3);
        }
        break;

        case INSTR_6502_SBC_ZEROPAGE_X:
        {
            Byte data = get_data_zeropage(memory, X);
            m_instruction_pointer++;
            A = sub_with_carry(data);
            use_cycles(4);
        }
        break;

        case INSTR_6502_SBC_ABSOLUTE:
        {
            Byte data = get_data_absolute(memory);
            m_instruction_pointer++;
            m_instruction_pointer++;
            A = sub_with_carry(data);
            use_cycles(4);
        }
        break;

        case INSTR_6502_SBC_ABSOLUTE_X:
        {
            Byte data = get_data_absolute(memory, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            m_instruction_pointer++;
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
            m_instruction_pointer++;
            A = sub_with_carry(data);
            use_cycles(6);
        }
        break;

        case INSTR_6502_SBC_INDIRECT_Y:
        {
            Byte data = get_data_indirect_indexed(memory, Y);
            m_instruction_pointer++;
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
            memory[m_stack_pointer] = A;
            m_stack_pointer--;
            use_cycles(3);
            break;

        case INSTR_6502_PHP:
            memory[m_stack_pointer] = flags_as_byte();
            m_stack_pointer--;
            use_cycles(3);
            break;

        case INSTR_6502_PLA:
            A = memory[m_stack_pointer];
            m_stack_pointer++;
            use_cycles(4);
            LDA_set_CPU_flags();
            break;

        case INSTR_6502_NOP:
            use_cycles(2);
            break;

        case INSTR_6502_BRK:
            use_cycles(7);
            B = true;

            memory[m_stack_pointer] = static_cast<Byte>(m_instruction_pointer >> 8);
            m_stack_pointer--;
            memory[m_stack_pointer] = static_cast<Byte>(m_instruction_pointer & 0xFF);
            m_stack_pointer--;
            memory[m_stack_pointer] = flags_as_byte();
            m_stack_pointer--;

            std::cout << "BRK reached" << std::endl;
            return ReturnCode::BREAK;

        case INSTR_6502_JSR_ABSOLUTE:
        {
            Word target_address = get_word(memory);
            m_instruction_pointer++;
            memory[m_stack_pointer] = static_cast<Byte>(m_instruction_pointer >> 8);
            m_stack_pointer--;
            memory[m_stack_pointer] = static_cast<Byte>(m_instruction_pointer & 0xFF);
            m_stack_pointer--;
            m_instruction_pointer = target_address;
            use_cycles(6);
        }
        break;

        case INSTR_6502_RTS:
        {
            m_stack_pointer++;
            Word pointer = get_word(memory, m_stack_pointer);
            m_stack_pointer++;

            m_instruction_pointer = pointer + 1;

            use_cycles(6);
        }
        break;

        case INSTR_6502_JMP_ABSOLUTE:
            // TODO I don't know if I'm supposed to jump to the address in memory at the IP,
            // or the address specified by that memory location.
            m_instruction_pointer = get_word(memory);
            use_cycles(3);
            break;

        case INSTR_6502_INC_ZEROPAGE:
        {
            Byte value = get_data_zeropage(memory);
            value++;
            set_data_zeropage(memory, value);
            m_instruction_pointer++;
            INC_set_CPU_flags(value);
            use_cycles(5);
        }
        break;

        case INSTR_6502_INC_ZEROPAGE_X:
        {
            Byte value = get_data_zeropage(memory, X);
            value++;
            set_data_zeropage(memory, value, X);
            m_instruction_pointer++;
            INC_set_CPU_flags(value);
            use_cycles(6);
        }
        break;

        case INSTR_6502_INC_ABSOLUTE:
        {
            Byte value = get_data_absolute(memory);
            value++;
            set_data_absolute(memory, value);
            m_instruction_pointer++;
            m_instruction_pointer++;
            INC_set_CPU_flags(value);
            use_cycles(6);
        }
        break;

        case INSTR_6502_INC_ABSOLUTE_X:
        {
            Byte value = get_data_absolute(memory, X);
            value++;
            set_data_absolute(memory, value, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
            INC_set_CPU_flags(value);
            use_cycles(7);
        }
        break;

        case INSTR_6502_DEC_ZEROPAGE:
        {
            Byte value = get_data_zeropage(memory);
            value--;
            set_data_zeropage(memory, value);
            m_instruction_pointer++;
            DEC_set_CPU_flags(value);
            use_cycles(5);
        }
        break;

        case INSTR_6502_DEC_ZEROPAGE_X:
        {
            Byte value = get_data_zeropage(memory, X);
            value--;
            set_data_zeropage(memory, value, X);
            m_instruction_pointer++;
            DEC_set_CPU_flags(value);
            use_cycles(6);
        }
        break;

        case INSTR_6502_DEC_ABSOLUTE:
        {
            Byte value = get_data_absolute(memory);
            value--;
            set_data_absolute(memory, value);
            m_instruction_pointer++;
            m_instruction_pointer++;
            DEC_set_CPU_flags(value);
            use_cycles(6);
        }
        break;

        case INSTR_6502_DEC_ABSOLUTE_X:
        {
            Byte value = get_data_absolute(memory, X);
            value--;
            set_data_absolute(memory, value, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
            DEC_set_CPU_flags(value);
            use_cycles(7);
        }
        break;

        case INSTR_6502_JMP_INDIRECT:
        {
            Word lookup_address = get_word(memory);
            m_instruction_pointer = get_word(memory, lookup_address);

            use_cycles(5);
        }
        break;

        case INSTR_6502_AND_IMMEDIATE:
            A &= get_data_immediate(memory);
            m_instruction_pointer++;
            AND_set_CPU_flags();
            use_cycles(2);
            break;

        case INSTR_6502_AND_ZEROPAGE_X:
            A &= get_data_zeropage(memory, X);
            m_instruction_pointer++;
            AND_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_AND_ZEROPAGE:
            A &= get_data_zeropage(memory);
            m_instruction_pointer++;
            AND_set_CPU_flags();
            use_cycles(3);
            break;

        case INSTR_6502_AND_ABSOLUTE:
            A &= get_data_absolute(memory);
            m_instruction_pointer++;
            m_instruction_pointer++;
            AND_set_CPU_flags();
            use_cycles(4);
            break;

        case INSTR_6502_AND_ABSOLUTE_X:
            A &= get_data_absolute(memory, X);
            m_instruction_pointer++;
            m_instruction_pointer++;
            AND_set_CPU_flags();
            use_cycles(4);
            if (page_crossed)
            {
                use_cycles(1);
            }
            break;

        case INSTR_6502_AND_ABSOLUTE_Y:
            A &= get_data_absolute(memory, Y);
            m_instruction_pointer++;
            m_instruction_pointer++;
            AND_set_CPU_flags();
            use_cycles(4);
            if (page_crossed)
            {
                use_cycles(1);
            }
            break;

        case INSTR_6502_AND_INDIRECT_X:
            A &= get_data_indexed_indirect(memory, X);
            m_instruction_pointer++;
            AND_set_CPU_flags();
            use_cycles(6);
            break;

        case INSTR_6502_AND_INDIRECT_Y:
            A &= get_data_indirect_indexed(memory, Y);
            m_instruction_pointer++;
            AND_set_CPU_flags();
            use_cycles(5);
            if (page_crossed)
            {
                use_cycles(1);
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

/** \brief Sets the value of a byte in memory, addressed by a full word.
 * \param memory Reference to system memory.
 * \param address The 16-bit address of the memory location to write to.
 * \param value The 8-bit value to write into memory.
 */
void Cpu6502::set_byte(Memory &memory, const Word address, Byte const value)
{
    memory[address] = value;
}

/** \brief Gets a byte from memory, addressed by the current instruction pointer.
 * \param memory Reference to system memory.
 * \return A byte from memory.
 */
Byte Cpu6502::get_byte(Memory &memory) const
{
    return memory[m_instruction_pointer];
}

/** \brief Gets a byte from the zero page in memory, from a specified 8-bit address.
 * \param memory Reference to system memory.
 * \param address The address in the zero page from which to fetch data.
 * \return 8-bit value from specified memory location.
 */
Byte Cpu6502::get_byte(Memory &memory, const Byte address) const
{
    return memory[address];
}

/** \brief Gets a byte from system memory.
 * \param memory Reference to system memory.
 * \param address A full 16-bit address from which to getch data.
 * \return 8-bit value from memory.
 */
Byte Cpu6502::get_byte(Memory &memory, const Word address) const
{
    return memory[address];
}

/** \brief Gets a full word from memory, addressed by the current instruction pointer.
 * \param memory Reference to system memory.
 * \return 16-bit value from memory.
 */
Word Cpu6502::get_word(Memory &memory) const
{
    Word val1 = static_cast<Word>(memory[m_instruction_pointer]);
    Word val2 = static_cast<Word>(memory[m_instruction_pointer + 1]);
    return static_cast<Word>(val2 << 8) | val1;
}

/** \brief Gets a full word from the zero page in memory.
 * \param memory Reference to system memory.
 * \param address 8-bit address in memory, from which to fetch data.
 * \return 16-bit value from specified memory location.
 */
Word Cpu6502::get_word(Memory &memory, const Byte address) const
{
    Word val1 = static_cast<Word>(memory[address]);
    Word val2 = static_cast<Word>(memory[address + 1]);
    return static_cast<Word>(val2 << 8) | val1;
}

/** \brief Gets a full word from anywhere in memory.
 * \param memory Reference to system memory.
 * \param address 16-bit address from which to fetch data.
 * \return 16-bit value from memory.
 */
Word Cpu6502::get_word(Memory &memory, const Word address) const
{
    Word val1 = static_cast<Word>(memory[address]);
    Word val2 = static_cast<Word>(memory[address + 1]);
    return static_cast<Word>(val2 << 8) | val1;
}

/** \brief Get data byte from memory using absolute addressing, with data addressed by current instruction pointer.
 * \param memory Reference to system memory.
 * \return 8-bit value from memory.
 */
Byte Cpu6502::get_data_absolute(Memory &memory) const
{
    // get address from next two bytes and add index
    Word address = get_word(memory);
    // return data at address
    return get_byte(memory, address);
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in the 16-bit address at the current instruction pointer.
 */
void Cpu6502::set_data_absolute(Memory &memory, const Byte data)
{
    Word address = get_word(memory);
    memory[address] = data;
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in the 16-bit address at the current instruction pointer.
 * \param index An offset from the specified memory location.
 */
void Cpu6502::set_data_absolute(Memory &memory, Byte data, Byte index)
{
    Word address = get_word(memory) + index;
    memory[address] = data;
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in the 8-bit (zero page) address at the current instruction pointer.
 */
void Cpu6502::set_data_zeropage(Memory &memory, Byte data)
{
    Byte data_address = get_byte(memory);
    memory[data_address] = data;
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in the 8-bit (zero page) address at the current instruction pointer.
 * \param index Offset from the memory location read by the instruction pointer.
 */
void Cpu6502::set_data_zeropage(Memory &memory, Byte data, Byte index)
{
    Byte data_address = get_byte(memory) + index;
    memory[data_address] = data;
}

/** \brief Get data byte from memory using absolute addressing, with data addressed by current instruction pointer and
 * an index.
 * \param memory Reference to system memory.
 * \param index A byte to add to the address to be read from.
 * \return 8-bit value from memory.
 */
Byte Cpu6502::get_data_absolute(Memory &memory, const Byte index)
{
    // get address from next two bytes and add index
    Word address = get_word(memory);
    Byte page1 = static_cast<Byte>(address >> 8);
    address += index;
    Byte page2 = static_cast<Byte>(address >> 8);

    // Check for page crossing for extra cycle.
    if (page1 != page2)
    {
        this->page_crossed = true;
    }

    // return data at address
    return get_byte(memory, address);
}

/** \brief Performs subtracton of accumulator and data, setting the carry bit as required.
 * \param data A byte of data to be subtracted from the accumulator.
 * \return A byte to be stored in the accumulator.
 */
Byte Cpu6502::sub_with_carry(const Byte data)
{
    return add_with_carry(~data);
}

/** \brief Performs addition of accumulator and data, setting the carry bit as required.
 * \param data A byte of data to be added to the accumulator.
 * \return A byte to be stored in the accumulator.
 */
Byte Cpu6502::add_with_carry(const Byte data)
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
Byte Cpu6502::get_data_relative(Memory &memory) const
{
    return get_data_immediate(memory);
}

/** \brief Fetches a byte using immediate addressing mode.
 * \param memory A reference to a memory array object.
 * \return A Byte from memory.
 */
Byte Cpu6502::get_data_immediate(Memory &memory) const
{
    return get_byte(memory);
}

/** \brief Fetches a byte using zpg addressing mode.
 * \param memory A reference to a memory array object.
 * \return A Byte from memory.
 */
Byte Cpu6502::get_data_zeropage(Memory &memory) const
{
    Byte data_address = get_byte(memory);
    return get_byte(memory, data_address);
}

/** \brief Fetches a byte using zpg addressing mode with an index, typically X or Y register.
 * \param memory A reference to a memory array object.
 * \param index An index into a memory region.
 * \return A Byte from memory.
 */
Byte Cpu6502::get_data_zeropage(Memory &memory, const Byte index) const
{
    Byte data_address = get_byte(memory) + index;
    return get_byte(memory, data_address);
}

/** \brief Get a word from the zero page with full wrapping.
 * \param memory Reference to system memory.
 * \param address The zero page address of the first byte to be read.
 * \return 16-bit value from the zero page.
 *
 * This function gets a word from the zero page in memory. Importantly, the FULL word is guaranteed to come from the
 * zero page. If the low byte is at the end of the zero page, the high byte will come from the start of the zero page.
 */
Word Cpu6502::get_word_zpg_wrap(Memory &memory, const Byte address) const
{
    Word val1 = static_cast<Word>(memory[address % 256]);
    Word val2 = static_cast<Word>(memory[(address + 1) % 256]);
    return static_cast<Word>(val2 << 8) | val1;
}

/** \brief Get data from memory using the (indirect,x) addressing mode.
 * \param memory Reference to system memory.
 * \param index Index added to address.
 * \return 8-bit value from memory.
 */
Byte Cpu6502::get_data_indexed_indirect(Memory &memory, const Byte index) const
{
    // read next byte and add index without carry
    Byte indirect_address = get_byte(memory) + index;

    // get target address from indirect_address data and next on zero page
    Word target_address = get_word_zpg_wrap(memory, indirect_address);

    // get data from target address and return
    return get_byte(memory, target_address);
}

/** \brief Set a value in memory using (indirect, x) addressing.
 * \param memory Reference to system memory.
 * \param data Byte of data to store in memory.
 * \param index Offset of memory location.
 */
void Cpu6502::set_data_indexed_indirect(Memory &memory, Byte data, Byte index)
{
    Byte indirect_address = get_byte(memory) + index;
    Word target_address = get_word_zpg_wrap(memory, indirect_address);
    memory[target_address] = data;
}

/** \brief Get data from memory using the (indirect),y addressing mode. Will set the page_crossed flag if a page is
 * crossed.
 * \param memory Reference to system memory.
 * \param index Index to add to address.
 * \return 8-bit value from memory.
 */
Byte Cpu6502::get_data_indirect_indexed(Memory &memory, const Byte index)
{
    // read next byte and add index without carry
    Byte indirect_address = get_byte(memory);

    // get target address from indirect_address data and next on zero page and add index
    Word target_address = get_word_zpg_wrap(memory, indirect_address);
    Byte page1 = static_cast<Byte>(target_address >> 8);
    target_address += index;
    Byte page2 = static_cast<Byte>(target_address >> 8);

    // Check if page crossed.
    if (page1 != page2)
    {
        page_crossed = true;
    }

    // get data from target address and return
    return get_byte(memory, target_address);
}

/** \brief Sets a value in memory.
 * \param memory Reference to system memory.
 * \param data A byte of data to store in memory.
 * \param index Memory location offset.
 */
void Cpu6502::set_data_indirect_indexed(Memory &memory, const Byte data, const Byte index)
{
    Byte indirect_address = get_byte(memory);
    Word target_address = get_word_zpg_wrap(memory, indirect_address) + index;
    memory[target_address] = data;
}

/** \brief Adds the signed value distance to the IP.
 * \param distance Signed value defining the distance to jump in memory.
 */
void Cpu6502::branch_relative(Byte distance)
{
    if (distance & BIT7)
    {
        // If jump is negative get the two's complement and subtract the result.
        distance = ~distance;
        distance += 1;
        m_instruction_pointer = m_instruction_pointer - distance;
    }
    else
    {
        // If the jump is positive, do the jump.
        m_instruction_pointer = m_instruction_pointer + distance;
    }
}

/** \brief Moves the stack pointer up by one position and returns the value found at that address.
 * \param memory Reference to system memory.
 * \return A byte from the stack.
 */
Byte Cpu6502::pop_from_stack(Memory &memory)
{
    m_stack_pointer++;
    return memory[m_stack_pointer];
}

/** \brief Encode all CPU flags into a single byte.
 * \return 8-bit value containing all CPU flags.
 */
Byte Cpu6502::flags_as_byte() const
{
    return static_cast<Byte>(
        (N << 7) | (V << 6) | (true << 5) | (B << 4) | (D << 3) | (I << 2) | (Z << 1) | (C << 0));
}

/** \brief Increases the number of available CPU cycles.
 * \param cycles_to_add How much to increase the count of available cycles.
 */
void Cpu6502::add_cycles(const int cycles_to_add)
{
    cycles_available += cycles_to_add;
}

/** \brief Reduce the number of available CPU cycles.
 * \param cycles_to_use How much to reduce the count of available cycles.
 */
void Cpu6502::use_cycles(const int cycles_to_use)
{
    cycles_available -= cycles_to_use;
}

/** \brief Set the instruction pointer of the CPU.
 * \param newIP The new value of the instruction pointer.
 */
void Cpu6502::set_instruction_pointer(const Word newIP)
{
    // TODO Bounds checking on IP value.
    this->m_instruction_pointer = newIP;
}

/** \brief Set the stack pointer of the CPU.
 * \param newSP The new value for the stack pointer.
 */
void Cpu6502::set_stack_pointer(const Word newSP)
{
    // TODO Bounds checking on SP value.
    this->m_stack_pointer = newSP;
}

/** \brief Get the current value of the CPU instruction pointer.
 * \return Current IP value.
 */
Word Cpu6502::get_instruction_pointer() const
{
    return this->m_instruction_pointer;
}

/** \brief Get the current value of the CPU stack pointer.
 * \return Current SP value.
 */
Word Cpu6502::get_stack_pointer() const
{
    return this->m_stack_pointer;
}
