// Own headers
#include "chip8.hpp"

// System headers
#include <array>
#include <fstream>
#include <vector>

// Third-party headers
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <argparse/argparse.hpp>
#include <boost/circular_buffer.hpp>
#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>

// Constants
static const sf::Color bgPixel{0, 0, 0, 255}; // Background pixels are black
static const sf::Color spritePixel{0, 255, 0, 255}; // Sprite pixel is Green

// Local function

static void read_file(std::vector<char> &rom, const std::string &file_name) {
  std::ifstream file;
  file.open(file_name.c_str(), std::ios::binary | std::ios::ate);

  if (file.is_open()) {
    std::streampos size = file.tellg();
    rom.resize(static_cast<std::size_t>(size));
    file.seekg(0, std::ios::beg);
    file.read(rom.data(), size);
    file.close();
  } else {
    throw std::invalid_argument("Given filename " + file_name +
                                " does not exist!");
  }
}

static void drawGfx(const std::array<uint8_t, display_size> &gfx,
                    sf::Image &window) {
  for (uint y = 0; y < display_y; ++y) {
    for (uint x = 0; x < display_x; ++x) {
      const auto pixel =
          (gfx.at(x + (display_x * y)) == 1) ? spritePixel : bgPixel;
      window.setPixel(x, y, pixel);
    }
  }
}

int main(int argc, char *argv[]) {
  // CLI Parser
  argparse::ArgumentParser program("CHIP8");
  program.add_argument("ROM").help("Specify the name of the ROM");
  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cout << err.what() << std::endl;
    std::cout << program;
    exit(0);
  }
  auto file_name = program.get<std::string>("ROM");

  // Emulator setup and load rom
  std::vector<char> rom;
  chip8 emulator;
  std::array<uint8_t, display_size> gfx{0};
  try {
    read_file(rom, file_name);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    std::abort();
  }
  emulator.load_memory(rom);

  // SFML Graphics
  constexpr int scaleFactor = 4;

  sf::RenderWindow window(sf::VideoMode(640.f, 480.f),
                          "CHIP8 Emulator/Interpretter");
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);

  sf::Image CHIP8_window;
  sf::Texture texture;
  sf::Sprite chip8_sprite;
  sf::Clock deltaClock;
  boost::circular_buffer<std::string> instr_cb(10);
  int slider_input = 10;
  bool fall_through = false;

  chip8_sprite.setScale(scaleFactor, scaleFactor);
  chip8_sprite.setPosition(float(window.getSize().x / 2) - (32 * scaleFactor),
                           float(window.getSize().y / 2) - (16 * scaleFactor));
  CHIP8_window.create(display_x, display_y, sf::Color::Black);

  while (window.isOpen()) {

    sf::Event event;
    bool shouldExecuteCycle = true;
    while (window.pollEvent(event)) {
      ImGui::SFML::ProcessEvent(event);

      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    ImGui::SFML::Update(window, deltaClock.restart());

    if constexpr (debug) {
      ImGui::Begin("Internal Register");
      ImGui::SetWindowPos(ImVec2(5, 5), ImGuiCond_Once);
      ImGui::BeginChild("Scrolling", ImVec2(300, 600));
      auto V_regiters = emulator.get_V_registers();
      auto index = 0;
      ImGui::TextColored(ImVec4(1, 0, 0, 1), "PC     : %d",
                         emulator.get_prog_counter());
      ImGui::Separator();
      ImGui::TextColored(ImVec4(1, 0, 0, 1), "I      : %#x",
                         emulator.get_I_register());
      ImGui::Separator();
      ImGui::TextColored(ImVec4(1, 0, 0, 1), "V register");
      for (auto reg : V_regiters) {
        ImGui::Text("V[0x%x] : %#x", index, reg);
        ++index;
      }
      ImGui::Separator();
      auto stack = emulator.get_stack();
      auto index1 = stack.size();
      ImGui::TextColored(ImVec4(1, 0, 0, 1), "Stack");
      while (!stack.empty()) {
        ImGui::Text("V[0x%x] : %#x", index, stack.top());
        stack.pop();
        --index1;
      }
      ImGui::EndChild();
      ImGui::End();
    }
    window.clear();

      ImGui::Begin("Adjust Speed");
      ImGui::SetWindowPos(ImVec2(800, 5), ImGuiCond_Once);
      ImGui::BeginChild("", ImVec2(300, 20));
      ImGui::SliderInt("", &slider_input, 1, 20);
      ImGui::EndChild();
      ImGui::End();

    if constexpr (debug) {
      bool step_next = false;
      ImGui::Begin("Debugger");
      ImGui::SetWindowPos(ImVec2(1200, 300), ImGuiCond_Once);

      if (ImGui::Button("Step Next")) {
        step_next = true;
      }
      if (ImGui::Button("Continue")) {
        fall_through = true;
      }
      if (ImGui::Button("Pause")) {
        fall_through = false;
      }
      shouldExecuteCycle = (step_next || fall_through);
      ImGui::End();
    }
    int cpu_freq = slider_input;
    while (cpu_freq--) {
      if (shouldExecuteCycle) {
        emulator.step_one_cycle();
        instr_cb.push_front(emulator.get_instruction());
      }
    }

    gfx = emulator.get_display_pixels();
    drawGfx(gfx, CHIP8_window);

    if constexpr (debug) {
      ImGui::Begin("Instruction window");
      ImGui::SetWindowFontScale(1.15F);
      ImGui::SetWindowPos(ImVec2(800, 800), ImGuiCond_Once);
      ImGui::BeginChild("Scrolling", ImVec2(300, 300));
      for (const auto &instr : instr_cb) {
        ImGui::Text("%s", instr.c_str());
      }
      ImGui::EndChild();
      ImGui::End();
    }
    texture.loadFromImage(CHIP8_window);
    chip8_sprite.setTexture(texture);
    window.draw(chip8_sprite);
    ImGui::SFML::Render(window);
    window.display();
  }
  ImGui::SFML::Shutdown();
}