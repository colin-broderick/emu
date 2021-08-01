#include "cpu.hpp"
#include "utils.hpp"

/** \brief Sets appropriate flags after performing LDA operations.
 * 
 * Flags are set based on the current value in the accumulator.
 * The negative (N) flag is set if the value of the accumulator is negative, i.e. if bit 7 is set.
 * The zero (Z) flag is set if the value of the accumulator is zero.
 */
void CPU::LDA_set_CPU_flags()
{
    N = (A & BIT7);
    Z = (A == 0);
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
void CPU::CMP_set_CPU_flags(const Byte data_from_memory)
{
    int difference = static_cast<int>(A - data_from_memory);
    C = (difference >= 0);
    Z = (difference == 0);
    N = (difference & BIT7);
}

/** \brief Sets appropriate CPU flags following an EOR operation.
 * 
 * The negative (N) flag is set if the value of the accumulator is negative, i.e. if bit 7 is set.
 * The zero (Z) flag is set if the value of the accumulator is zero.
*/
void CPU::EOR_set_CPU_flags()
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
void CPU::DEC_set_CPU_flags(const Byte data_from_memory)
{
    N = (data_from_memory & BIT7);  // Set N on if sign bit of result is set.
    Z = (data_from_memory == 0);    // Set Z on if result is zero.
}

/** \brief Sets appropriate CPU flags following an INC operation. INC changes the value
 * in memory, and the N and Z flags are set against the new value.
 * \param data_from_memory The new value of the byte in memory.
 * 
 * The negative (N) flag is set if the value is negative, i.e. if bit 7 is set.
 * The zero (Z) flag is set if the value is equal to zero.
 */
void CPU::INC_set_CPU_flags(const Byte data_from_memory)
{
    DEC_set_CPU_flags(data_from_memory);
}

/** \brief Sets appropriuate flags after performing ORA operation.
 * 
 * The negative (N) flag is set if the value of the accumulator is negative, i.e. if bit 7 is set.
 * The zero (Z) flag is set if the value of the accumulator is zero.
*/
void CPU::ORA_set_CPU_flags()
{
    LDA_set_CPU_flags();
}

/** \brief Sets appropriate flags after performing LDX operations.
 * 
 * Flags are set based on the current value of the X register.
 * The negative (N) flag is set if X is negative, i.e. bit 7 is set.
 * The zero (Z) flag is set if X is equal to zero.
*/
void CPU::LDX_set_CPU_flags()
{
    N = (X & BIT7);
    Z = (X == 0);
}

/** \brief Sets appropriate flags after performing LDY operations.
 * 
 * Flags are set based on the current value of the Y register.
 * The negative (N) flag is set if Y is negative, i.e. bit 7 is set.
 * The zero (Z) flag is set if Y is equal to zero.
*/
void CPU::LDY_set_CPU_flags()
{
    N = (Y & BIT7);
    Z = (Y == 0);
}

/** \brief Sets appropriate flags after performing TAX operation.
 * 
 * Flags are set based on the current value of the X register.
 * The negative (N) flag is set if X is negative, i.e. bit 7 is set.
 * The zero (Z) flag is set if X is equal to zero.
*/
void CPU::TAX_set_CPU_flags()
{
    LDX_set_CPU_flags();
}

/** \brief Sets appropriate flags after performing TXA operation.
 * 
 * Flags are set based on the current value in the accumulator.
 * The negative (N) flag is set if the value of the accumulator is negative, i.e. if bit 7 is set.
 * The zero (Z) flag is set if the value of the accumulator is zero.
*/
void CPU::TXA_set_CPU_flags()
{
    LDA_set_CPU_flags();
}

/** \brief Set CPU flags following DEX operation.
 * 
 * Flags are set based on the current value of the X register.
 * The negative (N) flag is set if X is negative, i.e. bit 7 is set.
 * The zero (Z) flag is set if X is equal to zero.
*/
void CPU::DEX_set_CPU_flags()
{
    LDX_set_CPU_flags();
}

/** \brief Set CPU flags following INX operaiton.
 * 
 * Flags are set based on the current value of the X register.
 * The negative (N) flag is set if X is negative, i.e. bit 7 is set.
 * The zero (Z) flag is set if X is equal to zero.
*/
void CPU::INX_set_CPU_flags()
{
    LDX_set_CPU_flags();
}

/** Set CPU flagas following DEY operation.
 * 
 * 
 * Flags are set based on the current value of the Y register.
 * The negative (N) flag is set if Y is negative, i.e. bit 7 is set.
 * The zero (Z) flag is set if Y is equal to zero.
*/
void CPU::DEY_set_CPU_flags()
{
    LDY_set_CPU_flags();
}

/** \brief Set CPU flags following INY operation.
 * 
 * 
 * Flags are set based on the current value of the Y register.
 * The negative (N) flag is set if Y is negative, i.e. bit 7 is set.
 * The zero (Z) flag is set if Y is equal to zero.
*/
void CPU::INY_set_CPU_flags()
{
    LDY_set_CPU_flags();
}

/** \brief Set CPU flags following CPX operation. 
 * \param data The setting of the flags depends upon some non-stored calculation result.
 * 
 * Flags are set based on the non-stored result of the calculation, which is passed to this function.
 * The carry (C) flag is set if the value is greater than or equal to zero.
 * The zero (Z) flag is set if the value is equal to zero.
 * The negative (N) flag is set if bit 7 of the value is equal to one; this is the sign bit if the value is interpreted as a signed.
 */
void CPU::CPX_set_CPU_flags(const int data)
{
    C = (data >= 0);
    Z = (data == 0);
    N = (data & BIT7);
}

/** \brief Set CPU flags following CPY operation.
 * \param data The setting of the flags depends upon some non-stored calculation result.
 * 
 * Flags are set based on the non-stored result of the calculation, which is passed to this function.
 * The carry (C) flag is set if the value is greater than or equal to zero.
 * The zero (Z) flag is set if the value is equal to zero.
 * The negative (N) flag is set if bit 7 of the value is equal to one; this is the sign bit if the value is interpreted as a signed.
 */
void CPU::CPY_set_CPU_flags(const int data)
{
    CPX_set_CPU_flags(data);
}

/** \brief Set CPU flags following an AND operation.
 * 
 * Flags are set based on the current value in the accumulator.
 * The negative (N) flag is set if the value of the accumulator is negative, i.e. if bit 7 is set.
 * The zero (Z) flag is set if the value of the accumulator is zero.
*/
void CPU::AND_set_CPU_flags()
{
    LDA_set_CPU_flags();
}
