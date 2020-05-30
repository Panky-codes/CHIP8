#include "chip8.hpp"
#include "fmt/format.h"
#include <random>

struct BCD_t {
  uint8_t MSB;
  uint8_t MidB;
  uint8_t LSB;
};

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

static constexpr BCD_t parse_BCD(const uint8_t number) {

  BCD_t BCD{0, 0, 0};
  BCD.LSB = number % 10;
  BCD.MidB = static_cast<uint8_t>(((number % 100) - BCD.LSB) / 10);
  BCD.MSB = static_cast<uint8_t>((number - BCD.MidB) / 100);
  return BCD;
}

static constexpr std::array<uint8_t, 80> chip8_fonts = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
chip8::chip8() {
  std::copy_n(chip8_fonts.begin(), chip8_fonts.size(), memory.begin());
}

void chip8::load_memory(std::vector<uint8_t> rom_opcodes) {
  std::copy_n(rom_opcodes.begin(), rom_opcodes.size(),
              memory.begin() + prog_mem_begin);
}

std::array<uint8_t, 16> chip8::get_V_registers() const { return V; }
std::array<uint8_t, 16> chip8::get_Keys_array() const { return Keys; }
std::array<uint8_t, 4096> chip8::get_memory_dump() const { return memory; }
std::stack<uint16_t> chip8::get_stack() const { return hw_stack; }

uint16_t chip8::get_prog_counter() const { return prog_counter; }
uint8_t chip8::get_delay_counter() const { return delay_timer; }
uint8_t chip8::get_sound_counter() const { return sound_timer; }
uint16_t chip8::get_I_register() const { return I; }
std::array<uint8_t, display_size> chip8::get_display() const { return display; }

