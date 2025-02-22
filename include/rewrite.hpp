#include <array>

/** CPU return codes. The CPU will generally run until it exhausts the supply of cycles, but under
 * certain conditions will return one of these codes. */
enum class ReturnCode
{
    /** Instructs the CPU to stop. */
    BREAK,
    /** Instructs the CPU to continue. */
    CONTINUE
};

namespace Memory
{
    inline std::array<uint8_t, 256 * 256> main_memory{0};
}

namespace Bus
{
    bool load_rom(const std::string &filename);
    void run();
}

namespace Cpu
{
    inline bool C, Z, I, D, B, V, N; // CPU flags.

    inline int cycles_available = 0;
    inline uint16_t stack_pointer = 0;
    inline uint16_t instruction_pointer = 0;
    inline uint8_t A;
    inline uint8_t X;
    inline uint8_t Y; // Accumulator and registers.
}

namespace Cpu
{
    constexpr static int CPU_frequency = 1790000; // Hz
    constexpr static int frame_rate = 60;
    constexpr static int cycles_per_frame = CPU_frequency / frame_rate;
    constexpr static int microseconds_per_frame = 1000000 / frame_rate;
}

namespace Cpu
{
    uint16_t get_word(uint16_t address);
    uint16_t get_word_zpg_wrap(const uint8_t address);
    void LDX_set_CPU_flags();
    uint8_t add_with_carry(const uint8_t data);
    ReturnCode tick(const int cycles_to_add);
}