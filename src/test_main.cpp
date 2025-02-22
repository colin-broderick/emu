#include <gtest/gtest.h>

#include "rewrite.hpp"

TEST(Bus, testRom0)
{
    Bus::load_rom("../test/test0.bin");

    /* The first tick of this program loads the value of memory[1] into A. This instruction should take two cycles. */
    EXPECT_EQ(Cpu::tick(2), ReturnCode::CONTINUE);
    EXPECT_EQ(Cpu::A, Memory::main_memory[1]);
    EXPECT_EQ(Cpu::cycles_available, 0);

    /* The second instruction stores A at the address pointed to by the next data byte, 0x0002, taking four cycles. */
    EXPECT_EQ(Cpu::tick(4), ReturnCode::CONTINUE);
    EXPECT_EQ(Cpu::cycles_available, 0);
    EXPECT_EQ(Cpu::A, Memory::main_memory[0x0200]);

    /* The third instruction is another LDA immediate, loading 5 into A. */
    EXPECT_EQ(Cpu::tick(2), ReturnCode::CONTINUE);
    EXPECT_EQ(Cpu::cycles_available, 0);
    EXPECT_EQ(Cpu::A, 5);

    /* The fourth instruction is another STA absolute, storing A (5) in 0x0201. */
    EXPECT_EQ(Cpu::tick(4), ReturnCode::CONTINUE);
    EXPECT_EQ(Cpu::cycles_available, 0);
    EXPECT_EQ(Cpu::A, Memory::main_memory[0x0201]);

    /* Next is LDA with value 8 */
    EXPECT_EQ(Cpu::tick(2), ReturnCode::CONTINUE);
    EXPECT_EQ(Cpu::cycles_available, 0);
    EXPECT_EQ(Cpu::A, 8);

    /* Next store A (8) in 0x0202 */
    EXPECT_EQ(Cpu::tick(4), ReturnCode::CONTINUE);
    EXPECT_EQ(Cpu::cycles_available, 0);
    EXPECT_EQ(Cpu::A, Memory::main_memory[0x0202]);
    EXPECT_EQ(Cpu::B, 0);

    /* The next and final instruction is a BREAK, which should take 7 cycles. The break flag should be set. */
    EXPECT_EQ(Cpu::tick(7), ReturnCode::BREAK);
    EXPECT_EQ(Cpu::cycles_available, 0);
    EXPECT_EQ(Cpu::B, 1);
}

int main(int argc, char **argv)
{
    std::cout.rdbuf(nullptr);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
