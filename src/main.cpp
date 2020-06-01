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

// static constexpr std::array<uint8_t, 5> pixels{0x20, 0x60, 0x20, 0x20, 0x70};

static void read_file(std::vector<unsigned char> &rom) {
  std::ifstream file;

  file.open("BC_test.ch8", std::ios::binary | std::ios::ate);

  if (file.is_open()) {
    std::streampos size = file.tellg();
    char* buffer = new char[size];
    rom.reserve(size);
    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();
    for (uint i = 0; i < size; i++)
    {
      rom.push_back(static_cast<uint8_t>(buffer[i]));
    }
    delete[] buffer;
  }
  else{
    throw std::bad_exception();
  }
}

int main() {
  sf::RenderWindow window(sf::VideoMode(640.f, 480.f), "ImGui + SFML = <3");
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);

  sf::Color bgPixel(0, 0, 0, 255);           // Background pixels are black
  sf::Color spritePixel(255, 255, 255, 255); // Sprite pixel is White
  sf::Image CHIP8_window;
  sf::Texture texture;
  sf::Sprite chip8_sprite;

  constexpr int scaleFactor = 4;
  chip8_sprite.setScale(scaleFactor, scaleFactor);
  chip8_sprite.setPosition(float(window.getSize().x / 2) - (32 * scaleFactor),
                           float(window.getSize().y / 2) - (16 * scaleFactor));
  CHIP8_window.create(64.F, 32.F, sf::Color::White);

  sf::Clock deltaClock;

  std::vector<unsigned char> rom;
  read_file(rom);
  chip8 emulator;
  emulator.load_memory(rom);

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
    // unsigned int y = 0;
    // for (auto pixel : pixels) {
    //   for (unsigned int i = 0; i < 8; i++) {
    //     if ((pixel & (0x80 >> i))) {
    //       CHIP8_window.setPixel(i, y, spritePixel);
    //     } else {
    //       CHIP8_window.setPixel(i, y, bgPixel);
    //     }
    //   }
    //   ++y;
    // }

    // Load ROM

    emulator.step_one_cycle();

    texture.loadFromImage(CHIP8_window);
    chip8_sprite.setTexture(texture);
    window.draw(chip8_sprite);
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}