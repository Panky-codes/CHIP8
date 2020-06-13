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
#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>


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

  sf::Color bgPixel{sf::Color::Black};     // Background pixels are black
  sf::Color spritePixel{sf::Color::Green}; // Sprite pixel is White
  sf::Image CHIP8_window;
  sf::Texture texture;
  sf::Sprite chip8_sprite;
  sf::Clock deltaClock;

  chip8_sprite.setScale(scaleFactor, scaleFactor);
  chip8_sprite.setPosition(float(window.getSize().x / 2) - (32 * scaleFactor),
                           float(window.getSize().y / 2) - (16 * scaleFactor));
  CHIP8_window.create(64.F, 32.F, sf::Color::Black);

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      ImGui::SFML::ProcessEvent(event);

      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    ImGui::SFML::Update(window, deltaClock.restart());

    ImGui::Begin("Change Background");
    if (ImGui::Button("Black")) {
      bgPixel = sf::Color::Black;
      spritePixel = sf::Color::Green;
    }
    if (ImGui::Button("Green")) {
      bgPixel = sf::Color::Green;
      spritePixel = sf::Color::Black;
    }
    ImGui::End();
    ImGui::Begin("Internal Register");
    ImGui::SetWindowPos(ImVec2(5,5), ImGuiCond_Once);
    ImGui::BeginChild("Scrolling", ImVec2(300, 700));
    auto V_regiters = emulator.get_V_registers();
    auto index = 0;
    ImGui::TextColored(ImVec4(1,0,0,1), "PC     : %d", emulator.get_prog_counter());
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1,0,0,1), "I      : %#x", emulator.get_I_register());
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1,0,0,1), "V register");
    for (auto reg : V_regiters) {
      ImGui::Text("V[0x%x] : %#x", index, reg);
      ++index;
    }
    ImGui::Separator();
    auto stack = emulator.get_stack();
    auto index1 = stack.size();
    ImGui::TextColored(ImVec4(1,0,0,1), "Stack");
    while(!stack.empty()){
      ImGui::Text("V[0x%x] : %#x", index, stack.top());
      stack.pop();
      --index1;
    }
    ImGui::EndChild();
    ImGui::End();

    window.clear();

    int nr = 10;
    while (nr--) {
      emulator.step_one_cycle();
      if (emulator.get_display_flag()) {
        gfx = emulator.get_display_pixels();
      }
    }
    for (uint y = 0; y < display_y; ++y) {
      for (uint x = 0; x < display_x; ++x) {
        if ((gfx.at(x + (display_x * y))) == 1) {
          CHIP8_window.setPixel(x, y, spritePixel);
        } else {
          CHIP8_window.setPixel(x, y, bgPixel);
        }
      }
    }

    texture.loadFromImage(CHIP8_window);
    chip8_sprite.setTexture(texture);
    window.draw(chip8_sprite);
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}