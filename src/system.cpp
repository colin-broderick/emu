#include <thread>

#include "system.hpp"

System::System()
{
    memory.data = {0};
}

inline void System::clock_function(Semaphore* cpu_sem, unsigned int cycles)
{
    auto time = std::chrono::system_clock::now();
    std::chrono::nanoseconds interval{150};

    while (cycles--)
    {
        time += interval;
        std::this_thread::sleep_until(time);
        cpu_sem->notify();
    }
}

void System::load_example_prog(unsigned int which)
{
    switch (which % 6)
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

        case 5:
            /*  This program increments Y and X until Y = 0x10, then continues to
            increment Y only until Y = 0x20. In the first stage, the value of X is
            pushed onto the stack on every iteration. In the second stage, the
            value is pulled from the stack into A at every iteration.
                Throughout, the value of X (or A) is put into memory creating a 
            mirrored pattern. Something like this could be used to draw pixels to
            a display buffer. */
            memory.data = {
                0xa2, 0x00,             // LDX #$00
                0xa0, 0x00,             // LDY #$00
                                        // firstloop:
                0x8a,                   // TXA
                0x99, 0x00, 0x02,       // STA $0200, Y
                0x48,                   // PHA
                0xe8,                   // INX
                0xc8,                   // INY
                0xc0, 0x10,             // CPY #$10
                0xd0, 0xf5,             // BNE firstloop
                                        // secondloop:
                0x68,                   // PLA
                0x99, 0x00, 0x02,       // STA $0200, Y
                0xc8,                   // INY
                0xc0, 0x20,             // CPY #$20
                0xd0, 0xf7,             // BNE secondloop
                0x00                    // BRK
            };
    }
}

void System::run()
{
    std::thread clock_thread{clock_function, &cpu.sem, 100000000};
    cpu.run(memory);
    clock_thread.join();
}
