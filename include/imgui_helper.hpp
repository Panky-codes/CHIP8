#ifndef IMGUI_HELPER_H_
#define IMGUI_HELPER_H_

// System headers
#include <array>
#include <cstdint>
#include <stack>

// Third-party headers
#include <boost/circular_buffer.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

namespace IMGUI {
using circ_buf = boost::circular_buffer<std::string>;


struct chip8_registers {
  std::array<uint8_t, 16> V{0};
  std::stack<uint16_t> hw_stack;
  uint16_t prog_counter{};
  uint16_t I{0};
};

inline void draw_registers_window(chip8_registers &&regs) {

  ImGui::Begin("Internal Register");
  ImGui::SetWindowPos(ImVec2(5, 5), ImGuiCond_Once);
  ImGui::BeginChild("Scrolling", ImVec2(300, 600));
  ImGui::TextColored(ImVec4(1, 0, 0, 1), "PC     : %d", regs.prog_counter);
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1, 0, 0, 1), "I      : %#x", regs.I);
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1, 0, 0, 1), "V register");
  auto index = 0;
  for (auto reg : regs.V) {
    ImGui::Text("V[0x%x] : %#x", index, reg);
    ++index;
  }
  ImGui::Separator();
  auto stack_index = static_cast<uint>(regs.hw_stack.size());
  ImGui::TextColored(ImVec4(1, 0, 0, 1), "Stack");
  while (!regs.hw_stack.empty()) {
    ImGui::Text("V[0x%x] : %#x", stack_index, regs.hw_stack.top());
    regs.hw_stack.pop();
    --stack_index;
  }
  ImGui::EndChild();
  ImGui::End();
}

inline bool draw_debugger_options(bool &fall_through) {
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
  ImGui::End();
  return (step_next || fall_through);
}

inline void draw_instruction_window(const circ_buf &instr_cb) {
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

inline void draw_slider_window(int &slider_input) {
  ImGui::Begin("Adjust Speed");
  ImGui::SetWindowPos(ImVec2(800, 5), ImGuiCond_Once);
  ImGui::BeginChild("", ImVec2(300, 20));
  ImGui::SliderInt("", &slider_input, 1, 20);
  ImGui::EndChild();
  ImGui::End();
}
} // IMGUI 

#endif // IMGUI_HELPER_H_