void chip8::step_one_cycle() {
  // The memory is read in big endian, i.e., MSB first
  auto opcode = static_cast<uint16_t>((memory[prog_counter] << 8) |
                                      (memory[prog_counter + 1U]));
  // Each cycle reads two consecutive opcodes
  // -Wconversion requires this cast as 2 will be implicitly
  // turned to an int
  prog_counter = static_cast<uint16_t>(prog_counter + 2);

  if (delay_timer > 0) {
    --delay_timer;
  }
  if (sound_timer > 0) {
    // Use SFML to BEEP
    --sound_timer;
  }
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
      const auto sum = static_cast<uint16_t>(V[Vy] + V[Vx]);
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
    prog_counter =
        static_cast<uint16_t>(last_three_nibbles(opcode) + V[0]) & 0x0FFF;
    ;
    break;
  }
  // OPCODE 2NNN : Execute subroutine starting at address NNN
  case (0x2000): {
    hw_stack.push(prog_counter);
    prog_counter = last_three_nibbles(opcode) & 0x0FFF;
    break;
  }
  case (0x0000): {
    // OPCODE 00EE : Return from a subroutine
    if (last_two_nibbles(opcode) == 0xEE) {
      prog_counter = hw_stack.top();
      hw_stack.pop();
    } else if (last_two_nibbles(opcode) == 0xE0) {
      display = {0};
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
      prog_counter = static_cast<uint16_t>(prog_counter + 2) & 0x0FFF;
    }
    break;
  }
  // OPCODE 4XNN : Skip the following instruction
  // if the value of register VX not equal to NN
  case (0x4000): {
    const auto Vx = static_cast<uint8_t>(second_nibble(opcode) >> 8);
    const uint8_t cmp_value = last_two_nibbles(opcode);
    if (V[Vx] != cmp_value) {
      prog_counter = static_cast<uint16_t>(prog_counter + 2) & 0x0FFF;
    }
    break;
  }
  // OPCODE 5XNN : Skip the following instruction if the value
  // of register VX is equal to the value of register VY
  case (0x5000): {
    const auto [Vx, Vy] = get_XY_nibbles(opcode);
    if (V[Vx] == V[Vy]) {
      prog_counter = static_cast<uint16_t>(prog_counter + 2) & 0x0FFF;
    }
    break;
  }
  // OPCODE 9XNN : Skip the following instruction if the value
  // of register VX is not equal to the value of register VY
  case (0x9000): {
    const auto [Vx, Vy] = get_XY_nibbles(opcode);
    if (V[Vx] != V[Vy]) {
      prog_counter = static_cast<uint16_t>(prog_counter + 2) & 0x0FFF;
    }
    break;
  }
  case (0xF000): {
    // OPCODE FX15:	Set the delay timer to the value of register VX
    if (last_two_nibbles(opcode) == 0x15) {
      const auto Vx = static_cast<uint8_t>(second_nibble(opcode) >> 8);
      delay_timer = V[Vx];
    }
    // OPCODE FX07: Store the current value of the delay timer in register VX
    else if (last_two_nibbles(opcode) == 0x07) {
      const auto Vx = static_cast<uint8_t>(second_nibble(opcode) >> 8);
      V[Vx] = delay_timer;
    }
    // OPCODE FX18: Set the sound timer to the value of register VX
    else if (last_two_nibbles(opcode) == 0x18) {
      const auto Vx = static_cast<uint8_t>(second_nibble(opcode) >> 8);
      sound_timer = V[Vx];
    }
    // OPCODE FX29: Set I to the memory address of the sprite data
    // corresponding to the hexadecimal digit stored in register VX
    else if (last_two_nibbles(opcode) == 0x29) {
      I = static_cast<uint16_t>(5 * (second_nibble(opcode) >> 8));
    }
    // OPCODE FX33: Store the binary-coded decimal equivalent of
    // the value stored in register VX at addresses I, I+1, and I+2
    else if (last_two_nibbles(opcode) == 0x33) {
      const auto Vx = static_cast<uint8_t>((second_nibble(opcode) >> 8));
      auto [MSB, MidB, LSB] = parse_BCD(V[Vx]);
      memory[I] = MSB;
      memory[I + 1] = MidB;
      memory[I + 2] = LSB;
    }
    // OPCODE FX55: Store the values of registers V0 to VX
    // inclusive in memory starting at address I
    // I is set to I + X + 1 after operation
    else if (last_two_nibbles(opcode) == 0x55) {
      const auto Vx = static_cast<uint8_t>((second_nibble(opcode) >> 8));
      for (size_t i = 0; i <= Vx; i++) {
        memory[I + i] = V[i];
      }
      I = static_cast<uint16_t>(I + Vx + 1);
    }
    // OPCODE FX65: Fill registers V0 to VX
    // inclusive with the values stored in memory starting at address I
    // I is set to I + X + 1 after operation
    else if (last_two_nibbles(opcode) == 0x65) {
      const auto Vx = static_cast<uint8_t>((second_nibble(opcode) >> 8));
      for (size_t i = 0; i <= Vx; i++) {
        V[i] = memory[I + i];
      }
      I = static_cast<uint16_t>(I + Vx + 1);
    }
    // OPCODE FX1E: Add the value stored in register VX to register I
    else if (last_two_nibbles(opcode) == 0x1E) {
      const auto Vx = static_cast<uint8_t>(second_nibble(opcode) >> 8);
      I = static_cast<uint16_t>(I + V[Vx]);
    } else {
      fmt::print("Unrecognized opcode: {0:#x} \n", opcode);
    }
    break;
  }
  // OPCODE ANNN: Store memory address NNN in register I
  case (0xA000): {
    I = last_three_nibbles(opcode);
    break;
  }
  // OPCODE DXYN: Draw a sprite at position VX, VY with N bytes
  // of sprite data starting at the address stored in I
  // Set VF to 01 if any set pixels are changed to unset, and 00 otherwise
  // TODO: Too many cast required. Check why
  case (0xD000): {
    const auto [Vx, Vy] = get_XY_nibbles(opcode);
    const auto N = last_nibble(opcode);

    for (uint16_t y = 0; y < N; y++) {
      auto pos = static_cast<uint16_t>(V[Vx] + (display_x * (y + V[Vy])));
      uint8_t sprite = memory.at(static_cast<uint16_t>(I + y));

      for (uint16_t x = 0; x < 8; x++) {
        auto actual_pos = static_cast<uint16_t>(pos + x);
        if ((sprite >> x)) {
          V[0xF] = 0;
          if (!(display[actual_pos] ^ (sprite >> x))) {
            V[0xF] = 1;
          }
          display[actual_pos] =
              static_cast<uint8_t>(display[actual_pos] ^ sprite >> x);
        }
      }
    }
    break;
  }
  }
}
