#define CATCH_CONFIG_MAIN
#include <algorithm>
#include "chip8.hpp"

#include "catch2/catch.hpp"

TEST_CASE("6XNN opcode") {
    chip8 emulator;
    emulator.set_debug_level(spdlog::level::off);
    std::vector<uint8_t> rom {0x61,0x32};
    emulator.load_memory(rom);
    emulator.step_one_cycle();
    auto actual_Vx = emulator.get_Vx_registers();
    REQUIRE(actual_Vx[1] == 0x32);

}

