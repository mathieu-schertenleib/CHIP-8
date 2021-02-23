#ifndef CHIP_8_INTERPRETER_HPP
#define CHIP_8_INTERPRETER_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>

namespace chip_8
{

using Byte = std::uint8_t;
using Word = std::uint16_t;

class Interpreter
{
public:
    Interpreter();

    template <std::size_t N>
    void load_program(std::array<Word, N> program) noexcept
    {
        for (std::size_t i {0}; i < N; ++i)
        {
            m_memory[512 + 2 * i] = program[i] & 0xFF;
            m_memory[512 + 2 * i + 1] = (program[i] >> 8) & 0xFF;
        }
    }

    void run(unsigned int cycles);

    void run_debug(unsigned int cycles);

private:
    [[nodiscard]] auto fetch_next_instruction() noexcept -> Word;

    void execute(Word instruction);

    // Clear the display.
    void op_00E0() noexcept;

    // Return from subroutine.
    void op_00EE();

    // Jump to location nnn.
    void op_1nnn(Word instruction) noexcept;

    // Call subroutine at location nnn.
    void op_2nnn();

    // Skip next instruction if Vx == kk.
    void op_3xkk(Word instruction) noexcept;

    // Skip next instruction if Vx != kk.
    void op_4xkk(Word instruction) noexcept;

    // Skip next instruction if Vx == Vy.
    void op_5xy0(Word instruction) noexcept;

    // Set Vx = kk.
    void op_6xkk(Word instruction) noexcept;

    // Set Vx = Vx + kk.
    void op_7xkk(Word instruction) noexcept;

    // Set Vx = Vy.
    void op_8xy0(Word instruction) noexcept;

    // Set Vx = Vx OR Vy.
    void op_8xy1(Word instruction) noexcept;

    // Set Vx = Vx AND Vy.
    void op_8xy2(Word instruction) noexcept;

    // Set Vx = Vx XOR Vy.
    void op_8xy3(Word instruction) noexcept;

    // Set Vx = Vx + Vy, set VF = carry.
    void op_8xy4(Word instruction) noexcept;

    // Set Vx = Vx - Vy, set VF = NOT borrow.
    void op_8xy5(Word instruction) noexcept;

    // Set VF = LSB(Vx), Vx = Vx SHR 1.
    void op_8xy6(Word instruction) noexcept;

    // Set Vx = Vy - Vx, set VF = NOT borrow.
    void op_8xy7(Word instruction) noexcept;

    // Set VF = MSB(Vx), Vx = Vx SHL 1.
    void op_8xyE(Word instruction) noexcept;

    // Skip next instruction if Vx != Vy.
    void op_9xy0(Word instruction) noexcept;

    // Set I = nnn.
    void op_Annn(Word instruction) noexcept;

    // Jump to location nnn + V0.
    void op_Bnnn(Word instruction) noexcept;

    // Set Vx = random byte AND kk.
    void op_Cxkk(Word instruction);

    // TODO
    // Display n-byte sprite starting at memory location I at (Vx, Vy), set
    // VF = collision.
    void op_Dxyn(Word instruction) noexcept;

    // Skip next instruction if key with the value of Vx is pressed.
    void op_Ex9E(Word instruction) noexcept;

    // Skip next instruction if key with the value of Vx is not pressed.
    void op_ExA1(Word instruction) noexcept;

    // Set Vx = delay timer value.
    void op_Fx07(Word instruction) noexcept;

    // TODO
    // Wait for a key press, store the value of the key in Vx.
    void op_Fx0A(Word instruction) noexcept;

    // Set delay timer = Vx.
    void op_Fx15(Word instruction) noexcept;

    // Set sound timer = Vx.
    void op_Fx18(Word instruction) noexcept;

    // Set I = I + Vx.
    void op_Fx1E(Word instruction) noexcept;

    // TODO
    // Set I = location of sprite for digit Vx.
    void op_Fx29(Word instruction) noexcept;

    // TODO
    // Store BCD representation of Vx in memory locations I, I+1, and I+2.
    void op_Fx33(Word instruction) noexcept;

    // Store registers V0 through Vx in memory starting at location I.
    void op_Fx55(Word instruction) noexcept;

    // Read registers V0 through Vx from memory starting at location I.
    void op_Fx65(Word instruction) noexcept;

    [[nodiscard]] static constexpr auto field_nnn(Word instruction) noexcept
        -> Word
    {
        return static_cast<Word>(instruction & 0x0FFF);
    }

    [[nodiscard]] static constexpr auto field_kk(Word instruction) noexcept
        -> Byte
    {
        return static_cast<Byte>(instruction & 0x00FF);
    }

    [[nodiscard]] static constexpr auto field_x(Word instruction) noexcept
        -> Byte
    {
        return static_cast<Byte>((instruction & 0x0F00) >> 8);
    }

    [[nodiscard]] static constexpr auto field_y(Word instruction) noexcept
        -> Byte
    {
        return static_cast<Byte>((instruction & 0x00F0) >> 4);
    }

    [[nodiscard]] static constexpr auto field_n(Word instruction) noexcept
        -> Byte
    {
        return static_cast<Byte>(instruction & 0x000F);
    }

    std::array<Byte, 4096> m_memory {};
    std::array<std::array<bool, 32>, 64> m_display {};
    std::array<Word, 16> m_stack {};
    std::array<Byte, 16> m_registers {};
    std::array<bool, 16> m_key_states {};
    std::default_random_engine m_random_engine;
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
} // namespace chip_8

#endif // CHIP_8_INTERPRETER_HPP
