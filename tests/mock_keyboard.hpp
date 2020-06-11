#ifndef MOCK_KEYBOARD_H_
#define MOCK_KEYBOARD_H_

#include "keyboard.hpp"
#include "trompeloeil.hpp"

class mockKeyboard : public keyboard {
public:
  MAKE_MOCK1(isKeyVxPressed, bool(const uint8_t &), override);
  MAKE_MOCK0(clearKeyInput, void(), override);
  MAKE_MOCK0(whichKeyIndexIfPressed, (std::pair<bool, uint8_t>()), override);
};

#endif // MOCK_KEYBOARD_H_
