#include "chip8.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <spdlog/common.h>
#include <spdlog/logger.h>

int main() 
{
    chip8 emulator;
    emulator.set_debug_level(spdlog::level::trace);
    std::vector<uint8_t> rom {0x61,0x32};
    emulator.load_memory(rom);
    emulator.step_one_cycle();

 return 0;
}

                
