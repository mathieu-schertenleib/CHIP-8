#ifndef CHIP_8_SRC_INTERPRETER_HPP
#define CHIP_8_SRC_INTERPRETER_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <type_traits>

// TODO: remove all constexpr execution because an actual program cannot be
// executed at compile-time anyway (IO, RNG, timers). Also allows to clean up
// the header file.

namespace chip_8
{
using Byte = std::uint8_t;
using Word = std::uint16_t;

// TODO: These should be private static in the Interpreter/Chip8 class
[[nodiscard]] constexpr auto field_nnn(Word instruction) noexcept
{
    return static_cast<Word>(instruction & 0x0FFF);
}

[[nodiscard]] constexpr auto field_kk(Word instruction) noexcept
{
    return static_cast<Byte>(instruction & 0x00FF);
}

[[nodiscard]] constexpr auto field_x(Word instruction) noexcept
{
    return static_cast<Byte>((instruction & 0x0F00) >> 8);
}

[[nodiscard]] constexpr auto field_y(Word instruction) noexcept
{
    return static_cast<Byte>((instruction & 0x00F0) >> 4);
}

[[nodiscard]] constexpr auto field_n(Word instruction) noexcept
{
    return static_cast<Byte>(instruction & 0x000F);
}

class Interpreter
{
public:
    template <std::size_t N>
    constexpr void load_program(std::array<Byte, N> program) noexcept
        requires(N <= 4096 - 512)
    {
        for (std::size_t i {0}; i < N; ++i)
        {
            m_memory[512 + i] = program[i];
        }
    }

    template <std::size_t N>
    constexpr void load_program(std::array<Word, N> program) noexcept
        requires(N <= (4096 - 512) / 2)
    {
        for (std::size_t i {0}; i < N; ++i)
        {
            m_memory[512 + 2 * i] = program[i] & 0xFF;
            m_memory[512 + 2 * i + 1] = (program[i] >> 8) & 0xFF;
        }
    }

    constexpr void run(unsigned int cycles)
    {
        for (unsigned int i {0}; i < cycles; ++i)
        {
            execute(fetch_next_instruction());
        }
    }

    void run_debug(unsigned int cycles)
    {
        for (unsigned int i {0}; i < cycles; ++i)
        {
            const auto instruction {fetch_next_instruction()};
            execute(instruction);
            std::cout << std::hex << instruction << "; ";
            for (const auto reg : m_registers)
            {
                std::cout << static_cast<int>(reg) << ' ';
            }
            std::cout << '\n';
        }
    }

private:
    // Clear the display.
    constexpr void op_00E0() noexcept { m_display = {}; }

    // Return from subroutine.
    constexpr void op_00EE()
    {
        --m_stack_pointer;
        m_program_counter = m_stack.at(m_stack_pointer);
    }

    // Jump to location nnn.
    constexpr void op_1nnn(Word instruction) noexcept
    {
        m_program_counter = field_nnn(instruction);
    }

    // Call subroutine at location nnn.
    constexpr void op_2nnn()
    {
        m_stack.at(m_stack_pointer) = m_program_counter;
        ++m_stack_pointer;
    }

    // Skip next instruction if Vx == kk.
    constexpr void op_3xkk(Word instruction) noexcept
    {
        if (m_registers[field_x(instruction)] == field_kk(instruction))
        {
            m_program_counter = static_cast<Word>(m_program_counter + 2);
        }
    }

    // Skip next instruction if Vx != kk.
    constexpr void op_4xkk(Word instruction) noexcept
    {
        if (m_registers[field_x(instruction)] != field_kk(instruction))
        {
            m_program_counter = static_cast<Word>(m_program_counter + 2);
        }
    }

    // Skip next instruction if Vx == Vy.
    constexpr void op_5xy0(Word instruction) noexcept
    {
        if (m_registers[field_x(instruction)] ==
            m_registers[field_y(instruction)])
        {
            m_program_counter = static_cast<Word>(m_program_counter + 2);
        }
    }

    // Set Vx = kk.
    constexpr void op_6xkk(Word instruction) noexcept
    {
        m_registers[field_x(instruction)] = field_kk(instruction);
    }

    // Set Vx = Vx + kk.
    constexpr void op_7xkk(Word instruction) noexcept
    {
        m_registers[field_x(instruction)] = static_cast<Byte>(
            m_registers[field_x(instruction)] + field_kk(instruction));
    }

    // Set Vx = Vy.
    constexpr void op_8xy0(Word instruction) noexcept
    {
        m_registers[field_x(instruction)] = m_registers[field_y(instruction)];
    }

