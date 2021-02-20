#ifndef CHIP_8_SRC_INTERPRETER_HPP
#define CHIP_8_SRC_INTERPRETER_HPP

#include <array>
#include <cstdint>

class Chip8
{
private:
    using Byte = std::uint8_t;
    using Word = std::uint16_t;

    std::array<Byte, 4096> m_memory {};
    std::array<Word, 16> m_stack {};
    std::array<Byte, 16> m_registers {};
    Word m_address_register {};
    Word m_program_counter {512};
    Byte m_stack_pointer {};
    Byte m_delay_timer {};
    Byte m_sound_timer {};

    static constexpr Byte sprite_data[16][5] {{0xF0, 0x90, 0x90, 0x90, 0xF0},
                                              {0x20, 0x60, 0x20, 0x20, 0x70},
                                              {0xF0, 0x10, 0xF0, 0x80, 0xF0},
                                              {0xF0, 0x10, 0xF0, 0x10, 0xF0},
                                              {0x90, 0x90, 0xF0, 0x10, 0x10},
                                              {0xF0, 0x80, 0xF0, 0x10, 0xF0},
                                              {0xF0, 0x80, 0xF0, 0x90, 0xF0},
                                              {0xF0, 0x10, 0x20, 0x40, 0x40},
                                              {0xF0, 0x90, 0xF0, 0x90, 0xF0},
                                              {0xF0, 0x90, 0xF0, 0x10, 0xF0},
                                              {0xF0, 0x90, 0xF0, 0x90, 0x90},
                                              {0xE0, 0x90, 0xE0, 0x90, 0xE0},
                                              {0xF0, 0x80, 0x80, 0x80, 0xF0},
                                              {0xE0, 0x90, 0x90, 0x90, 0xE0},
                                              {0xF0, 0x80, 0xF0, 0x80, 0xF0},
                                              {0xF0, 0x80, 0xF0, 0x80, 0x80}};
};

#endif // CHIP_8_SRC_INTERPRETER_HPP
