#include <thread>

#include "system.hpp"

System::System()
{
    memory.data = {0};
}

inline void System::clock_function(Semaphore* cpu_sem, unsigned int cycles)
{
    while (cycles--)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{150});
        cpu_sem->notify();
    }
}

void System::load_example_prog(unsigned int which)
{
    switch (which % 4)
    {
        case 0:
            /*  Loads values into A and stores them elsewhere in memory. */
            memory.data = {
                0xa9, 0x01,             // LDA #$01
                0x8d, 0x00, 0x02,       // STA #0200
                0xa9, 0x05,             // LDA #$05
                0x8d, 0x01, 0x02,       // STA #0201
                0xa9, 0x08,             // LDA #$08
                0x8d, 0x02, 0x02,       // STA $0202
                0x00                    // BRK
            };
            break;

        case 1:
            /*  Loads a value into A, copies it into X, increments X, the adds
            the two values. */
            memory.data = {
                0xa9, 0xc0,             // LDA #$C0
                0xaa,                   // TAX
                0xe8,                   // INX
                0x69, 0xc4,             // ADC #$C4
                0x00                    // BRK
            };
            break;

        case 2:
            /*  Loads the value 8 into X. Decrements X in a loop until X = 3, 
            then stores the final value of X in address 0x0201. */
            memory.data = {
                0xa2, 0x08,             // LDX #$08
                                        // decrement:
                0xca,                   // DEX
                0x8e, 0x00, 0x02,       // STX $0200
                0xe0, 0x03,             // CPX #$03
                0xd0, 0xf8,             // BNE decrement (relative jump -6 addresses)
                0x8e, 0x01, 0x02,       // STX $0201
                0x00                    // BRK
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
                0xa2, 0x01,             // LDX #$01
                0xa9, 0x05,             // LDA #$05
                0x85, 0x01,             // STA $01
                0xa9, 0x07,             // LDA #$07
                0x85, 0x02,             // STA $02
                0xa0, 0x0a,             // LDY #$0a
                0x8c, 0x05, 0x07,       // STY $0705
                0xa1, 0x00,             // LDA ($00, X) 
                0x00
            };
    }
}

void System::run()
{
    std::thread clock_thread{clock_function, &cpu.sem, 100};
    cpu.run(memory);
    clock_thread.join();
}
