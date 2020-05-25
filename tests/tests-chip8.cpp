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
  SECTION("8XY6 SHIFT RIGHT WITH LSB 0") {
    // 89 53 -> V9 = V5 >> 1 and V[F] = 0
    std::vector<uint8_t> rom{0x69, 0x32, 0x65, 0x86, 0x89, 0x56};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[5] == static_cast<uint8_t>(0x86 >> 1));
    REQUIRE(actual_V[9] == static_cast<uint8_t>(0x86 >> 1));
    REQUIRE(actual_V[0xF] == 0);
  }
  SECTION("8XY6 SHIFT RIGHT WITH LSB 1") {
    // 89 53 -> V9 = V5 >> 1 and V[F] = 1
    std::vector<uint8_t> rom{0x69, 0x32, 0x65, 0x85, 0x89, 0x56};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[5] == static_cast<uint8_t>(0x85 >> 1));
    REQUIRE(actual_V[9] == static_cast<uint8_t>(0x85 >> 1));
    REQUIRE(actual_V[0xF] == 1);
  }
  SECTION("8XYE SHIFT LEFT WITH MSB 0") {
    // 89 53 -> V9 = V5 << 1 and V[F] = 0
    std::vector<uint8_t> rom{0x69, 0x32, 0x65, 0x76, 0x89, 0x5E};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[5] == static_cast<uint8_t>(0x76 << 1));
    REQUIRE(actual_V[9] == static_cast<uint8_t>(0x76 << 1));
    REQUIRE(actual_V[0xF] == 0);
  }
  SECTION("8XYE SHIFT LEFT WITH MSB 1") {
    // 89 53 -> V9 = V5 << 1 and V[F] = 0
    std::vector<uint8_t> rom{0x69, 0x32, 0x65, 0x86, 0x89, 0x5E};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[5] == static_cast<uint8_t>(0x86 << 1));
    REQUIRE(actual_V[9] == static_cast<uint8_t>(0x86 << 1));
    REQUIRE(actual_V[0xF] == 1);
  }
  SECTION("CXNN Random number generator") {
    std::vector<uint8_t> rom{0xC7, 0xDD};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[5] < 0xDD);
  }
  SECTION("1NNN JUMP") {
    std::vector<uint8_t> rom{0x17, 0xDD};

    emulator.load_memory(rom);
    emulator.step_one_cycle();

    REQUIRE(emulator.get_prog_counter() == 0x7DD);
  }
  SECTION("BNNN JUMP WITH OFFSET") {
    std::vector<uint8_t> rom{0x60, 0x32, 0xB7, 0xDD};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();

    REQUIRE(emulator.get_prog_counter() == (0x7DD + 0x32));
  }
  SECTION("2NNN SUBROUTINE") {
    std::vector<uint8_t> rom{0x60, 0x32, 0x27, 0xDD};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    const auto pc_before_SUBR_call = emulator.get_prog_counter();
    emulator.step_one_cycle();
    const auto stack = emulator.get_stack();

    REQUIRE(stack.top() == (pc_before_SUBR_call + 2));
  }
  SECTION("00E0 RET") {
    // STA 0x32 in V1 register
    // JMP to address 0x206
    // STA 0x36 in V1 register
    // RET
    // STA 0x56 in V5 register
    std::vector<uint8_t> rom{0x61, 0x32, 0x22, 0x06, 0x65,
                             0x56, 0x61, 0x36, 0x00, 0xEE};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[1] == (0x36));
    REQUIRE(actual_V[5] == (0x56));
  }
  SECTION("3XNN IF EQUAL") {
    std::vector<uint8_t> rom{0x68, 0x32, 0x38, 0x32, 0x68, 0x82, 0x68, 0x12};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[8] == 0x12);
  }
  SECTION("3XNN IF EQUAL BADWEATHER") {
    std::vector<uint8_t> rom{0x68, 0x32, 0x38, 0x34, 0x68, 0x82, 0x68, 0x12};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[8] == 0x82);
  }
  SECTION("4XNN IF NOT EQUAL") {
    std::vector<uint8_t> rom{0x68, 0x32, 0x48, 0x34, 0x68, 0x82, 0x68, 0x12};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[8] == 0x12);
  }
  SECTION("4XNN IF NOT EQUAL BADWEATHER") {
    std::vector<uint8_t> rom{0x68, 0x32, 0x48, 0x32, 0x68, 0x82, 0x68, 0x12};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[8] == 0x82);
  }
  SECTION("5XY0 IF X and Y ARE EQUAL") {
    std::vector<uint8_t> rom{0x68, 0x32, 0x67, 0x32, 0x58,
                             0x70, 0x68, 0x82, 0x68, 0x12};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[8] == 0x12);
  }
  SECTION("5XY0 IF X and Y ARE EQUAL BADWEATHER") {
    std::vector<uint8_t> rom{0x68, 0x32, 0x67, 0x34, 0x58,
                             0x70, 0x68, 0x82, 0x68, 0x12};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[8] == 0x82);
  }
  SECTION("9XY0 IF X and Y ARE NOT EQUAL") {
    std::vector<uint8_t> rom{0x68, 0x32, 0x67, 0x34, 0x98,
                             0x70, 0x68, 0x82, 0x68, 0x12};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[8] == 0x12);
  }
  SECTION("9XY0 IF X and Y ARE NOT EQUAL BADWEATHER") {
    std::vector<uint8_t> rom{0x68, 0x32, 0x67, 0x32, 0x98,
                             0x70, 0x68, 0x82, 0x68, 0x12};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[8] == 0x82);
  }
  SECTION("FX15 LDA Delay time") {
    std::vector<uint8_t> rom{0x68, 0x32, 0xF8, 0x15};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();

    REQUIRE(emulator.get_delay_counter() == 0x32);
  }
  SECTION("FX07 LDA Delay time in Vx") {
    std::vector<uint8_t> rom{0x68, 0x32, 0xF8, 0x15, 0xF8, 0x07};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    auto actual_V = emulator.get_V_registers();

    REQUIRE(actual_V[8] == 0x31);
  }
  SECTION("FX18 LDA Sound timer") {
    std::vector<uint8_t> rom{0x6C, 0x32, 0xFC, 0x18};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();

    REQUIRE(emulator.get_sound_counter() == 0x32);
  }
  SECTION("ANNN STA NNN in I register") {
    std::vector<uint8_t> rom{0xAC, 0x32};

    emulator.load_memory(rom);
    emulator.step_one_cycle();

    REQUIRE(emulator.get_I_register() == 0x0C32);
  }
  SECTION("FX1E ADD Vx to I register") {
    std::vector<uint8_t> rom{0x6C, 0x02,0xAC, 0x32, 0xFC, 0x1E};

    emulator.load_memory(rom);
    emulator.step_one_cycle();
    emulator.step_one_cycle();
    emulator.step_one_cycle();

    REQUIRE(emulator.get_I_register() == 0x0C34);
  }
}
