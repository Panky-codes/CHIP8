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
        auto actual_V = emulator.get_V_registers();
        auto final_pc = emulator.get_prog_counter();

        REQUIRE(actual_V[1] == 0x32);
        REQUIRE((final_pc - initial_pc) == 2);
    }
    SECTION("8XY0") {
        //81 30 -> Store the value of V3 in V1
        std::vector<uint8_t> rom{0x63, 0x32, 0x81, 0x30};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[3] == 0x32);
        REQUIRE(actual_V[1] == 0x32);
    }
    SECTION("7XNN") {
        //71 33 -> Add the value NN to register V1
        std::vector<uint8_t> rom{0x63, 0x32, 0x73, 0x30};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[3] == (0x32 + 0x30));
    }
    SECTION("7XNN 8bit overflow") {
        //71 33 -> Add the value NN to register V1
        std::vector<uint8_t> rom{0x63, 0x32, 0x73, 0xF0};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[3] == (0x22));
    }
    SECTION("8XY4") {
        //81 34 -> Add the value V3 to register V1 and set VF if it overflows
        std::vector<uint8_t> rom{0x61, 0x32, 0x63, 0x36, 0x81, 0x34};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[1] == (0x32 + 0x36));
        REQUIRE(actual_V[0xF] == 0);
    }
    SECTION("8XY4 8bit overflow and VF should be 1") {
        //81 34 -> Add the value V3 to register V1 and set VF to 1 as it overflows
        std::vector<uint8_t> rom{0x61, 0x32, 0x63, 0xF1, 0x81, 0x34};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[1] == 0x23);
        REQUIRE(actual_V[0xF] == 1);
    }
}

