// Own headers
#include "chip8.hpp"
#include "imgui_helper.hpp"

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
  boost::circular_buffer<std::string> instr_cb(10);
  int slider_input = 10;
  bool fall_through = false;
  sf::Image CHIP8_window;
  sf::Texture texture;
  sf::Sprite chip8_sprite;
  sf::Clock deltaClock;

  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);
  chip8_sprite.setScale(scaleFactor, scaleFactor);
  chip8_sprite.setPosition(float(window.getSize().x / 2) - (32 * scaleFactor),
                           float(window.getSize().y / 2) - (16 * scaleFactor));
  CHIP8_window.create(display_x, display_y, sf::Color::Black);

  // Main emulator loop
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

    window.clear();

    if constexpr (debug) {
      using ch8_regs = IMGUI::chip8_registers;
      IMGUI::draw_registers_window(
          ch8_regs{emulator.get_V_registers(), emulator.get_stack(),
                   emulator.get_prog_counter(), emulator.get_I_register()});
    }

    IMGUI::draw_slider_window(slider_input);

    if constexpr (debug) {
      // mutates fall_through option based on input
      shouldExecuteCycle = IMGUI::draw_debugger_options(fall_through);
    }

    int cpu_freq = slider_input;
    while (cpu_freq--) {
      if (shouldExecuteCycle) {
        emulator.step_one_cycle();
        instr_cb.push_front(emulator.get_instruction());
      }
    }

    const auto gfx = emulator.get_display_pixels();
    drawGfx(gfx, CHIP8_window);

    if constexpr (debug) {
      IMGUI::draw_instruction_window(instr_cb);
    }

    texture.loadFromImage(CHIP8_window);
    chip8_sprite.setTexture(texture);
    window.draw(chip8_sprite);
    ImGui::SFML::Render(window);
    window.display();
  }
  ImGui::SFML::Shutdown();
}