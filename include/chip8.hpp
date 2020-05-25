#ifndef CHIP_8_H_
#define CHIP_8_H_

#include <array>
#include <cstdint>
#include <stack>
#include <vector>

class chip8 {
public:
  chip8();
  void load_memory(std::vector<uint8_t> rom_opcodes);
  void reset();
  void step_one_cycle();
  [[nodiscard]] std::array<uint8_t, 16> get_V_registers() const;
  [[nodiscard]] uint16_t get_prog_counter() const;
  [[nodiscard]] uint8_t get_delay_counter() const;
  [[nodiscard]] uint8_t get_sound_counter() const;
  [[nodiscard]] std::stack<uint16_t> get_stack() const;

private:
  std::array<uint8_t, 4096> memory{0};
  std::array<uint8_t, 16> V{0};
  /* std::array<uint8_t, 16> stack{0}; */
  std::stack<uint16_t> hw_stack;
  std::array<uint8_t, (64 * 32)> graphics{0};
  uint8_t stack_pointer{0};
  uint16_t I{0};
  static constexpr int prog_mem_begin = 512;
  uint16_t prog_counter{prog_mem_begin};
  uint8_t delay_timer{0};
  uint8_t sound_timer{0};
};

#endif
