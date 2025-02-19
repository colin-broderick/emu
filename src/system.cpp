#include <thread>
#include <algorithm>
#include <cstring>

#include "system.hpp"

constexpr uint32_t ROM_BUFFER_SIZE = 0xFFFF;

/** \brief Constructor for the NES system. */
System::System()
{
}

/** \brief Load a standard ROM file into system memory.
 * \param filename The name of the ROM file.
 * \return bool true if load successful, otherwise bool false. TODO
 */
bool System::load_rom(const std::string &filename)
{
    std::ifstream input_file(filename, std::ios::binary);
    char buf[ROM_BUFFER_SIZE];
    input_file.read(buf, ROM_BUFFER_SIZE);
    Byte *buf2 = (Byte *)buf;
    std::memcpy(memory.data.data(), buf2, ROM_BUFFER_SIZE);
    return true;
}

/** \brief Load an example program for testing purposes.
 * \param which Selector for the program to be loaded.
 */
void System::load_example_prog(const uint32_t which)
{
    switch (which % 11)
    {
    case 0:
        /*  Loads values into A and stores them elsewhere in memory. */
        memory.data = {
            0xa9, 0x01,       // LDA #$01
            0x8d, 0x00, 0x02, // STA #0200
            0xa9, 0x05,       // LDA #$05
            0x8d, 0x01, 0x02, // STA #0201
            0xa9, 0x08,       // LDA #$08
            0x8d, 0x02, 0x02, // STA $0202
            0x00              // BRK
        };
        break;

    case 1:
        /*  Loads a value into A, copies it into X, increments X, the adds
        the two values. */
        memory.data = {
            0xa9, 0xc0, // LDA #$C0
            0xaa,       // TAX
            0xe8,       // INX
            0x69, 0xc4, // ADC #$C4
            0x00        // BRK
        };
        break;

    case 2:
        /*  Loads the value 8 into X. Decrements X in a loop until X = 3,
        then stores the final value of X in address 0x0201. */
        memory.data = {
            0xa2, 0x08,       // LDX #$08
                              // decrement:
            0xca,             // DEX
            0x8e, 0x00, 0x02, // STX $0200
            0xe0, 0x03,       // CPX #$03
            0xd0, 0xf8,       // BNE decrement (relative jump -6 addresses)
            0x8e, 0x01, 0x02, // STX $0201
            0x00              // BRK
        };
        break;

    case 4:
        /*  This one is dodgy because it overwrites itself; this is because
        the example I drew from assumed the IP started at 0x0600, and stores
        data around 0x0000, whereas my program starts at 0x0000 and stores
        data in the same place.
            As it happens, this program is still able to complete
        successfully. But self-altering programs should be avoided! */
        memory.data = {
            0xa2, 0x01,       // LDX #$01
            0xa9, 0x05,       // LDA #$05
            0x85, 0x01,       // STA $01
            0xa9, 0x07,       // LDA #$07
            0x85, 0x02,       // STA $02
            0xa0, 0x0a,       // LDY #$0a
            0x8c, 0x05, 0x07, // STY $0705
            0xa1, 0x00,       // LDA ($00, X)
            0x00};
        break;

    case 5:
        /*  This program increments Y and X until Y = 0x10, then continues to
        increment Y only until Y = 0x20. In the first stage, the value of X is
        pushed onto the stack on every iteration. In the second stage, the
        value is pulled from the stack into A at every iteration.
            Throughout, the value of X (or A) is put into memory creating a
        mirrored pattern. Something like this could be used to draw pixels to
        a display buffer. */
        memory.data = {
            0xa2, 0x00,       // LDX #$00
            0xa0, 0x00,       // LDY #$00
                              // firstloop:
            0x8a,             // TXA
            0x99, 0x00, 0x02, // STA $0200, Y
            0x48,             // PHA
            0xe8,             // INX
            0xc8,             // INY
            0xc0, 0x10,       // CPY #$10
            0xd0, 0xf5,       // BNE firstloop
                              // secondloop:
            0x68,             // PLA
            0x99, 0x00, 0x02, // STA $0200, Y
            0xc8,             // INY
            0xc0, 0x20,       // CPY #$20
            0xd0, 0xf7,       // BNE secondloop
            0x00              // BRK
        };
        break;
    case 6:
        memory.data = {
            0xa2, 0x01,       // LDX #$01
            0xa2, 0x02,       // LDY #$02
            0x20, 0x09, 0x00, // JSR #$08
            0xa2, 0x03,       // LDY #$03
            0xa2, 0x04,       // LDA #$04
            0x60,
            0x00};
        break;
    case 7: // simple test for JMP
        memory.data = {
            0x4c, 0x34, 0x12, // JMP $1234
        };
        break;
    case 8: // simple test for JMP INDIRECT
        memory.data = {
            0x6c, 0x03, 0x00, // JMP $0003
            0xfc, 0xba};
        break;
    case 9: // test for AND operations
        memory.data = {
            0x3d, 0x00 // AND $0000, X
        };
        break;
    case 10: // test for AND with indirect y
        memory.data = {
            0x31, 0xff, // AND ($33, X)
            0x00};
        memory[0x0100] = 0x44;
        memory[0x00fd] = 0x0c;
        memory[0x31cc] = 0xc0;
        memory[0x44cc] = 0x0c;
        memory[0x2103] = 0x0c;
    }
}

/** \brief Run the loaded program until it exits. */
void System::run()
{
    auto time = std::chrono::high_resolution_clock::now();
    auto interval = std::chrono::microseconds{Cpu6502::microseconds_per_frame};

    while (cpu.run(memory, Cpu6502::cycles_per_frame) != Cpu6502::ReturnCode::BREAK)
    {
        time += interval;
        std::this_thread::sleep_until(time);
    }
}
