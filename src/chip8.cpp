#include "chip8.hpp"

#include <random>

#include "fmt/format.h"

// Mask function to get the first Nibble 0xN000
// example: input is 0x6133, output will be 0x6000
static constexpr uint16_t first_nibble(const uint16_t opcode) noexcept {
  return (opcode & 0xF000U);
}

// Mask function to get the second Nibble 0x0N00
// example: input is 0x6133, output will be 0x0100
static constexpr uint16_t second_nibble(const uint16_t opcode) noexcept {
  return (opcode & 0x0F00U);
}

// Mask function to get the third Nibble 0x00N0
// example: input is 0x6133, output will be 0x0030
static constexpr uint8_t third_nibble(const uint16_t opcode) noexcept {
  return (opcode & 0x00F0U);
}

// Mask function to get the last Nibble 0x000N
// example: input is 0x6133, output will be 0x0003
static constexpr uint8_t last_nibble(const uint16_t opcode) noexcept {
  return (opcode & 0x000FU);
}
// Mask function to get the last two nibbles 0x00NN
// example: input is 0x6133, output will be 0x0033
static constexpr uint8_t last_two_nibbles(const uint16_t opcode) noexcept {
  return (opcode & 0x00FFU);
}

// Mask function to get the last three nibbles 0x0NNN
// example: input is 0x6133, output will be 0x0133
static constexpr uint16_t last_three_nibbles(const uint16_t opcode) noexcept {
  return (opcode & 0x0FFFU);
}
// Mask function to get the last two nibbles 0x00NN
// example: input is 0x6133, output will be 0x0100
static constexpr std::pair<uint8_t, uint8_t>
get_XY_nibbles(const uint16_t opcode) noexcept {
  return {(second_nibble(opcode) >> 8), (third_nibble(opcode) >> 4)};
}

void chip8::load_memory(std::vector<uint8_t> rom_opcodes) {
  std::copy_n(rom_opcodes.begin(), rom_opcodes.size(),
              memory.begin() + prog_mem_begin);
}

std::array<uint8_t, 16> chip8::get_V_registers() const { return V; }
std::stack<uint16_t> chip8::get_stack() const { return hw_stack; }

uint16_t chip8::get_prog_counter() const { return prog_counter; }

