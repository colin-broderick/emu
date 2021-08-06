#ifndef PPU_H
#define PPU_H

#include "memory.hpp"
#include "cpu.hpp"
#include "utils.hpp"

/** \brief The PPU class represents the NES Picture Processing Unit. It is responsible for rendering the game scene
 * graphics into name tables.
 */
class PPU
{
    private:
        // PPU register classes --------------------------------------------------------------------
        union PPUCTRL
        {
            Byte value : 8;
            struct
            {
                Byte V : 1;  // NMI_enable
                Byte P : 1;  // PPU master/slave 
                Byte H : 1;  // Sprite height
                Byte B : 1;  // Background tile select
                Byte S : 1;  // Sprite tile select
                Byte I : 1;  // Increment mode
                Byte NN : 2; // Nametable select
            };
        };
        union PPUSTATUS
        {
            Byte value;
            struct
            {
                Byte V : 1;  // vblank
                Byte S : 1;  // sprite 0 hit
                Byte O : 1;  // sprite overflow
            };
        };
        union PPUMASK
        {
            Byte value;
            struct
            {
                Byte B : 1;  // Emphasize blue
                Byte G : 1;  // Emphasize green
                Byte R : 1;  // Emphasize red
                Byte s : 1;  // Sprite enable
                Byte b : 1;  // Background enable
                Byte M : 1;  // Sprite left column enable
                Byte m : 1;  // Background left column enable
                Byte g : 1;  // Greyscale enable
            };
        };

    public:
        // Constructors ----------------------------------------------------------------------------
        PPU();

        // General methods -------------------------------------------------------------------------
        int run(Memory& memory, const int cycles);

        enum RETURN_CODE
        {
            BREAK,
            CONTINUE
        };

        // Operators -------------------------------------------------------------------------------
        friend std::ostream& operator<<(std::ostream& stream, const PPU& ppu);

        // Public attributes -----------------------------------------------------------------------
        PPUCTRL ppuctrl;
        PPUMASK ppumask;
        PPUSTATUS ppustatus;

    private:
        // Important addresses ---------------------------------------------------------------------
        const static Word pattern_table_0_address = 0x0000;
        const static Word pattern_table_1_address = 0x1000;
        const static Word name_table_0_address    = 0x2000;
        const static Word attrib_table_0_address  = 0x23C0;
        const static Word name_table_1_address    = 0x2400;
        const static Word attrib_table_1_address  = 0x27C0;
        const static Word name_table_2_address    = 0x2800;
        const static Word attrib_table_2_address  = 0x2BC0;
        const static Word name_table_3_address    = 0x2C00;
        const static Word attrib_table_3_address  = 0x2FC0;
        const static Word image_palette_address   = 0x3F00;

        const static Word ppuctrl_address         = 0x2000;
        const static Word ppumask_address         = 0x2001;
        const static Word ppustatus_address       = 0x2002;

        // Private attributes ----------------------------------------------------------------------
        int cycles_available = 0;

        // Private methods -------------------------------------------------------------------------
        void use_cycles(const int cycles_to_use);
        void add_cycles(const int cycles_to_add);
        void interrupt_cpu(CPU& cpu);
        void update_registers(Memory& memory);
};

#endif
