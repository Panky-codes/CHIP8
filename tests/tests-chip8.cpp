#include "catch2/catch.hpp"
#include "chip8.hpp"

TEST_CASE("Opcode verification") {
    chip8 emulator;
    SECTION("6XNN STA NN in Vx") {
        std::vector<uint8_t> rom{0x61, 0x32};

        emulator.load_memory(rom);
        auto initial_pc = emulator.get_prog_counter();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();
        auto final_pc = emulator.get_prog_counter();

        REQUIRE(actual_V[1] == 0x32);
        REQUIRE((final_pc - initial_pc) == 2);
    }
    SECTION("8XY0 STA Vy in Vx") {
        // 81 30 -> Store the value of V3 in V1
        std::vector<uint8_t> rom{0x63, 0x32, 0x81, 0x30};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[3] == 0x32);
        REQUIRE(actual_V[1] == 0x32);
    }
    SECTION("7XNN ADD without 8 bit overflow ") {
        // 71 33 -> Add the value NN to register V1
        std::vector<uint8_t> rom{0x63, 0x32, 0x73, 0x30};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[3] == (0x32 + 0x30));
    }
    SECTION("7XNN ADD with 8bit overflow") {
        // 71 33 -> Add the value NN to register V1
        std::vector<uint8_t> rom{0x63, 0x32, 0x73, 0xF0};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[3] == (0x22));
    }
    SECTION("8XY4 ADD without 8bit overflow") {
        // 81 34 -> Add the value V3 to register V1 and set VF if it overflows
        std::vector<uint8_t> rom{0x61, 0x32, 0x63, 0x36, 0x81, 0x34};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[1] == (0x32 + 0x36));
        REQUIRE(actual_V[0xF] == 0);
    }
    SECTION("8XY4 ADD with 8bit overflow and VF should be 1") {
        // 81 34 -> Add the value V3 to register V1 and set VF to 1 as it
        // overflows
        std::vector<uint8_t> rom{0x61, 0x32, 0x63, 0xF1, 0x81, 0x34};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[1] == 0x23);
        REQUIRE(actual_V[0xF] == 1);
    }
    SECTION("8XY5 SUB without borrow") {
        // 81 35 -> Subtract the value V3 from register V1 and set VF to 1
        std::vector<uint8_t> rom{0x61, 0x32, 0x63, 0x26, 0x81, 0x35};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[1] == 0xC);
        REQUIRE(actual_V[0xF] == 1);
    }
    SECTION("8XY5 SUB with borrow") {
        // 81 35 -> Subtract the value V3 from register V1 and set VF to 0
        std::vector<uint8_t> rom{0x61, 0x32, 0x63, 0x36, 0x81, 0x35};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[0xF] == 0);
        REQUIRE(actual_V[1] == static_cast<uint8_t>(-1 * 0x4));
    }
    SECTION("8XY7 SUB without borrow") {
        // 81 37 -> Subtract the value V1 from register V3 in V1 and set VF to 1
        std::vector<uint8_t> rom{0x61, 0x32, 0x63, 0x36, 0x81, 0x37};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[1] == 0x4);
        REQUIRE(actual_V[0xF] == 1);
    }
    SECTION("8XY7 SUB with borrow") {
        // 81 37 -> Subtract the value V1 from register V3 in V1 and set VF to 0
        std::vector<uint8_t> rom{0x61, 0x32, 0x63, 0x26, 0x81, 0x37};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[0xF] == 0);
        REQUIRE(actual_V[1] == static_cast<uint8_t>(-1 * 0xC));
    }
    SECTION("8XY2 AND") {
        // 81 32 -> VA = VA & V6
        std::vector<uint8_t> rom{0x6A, 0x32, 0x66, 0x2C, 0x8A, 0x62};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[0xA] == (0x32 & 0x2C));
    }
    SECTION("8XY1 OR") {
        // 81 31 -> V1 = V1 | V3
        std::vector<uint8_t> rom{0x61, 0x12, 0x63, 0x36, 0x81, 0x31};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[1] == (0x12 | 0x36));
    }
    SECTION("8XY3 XOR") {
        // 89 53 -> V9 = V9 ^ V5
        std::vector<uint8_t> rom{0x69, 0x32, 0x65, 0x86, 0x89, 0x53};

        emulator.load_memory(rom);
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        emulator.step_one_cycle();
        auto actual_V = emulator.get_V_registers();

        REQUIRE(actual_V[9] == (0x32 ^ 0x86));
    }
}