void chip8::step_one_cycle() {
  // The memory is read in big endian, i.e., MSB first
  auto opcode = static_cast<uint16_t>((memory[prog_counter] << 8) |
                                      (memory[prog_counter + 1U]));
  // Each cycle reads two consecutive opcodes
  // -Wconversion requires this cast as 2 will be implicitly
  // turned to an int
  prog_counter = static_cast<uint16_t>(prog_counter + 2);

  switch (first_nibble(opcode)) {
  // OPCODE 6XNN: Store number NN in register VX
  case (0x6000): {
    const auto Vx = static_cast<uint8_t>(second_nibble(opcode) >> 8);
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
    else if (last_nibble(opcode) == 4) {
      const auto [Vx, Vy] = get_XY_nibbles(opcode);
      const uint16_t sum = static_cast<uint16_t>(V[Vy] + V[Vx]);
      // mask the sum with 0b100000000 (0x100) to get the overflow bit
      V[0xF] = static_cast<uint8_t>((sum & 0x100) >> 8);
      V[Vx] = static_cast<uint8_t>(sum);
    }
    // OPCODE 8XY5 : Subtract the value of register VY from register VX
    // Set VF to 01 if a borrow does not occur, else to 0
    else if (last_nibble(opcode) == 5) {
      const auto [Vx, Vy] = get_XY_nibbles(opcode);
      if (V[Vx] > V[Vy]) {
        V[0xF] = 1;
      } else {
        V[0xF] = 0;
      }
      V[Vx] = static_cast<uint8_t>(V[Vx] - V[Vy]);
    }
    // OPCODE 8XY7 : Set register VX to the value of VY minus VX
    // Set VF to 01 if a borrow does not occur, else to 0
    else if (last_nibble(opcode) == 7) {
      const auto [Vx, Vy] = get_XY_nibbles(opcode);
      if (V[Vy] > V[Vx]) {
        V[0xF] = 1;
      } else {
        V[0xF] = 0;
      }
      V[Vx] = static_cast<uint8_t>(V[Vy] - V[Vx]);
    }
    // OPCODE 8XY2 : Set VX to VX AND VY
    else if (last_nibble(opcode) == 2) {
      const auto [Vx, Vy] = get_XY_nibbles(opcode);
      V[Vx] = V[Vx] & V[Vy];
    }
    // OPCODE 8XY1 : Set VX to VX OR VY
    else if (last_nibble(opcode) == 1) {
      const auto [Vx, Vy] = get_XY_nibbles(opcode);
      V[Vx] = V[Vx] | V[Vy];
    }
    // OPCODE 8XY3 : Set VX to VX XOR VY
    else if (last_nibble(opcode) == 3) {
      const auto [Vx, Vy] = get_XY_nibbles(opcode);
      V[Vx] = V[Vx] ^ V[Vy];
    }
    // OPCODE 8XY6 : Store the value of register VY
    // shifted right one bit in register VX
    // Set register VF to the least significant
    // bit prior to the shift
    // Vy is first changed and then it is stored in Vx
    else if (last_nibble(opcode) == 6) {
      const auto [Vx, Vy] = get_XY_nibbles(opcode);
      V[0xF] = V[Vy] & 0x01;
      V[Vy] = static_cast<uint8_t>(V[Vy] >> 1);
      V[Vx] = V[Vy];
    }
    // OPCODE 8XYE : Store the value of register VY
    //  shifted left one bit in register VX
    // Set register VF to the most significant
    // bit prior to the shift
    // Vy is first changed and then it is stored in Vx
    else if (last_nibble(opcode) == 0xE) {
      const auto [Vx, Vy] = get_XY_nibbles(opcode);
      V[0xF] = static_cast<uint8_t>((V[Vy] & 0x80) >> 7);
      V[Vy] = static_cast<uint8_t>(V[Vy] << 1);
      V[Vx] = V[Vy];
    } else {
      // TODO: Probably error handling? Thrown an exception ?
      fmt::print("Unrecognized opcode: {0:#x} \n", opcode);
    }
    break;
  }
  // OPCODE 7XNN : Add NN to register VX
  case (0x7000): {
    const auto Vx = static_cast<uint8_t>(second_nibble(opcode) >> 8);
    // static_cast replicates the actual CHIP8 adder where if a 8bit
    // overflow happens the addition resets to 0 once the value crosses
    // 255
    V[Vx] = static_cast<uint8_t>((last_two_nibbles(opcode) + V[Vx]));
    break;
  }
  // OPCODE CXNN : Set VX to a random number with a mask of NN
  case (0xC000): {
    const auto Vx = static_cast<uint8_t>(second_nibble(opcode) >> 8);
    const uint8_t mask = last_two_nibbles(opcode);
    std::random_device rseed;
    std::mt19937 rgen(rseed()); // mersenne_twister
    std::uniform_int_distribution<int> idist(0, 255);

    V[Vx] = static_cast<uint8_t>(idist(rgen) & mask);
    break;
  }
  // OPCODE 1NNN : Jump to address NNN
  case (0x1000): {
    prog_counter = last_three_nibbles(opcode);
    break;
  }
  // OPCODE BNNN : Jump to address NNN + V0
  case (0xB000): {
    prog_counter = static_cast<uint16_t>(last_three_nibbles(opcode) + V[0]);
    break;
  }
  // OPCODE 2NNN : Execute subroutine starting at address NNN
  case (0x2000): {
    hw_stack.push(prog_counter);
    prog_counter = last_three_nibbles(opcode);
    break;
  }
  case (0x0000): {
    // OPCODE 00EE : Return from a subroutine
    if (last_two_nibbles(opcode) == 0xEE) {
      prog_counter = hw_stack.top();
      hw_stack.pop();
    } else if (last_two_nibbles(opcode) == 0xE0) {
      // TODO: Implement Clear screen
    } else {
      fmt::print("Unrecognized opcode: {0:#x} \n", opcode);
    }
    break;
  }
  // OPCODE 3XNN : Skip the following instruction
  // if the value of register VX equals NN
  case (0x3000): {
    const auto Vx = static_cast<uint8_t>(second_nibble(opcode) >> 8);
    const uint8_t cmp_value = last_two_nibbles(opcode);
    if (V[Vx] == cmp_value) {
      prog_counter = static_cast<uint16_t>(prog_counter + 2);
    }
    break;
  }
  // OPCODE 4XNN : Skip the following instruction
  // if the value of register VX not equal to NN
  case (0x4000): {
    const auto Vx = static_cast<uint8_t>(second_nibble(opcode) >> 8);
    const uint8_t cmp_value = last_two_nibbles(opcode);
    if (V[Vx] != cmp_value) {
      prog_counter = static_cast<uint16_t>(prog_counter + 2);
    }
    break;
  }
  // OPCODE 5XNN : Skip the following instruction if the value 
  // of register VX is equal to the value of register VY 
  case (0x5000): {
    const auto [Vx, Vy] = get_XY_nibbles(opcode);
    if (V[Vx] == V[Vy]) {
      prog_counter = static_cast<uint16_t>(prog_counter + 2);
    }
    break;
  }
  // OPCODE 9XNN : Skip the following instruction if the value 
  // of register VX is not equal to the value of register VY 
  case (0x9000): {
    const auto [Vx, Vy] = get_XY_nibbles(opcode);
    if (V[Vx] != V[Vy]) {
      prog_counter = static_cast<uint16_t>(prog_counter + 2);
    }
    break;
  }
  }
}
