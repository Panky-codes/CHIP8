#define CATCH_CONFIG_MAIN
#include <algorithm>

#include "catch2/catch.hpp"
#include "chip8.hpp"

TEST_CASE("Opcode verification") {
    chip8 emulator;
    SECTION("6XNN") {
        std::vector<uint8_t> rom{0x61, 0x32};

        emulator.load_memory(rom);
        auto initial_pc = emulator.get_prog_counter();
        emulator.step_one_cycle();
        auto actual_Vx = emulator.get_Vx_registers();
        auto final_pc = emulator.get_prog_counter();

        REQUIRE(actual_Vx[1] == 0x32);
        REQUIRE((final_pc - initial_pc) == 2);
    }
    SECTION("8XY0") {
        //81 30 -> Store the value of V3 in V1
        std::vector<uint8_t> rom{0x63, 0x32, 0x81, 0x30};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_Vx = emulator.get_Vx_registers();

        REQUIRE(actual_Vx[3] == 0x32);
        REQUIRE(actual_Vx[1] == 0x32);
    }
}

