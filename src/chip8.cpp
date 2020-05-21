#include "chip8.hpp"

#include <algorithm>

#include "fmt/format.h"

// Mask function to get the first Nibble 0xN000
// example: input is 0x6133, output will be 0x6000
static constexpr inline uint16_t first_nibble(const uint16_t opcode) noexcept {
    return (opcode & 0xF000U);
}

// Mask function to get the second Nibble 0x0N00
// example: input is 0x6133, output will be 0x0100
static constexpr inline uint16_t second_nibble(const uint16_t opcode) noexcept {
    return (opcode & 0x0F00U);
}

// Mask function to get the third Nibble 0x00N0
// example: input is 0x6133, output will be 0x0030
static constexpr inline uint8_t third_nibble(const uint16_t opcode) noexcept {
    return (opcode & 0x00F0U);
}

// Mask function to get the last Nibble 0x000N
// example: input is 0x6133, output will be 0x0003
static constexpr inline uint8_t last_nibble(const uint16_t opcode) noexcept {
    return (opcode & 0x000FU);
}
// Mask function to get the last two nibbles 0x00NN
// example: input is 0x6133, output will be 0x0100
static constexpr inline uint8_t last_two_nibbles(
    const uint16_t opcode) noexcept {
    return (opcode & 0x00FFU);
}

// Mask function to get the last two nibbles 0x00NN
// example: input is 0x6133, output will be 0x0100
static constexpr inline std::pair<uint8_t,uint8_t> get_XY_nibbles(
    const uint16_t opcode) noexcept {
    return  {(second_nibble(opcode) >> 8),(third_nibble(opcode) >> 4) };
}

void chip8::load_memory(std::vector<uint8_t> rom_opcodes) {
    std::copy_n(rom_opcodes.begin(), rom_opcodes.size(),
                memory.begin() + prog_mem_begin);
}

std::array<uint8_t, 16> chip8::get_V_registers() const { return V; }

uint16_t chip8::get_prog_counter() const { return prog_counter; }

void chip8::step_one_cycle() {
    // The memory is read in big endian, i.e., MSB first
    uint16_t opcode = static_cast<uint16_t>((memory[prog_counter] << 8) |
                                            (memory[prog_counter + 1U]));

    switch (first_nibble(opcode)) {
        // OPCODE 6XNN: Store number NN in register VX
        case (0x6000): {
            const uint8_t Vx =
                static_cast<uint8_t>(second_nibble(opcode) >> 8);
            V[Vx] = last_two_nibbles(opcode);
            break;
        }
        case (0x8000): {
            // OPCODE 8XY0 : Store the value of register VY in register VX
            if (last_nibble(opcode) == 0) {
                const auto [Vx, Vy] = get_XY_nibbles(opcode);
                V[Vx] = V[Vy];
            }
            // OPCODE 8XY4 : Add the value of register VY to register VX
            // Set VF to 01 if a carry occurs else to 0
            else if(last_nibble(opcode) == 4) 
            {
                const auto [Vx, Vy] = get_XY_nibbles(opcode);
                const uint16_t sum = static_cast<uint16_t>(V[Vy] + V[Vx]);
                //mask the sum with 0b100000000 (0x100) to get the overflow bit
                V[0xF] = static_cast<uint8_t>((sum & 0x100) >> 8);
                V[Vx] = static_cast<uint8_t>(sum);
            }
            // OPCODE 8XY5 : Subtract the value of register VY from register VX 
            // Set VF to 01 if a borrow does not occur, else to 0
            else if(last_nibble(opcode) == 5) 
            {
                const auto [Vx, Vy] = get_XY_nibbles(opcode);
                if (V[Vx] > V[Vy])
                {
                    V[0xF] = 1;
                }
                else
                {
                    V[0xF] = 0;
                }
                V[Vx] = static_cast<uint8_t>(V[Vx] - V[Vy]);
            }
            // OPCODE 8XY7 : Set register VX to the value of VY minus VX
            // Set VF to 01 if a borrow does not occur, else to 0
            else if(last_nibble(opcode) == 7) 
            {
                const auto [Vx, Vy] = get_XY_nibbles(opcode);
                if (V[Vy] > V[Vx])
                {
                    V[0xF] = 1;
                }
                else
                {
                    V[0xF] = 0;
                }
                V[Vx] = static_cast<uint8_t>(V[Vy] - V[Vx]);
            }
            // OPCODE 8XY2 : Set VX to VX AND VY 
            else if(last_nibble(opcode) == 2) 
            {
                const auto [Vx, Vy] = get_XY_nibbles(opcode);
                V[Vx] = V[Vx] & V[Vy];
            }
            // OPCODE 8XY1 : Set VX to VX OR VY 
            else if(last_nibble(opcode) == 1) 
            {
                const auto [Vx, Vy] = get_XY_nibbles(opcode);
                V[Vx] = V[Vx] | V[Vy];
            }
            // OPCODE 8XY3 : Set VX to VX XOR VY 
            else if(last_nibble(opcode) == 3) 
            {
                const auto [Vx, Vy] = get_XY_nibbles(opcode);
                V[Vx] = V[Vx] ^ V[Vy];
            }
            break;
        }
        // OPCODE 7XNN : Add NN to register VX
        case (0x7000): {
            const uint8_t Vx =
                static_cast<uint8_t>(second_nibble(opcode) >> 8);
            // static_cast replicates the actual CHIP8 adder where if a 8bit
            // overflow happens the addition resets to 0 once the value crosses
            // 255
            V[Vx] =
                static_cast<uint8_t>((last_two_nibbles(opcode) + V[Vx]));
            break;
        }
    }
    // Each cycle reads two consecutive opcodes
    ++prog_counter;
    ++prog_counter;
}

