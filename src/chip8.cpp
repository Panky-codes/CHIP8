#include "chip8.hpp"

#include <algorithm>

// Mask function to get the first Nibble 0xN000
// example: input is 0x6133, output will be 0x6000
static constexpr inline uint16_t first_nibble(const uint16_t opcode) noexcept{
    return (opcode & 0xF000);
}

// Mask function to get the second Nibble 0x0N00
// example: input is 0x6133, output will be 0x0100
static constexpr inline uint16_t second_nibble(const uint16_t opcode) noexcept{
    return (opcode & 0x0F00);
}

// Mask function to get the last two nibbles 0x00NN
// example: input is 0x6133, output will be 0x0100
static constexpr inline uint8_t last_two_nibbles(const uint16_t opcode) noexcept{
    return (opcode & 0x00FF);
}

void chip8::set_debug_level(spdlog::level::level_enum debug_level)
{
    logger->set_level(debug_level);
    logger->trace("All set to log!!");
}

void chip8::load_memory(std::vector<uint8_t> rom_opcodes) {
    std::copy_n(rom_opcodes.begin(), rom_opcodes.size(),
                memory.begin() + prog_mem_begin);
}

std::array<uint8_t, 16> chip8::get_Vx_registers() const
{
    return Vx;
}

void chip8::step_one_cycle() {
    // The memory is read in big endian, i.e., MSB first
    uint16_t opcode = (static_cast<uint16_t>(memory[prog_counter] << 8)) | (memory[prog_counter + 1]);

    switch (first_nibble(opcode)) {
        // OPCODE 6XNN: Store number NN in register VX
        case (0x6000):
            const unsigned int reg = (second_nibble(opcode) >> 8);
            Vx[reg] = last_two_nibbles(opcode);
            logger->trace("V{0} = {1:#x}",reg, Vx[reg]);
    }
}