    // Set Vx = Vx OR Vy.
    constexpr void op_8xy1(Word instruction) noexcept
    {
        m_registers[field_x(instruction)] =
            static_cast<Byte>(m_registers[field_x(instruction)] |
                              m_registers[field_y(instruction)]);
    }

    // Set Vx = Vx AND Vy.
    constexpr void op_8xy2(Word instruction) noexcept
    {
        m_registers[field_x(instruction)] =
            static_cast<Byte>(m_registers[field_x(instruction)] &
                              m_registers[field_y(instruction)]);
    }

    // Set Vx = Vx XOR Vy.
    constexpr void op_8xy3(Word instruction) noexcept
    {
        m_registers[field_x(instruction)] =
            static_cast<Byte>(m_registers[field_x(instruction)] ^
                              m_registers[field_y(instruction)]);
    }

    // Set Vx = Vx + Vy, set VF = carry.
    constexpr void op_8xy4(Word instruction) noexcept
    {
        const auto old_value {m_registers[field_x(instruction)]};
        m_registers[field_x(instruction)] =
            static_cast<Byte>(m_registers[field_x(instruction)] +
                              m_registers[field_y(instruction)]);
        m_registers[0xF] = m_registers[field_x(instruction)] < old_value;
    }

    // Set Vx = Vx - Vy, set VF = NOT borrow.
    constexpr void op_8xy5(Word instruction) noexcept
    {
        const auto old_value {m_registers[field_x(instruction)]};
        m_registers[field_x(instruction)] =
            static_cast<Byte>(m_registers[field_x(instruction)] -
                              m_registers[field_y(instruction)]);
        m_registers[0xF] = m_registers[field_x(instruction)] < old_value;
    }

    // Set VF = LSB(Vx), Vx = Vx SHR 1.
    constexpr void op_8xy6(Word instruction) noexcept
    {
        m_registers[0xF] = m_registers[field_x(instruction)] & 0x1;
        m_registers[field_x(instruction)] =
            static_cast<Byte>(m_registers[field_x(instruction)] >> 1);
    }

    // Set Vx = Vy - Vx, set VF = NOT borrow.
    constexpr void op_8xy7(Word instruction) noexcept
    {
        const auto diff {static_cast<Byte>(m_registers[field_y(instruction)] -
                                           m_registers[field_x(instruction)])};
        m_registers[field_x(instruction)] = diff;
        m_registers[0xF] = diff < m_registers[field_y(instruction)];
    }

    // Set VF = MSB(Vx), Vx = Vx SHL 1.
    constexpr void op_8xyE(Word instruction) noexcept
    {
        m_registers[0xF] = (m_registers[field_x(instruction)] & 0x80) >> 7;
        m_registers[field_x(instruction)] =
            static_cast<Byte>(m_registers[field_x(instruction)] << 1);
    }

    // Skip next instruction if Vx != Vy.
    constexpr void op_9xy0(Word instruction) noexcept
    {
        if (m_registers[field_x(instruction)] !=
            m_registers[field_y(instruction)])
        {
            m_program_counter = static_cast<Word>(m_program_counter + 2);
        }
    }

    // Set I = nnn.
    constexpr void op_Annn(Word instruction) noexcept
    {
        m_address_register = field_nnn(instruction);
    }

    // Jump to location nnn + V0.
    constexpr void op_Bnnn(Word instruction) noexcept
    {
        m_program_counter = static_cast<Word>((instruction) + m_registers[0x0]);
    }

    // Set Vx = random byte AND kk.
    void op_Cxkk(Word instruction);

    // TODO
    // Display n-byte sprite starting at memory location I at (Vx, Vy), set
    // VF = collision.
    constexpr void op_Dxyn(Word instruction) noexcept {}

    // Skip next instruction if key with the value of Vx is pressed.
    constexpr void op_Ex9E(Word instruction) noexcept
    {
        if (m_key_states[m_registers[field_x(instruction)]])
        {
            m_program_counter = static_cast<Word>(m_program_counter + 2);
        }
    }

    // Skip next instruction if key with the value of Vx is not pressed.
    constexpr void op_ExA1(Word instruction) noexcept
    {
        if (!m_key_states[m_registers[field_x(instruction)]])
        {
            m_program_counter = static_cast<Word>(m_program_counter + 2);
        }
    }

    // Set Vx = delay timer value.
    constexpr void op_Fx07(Word instruction) noexcept
    {
        m_registers[field_x(instruction)] = m_delay_timer;
    }

    // TODO
    // Wait for a key press, store the value of the key in Vx.
    constexpr void op_Fx0A(Word instruction) noexcept {}

