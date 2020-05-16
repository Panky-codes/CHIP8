#ifndef CHIP_8_H_
#define CHIP_8_H_

#include <spdlog/common.h>

#include <array>
#include <cstdint>
#include <vector>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

class chip8 {
   public:
    void load_memory(std::vector<uint8_t> rom_opcodes);
    void set_debug_level(spdlog::level::level_enum debug_level);
    void reset();
    void step_one_cycle();
    std::array<uint8_t, 16> get_Vx_registers() const;

   private:
    std::array<uint8_t, 4096> memory{0};
    std::array<uint8_t, 16> Vx{0};
    std::array<uint8_t, 16> stack{0};
    std::array<uint8_t, (64 * 32)> graphics{0};
    uint8_t stack_pointer{0};
    uint16_t I{0};
    static constexpr int prog_mem_begin = 512;
    uint16_t prog_counter{prog_mem_begin};
    uint8_t delay_timer{0};
    uint8_t sound_timer{0};
    std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("console");
};

#endif
