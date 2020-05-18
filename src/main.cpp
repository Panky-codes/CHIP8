#include "chip8.hpp"

int main() 
{
    chip8 emulator;
    std::vector<uint8_t> rom {0x61,0x32};
    emulator.load_memory(rom);
    emulator.step_one_cycle();

 return 0;
}

                
