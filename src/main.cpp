#include "imgui-SFML.h"
#include "imgui.h"

#include "fmt/format.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <array>

static constexpr std::array<uint8_t, 5> pixels{0x20, 0x60, 0x20, 0x20, 0x70};

int main() {
  sf::RenderWindow window(sf::VideoMode(640.f, 480.f), "ImGui + SFML = <3");
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);
  sf::Color blackPixel(0, 0, 0, 255);
  sf::Color whitePixel(255, 255, 255, 255);
  sf::Image CHIP8_window;
  sf::Texture texture;
  sf::Sprite chip8_sprite;
  chip8_sprite.setScale(4, 4);
  chip8_sprite.setPosition(float(window.getSize().x / 2),
                           float(window.getSize().y / 2));
  CHIP8_window.create(8.F, 5.F, sf::Color::Black);

  for (auto pixel : pixels) {
    for (int i = 0; i < 8; i++) {
      if ((pixel & (0x80 >> i))) {
        fmt::print("*");
      } else {
        fmt::print(" ");
      }
    }
    fmt::print("\n");
  }

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

    ImGui::Begin("Hello, world!");
    ImGui::Button("Look at this pretty button");
    ImGui::End();
    ImGui::Begin("Change color here");
    ImGui::Text("Hello, world %d", 123);
    if (ImGui::Button("Save"))
      whitePixel = sf::Color::White;

    if (ImGui::Button("Load"))
      whitePixel = sf::Color::Blue;
    ImGui::End();

    // ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
    // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

    window.clear();
    unsigned int y = 0;
    for (auto pixel : pixels) {
      for (unsigned int i = 0; i < 8; i++) {
        if ((pixel & (0x80 >> i))) {
          CHIP8_window.setPixel(i, y, whitePixel);
        } else {
          CHIP8_window.setPixel(i, y, blackPixel);
        }
      }
      ++y;
    }

    texture.loadFromImage(CHIP8_window);
    chip8_sprite.setTexture(texture);
    window.draw(chip8_sprite);
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}