    // Set delay timer = Vx.
    constexpr void op_Fx15(Word instruction) noexcept
    {
        m_delay_timer = m_registers[field_x(instruction)];
    }

    // Set sound timer = Vx.
    constexpr void op_Fx18(Word instruction) noexcept
    {
        m_sound_timer = m_registers[field_x(instruction)];
    }

    // Set I = I + Vx.
    constexpr void op_Fx1E(Word instruction) noexcept
    {
        m_address_register = static_cast<Word>(
            m_address_register + m_registers[field_x(instruction)]);
    }

    // TODO
    // Set I = location of sprite for digit Vx.
    constexpr void op_Fx29(Word instruction) noexcept {}

    // TODO
    // Store BCD representation of Vx in memory locations I, I+1, and I+2.
    constexpr void op_Fx33(Word instruction) noexcept {}

    // Store registers V0 through Vx in memory starting at location I.
    constexpr void op_Fx55(Word instruction) noexcept
    {
        for (std::size_t i {0}; i < field_x(instruction); ++i)
        {
            m_memory[m_address_register + i] = m_registers[i];
        }
    }

    // Read registers V0 through Vx from memory starting at location I.
    constexpr void op_Fx65(Word instruction) noexcept
    {
        for (std::size_t i {0}; i < field_x(instruction); ++i)
        {
            m_registers[i] = m_memory[m_address_register + i];
        }
    }

    [[nodiscard]] constexpr auto fetch_next_instruction() noexcept -> Word
    {
        const auto instruction {
            static_cast<Word>((m_memory[m_program_counter + 1] << 8) |
                              m_memory[m_program_counter])}; // Big-endian
        m_program_counter = static_cast<Word>(m_program_counter + 2);
        return instruction;
    }

    constexpr void execute(Word instruction)
    {
        switch ((instruction & 0xF000) >> 12)
        {
        case 0x0:
            if (instruction == 0x00E0)
                return op_00E0();
            else if (instruction == 0x00EE)
                return op_00EE();
            else
                throw std::runtime_error(
                    "0nnn: unsupported instruction (jump to host machine code "
                    "instruction at addess nnn)");
        case 0x1: return op_1nnn(instruction);
        case 0x2: return op_2nnn();
        case 0x3: return op_3xkk(instruction);
        case 0x4: return op_4xkk(instruction);
        case 0x5: return op_5xy0(instruction);
        case 0x6: return op_6xkk(instruction);
        case 0x7: return op_7xkk(instruction);
        case 0x8:
            switch (instruction & 0x000F)
            {
            case 0x0: return op_8xy0(instruction);
            case 0x1: return op_8xy1(instruction);
            case 0x2: return op_8xy2(instruction);
            case 0x3: return op_8xy3(instruction);
            case 0x4: return op_8xy4(instruction);
            case 0x5: return op_8xy5(instruction);
            case 0x6: return op_8xy6(instruction);
            case 0x7: return op_8xy7(instruction);
            case 0xE: return op_8xyE(instruction);
            default: return;
            }
        case 0x9: return op_9xy0(instruction);
        case 0xA: return op_Annn(instruction);
        case 0xB: return op_Bnnn(instruction);
        case 0xC: return op_Cxkk(instruction);
        case 0xD: return op_Dxyn(instruction);
        case 0xE:
            if ((instruction & 0x00FF) == 0x9E)
                return op_Ex9E(instruction);
            else if ((instruction & 0x00FF) == 0xA1)
                return op_ExA1(instruction);
            else
                throw std::runtime_error("Invalid opcode");
        case 0xF:
            switch (instruction & 0x00FF)
            {
            case 0x07: return op_Fx07(instruction);
            case 0x0A: return op_Fx0A(instruction);
            case 0x15: return op_Fx15(instruction);
            case 0x18: return op_Fx18(instruction);
            case 0x1E: return op_Fx1E(instruction);
            case 0x29: return op_Fx29(instruction);
            case 0x33: return op_Fx33(instruction);
            case 0x55: return op_Fx55(instruction);
            case 0x65: return op_Fx65(instruction);
            default: throw std::runtime_error("Invalid opcode");
            }
        default: throw std::runtime_error("Invalid opcode");
        }
    }

    std::array<Byte, 4096> m_memory {};
    std::array<std::array<bool, 32>, 64> m_display {};
    std::array<Word, 16> m_stack {};
    std::array<Byte, 16> m_registers {};
    std::array<bool, 16> m_key_states {};
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

#endif // CHIP_8_SRC_INTERPRETER_HPP
