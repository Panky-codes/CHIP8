#include "imgui-SFML.h"
#include "imgui.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

int main() {
  sf::RenderWindow window(sf::VideoMode(640, 480), "ImGui + SFML = <3");
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);

  sf::CircleShape shape(100.f);
  shape.setFillColor(sf::Color::Green);
  shape.setOrigin(sf::Vector2f{-320.0f,-240.0f});

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
      shape.setFillColor(sf::Color::Red);

    if (ImGui::Button("Load"))
      shape.setFillColor(sf::Color::Green);
    ImGui::End();

    // ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
    // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    window.clear();
    window.draw(shape);
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}