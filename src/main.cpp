#include <array>
#include <fstream>
#include <vector>

#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "chip8.hpp"
#include "fmt/format.h"

static void read_file(std::vector<char> &rom) {
  std::ifstream file;
  file.open("test_opcode.ch8", std::ios::binary | std::ios::ate);

  if (file.is_open()) {
    std::streampos size = file.tellg();
    rom.resize(size);
    file.seekg(0, std::ios::beg);
    file.read(rom.data(), size);
    file.close();
  } else {
    throw std::bad_exception();
  }
}

int main() {
  std::vector<char> rom;
  chip8 emulator;
  constexpr int scaleFactor = 4;

  sf::RenderWindow window(sf::VideoMode(640.f, 480.f), "ImGui + SFML = <3");
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);

  sf::Color bgPixel(0, 0, 0, 255);           // Background pixels are black
  sf::Color spritePixel(255, 255, 255, 255); // Sprite pixel is White
  sf::Image CHIP8_window;
  sf::Texture texture;
  sf::Sprite chip8_sprite;

  chip8_sprite.setScale(scaleFactor, scaleFactor);
  chip8_sprite.setPosition(float(window.getSize().x / 2) - (32 * scaleFactor),
                           float(window.getSize().y / 2) - (16 * scaleFactor));
  CHIP8_window.create(64.F, 32.F, sf::Color::Black);

  // Load ROM
  read_file(rom);
  emulator.load_memory(rom);

  sf::Clock deltaClock;
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
      spritePixel = sf::Color::White;
    }
    if (ImGui::Button("White")) {
      bgPixel = sf::Color::White;
      spritePixel = sf::Color::Black;
    }
    ImGui::End();

    window.clear();

    emulator.step_one_cycle();
    auto gfx = emulator.get_display();

    for (uint y = 0; y < display_y; ++y) {
      for (uint x = 0; x < display_x; ++x) {
        if ((gfx[x + (display_x * y)]) == 1) {
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