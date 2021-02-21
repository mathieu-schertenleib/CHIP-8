#ifndef CHIP_8_SRC_INTERPRETER_HPP
#define CHIP_8_SRC_INTERPRETER_HPP

#include <array>
#include <cstdint>
#include <stdexcept>

class Chip8
{
private:
    using Byte = std::uint8_t;
    using Word = std::uint16_t;

    enum struct Result
    {
        success,
        invalid_opcode,
        unsupported_instruction,
        stack_overflow,
        stack_underflow,
        out_of_bounds_program_counter
    };

    static constexpr auto execute_0nnn() noexcept -> Result
    {
        return Result::unsupported_instruction;
    }

    constexpr auto execute_00E0() noexcept -> Result {}

    constexpr auto execute_00EE() noexcept -> Result
    {
        if (m_stack_pointer > 0)
        {
            --m_stack_pointer;
            m_program_counter = m_stack[m_stack_pointer];
            return Result::success;
        }
        else
        {
            return Result::stack_underflow;
        }
    }

    constexpr auto execute_1nnn() noexcept -> Result
    {
        m_program_counter = m_instruction_register & 0x0FFF;
        return Result::success;
    }

    constexpr auto execute_2nnn() noexcept -> Result
    {
        if (m_stack_pointer < m_stack.size() - 1)
        {
            m_stack[m_stack_pointer] = m_program_counter;
            ++m_stack_pointer;
            return Result::success;
        }
        else
        {
            return Result::stack_overflow;
        }
    }

    constexpr auto execute_3xkk() noexcept -> Result {}

    constexpr auto execute_4xkk() noexcept -> Result {}

    constexpr auto execute_5xy0() noexcept -> Result {}

    constexpr auto execute_6xkk() noexcept -> Result {}

    constexpr auto execute_7xkk() noexcept -> Result {}

    constexpr auto execute_8xy0() noexcept -> Result {}

    constexpr auto execute_8xy1() noexcept -> Result {}

    constexpr auto execute_8xy2() noexcept -> Result {}

    constexpr auto execute_8xy3() noexcept -> Result {}

    constexpr auto execute_8xy4() noexcept -> Result {}

    constexpr auto execute_8xy5() noexcept -> Result {}

    constexpr auto execute_8xy6() noexcept -> Result {}

    constexpr auto execute_8xy7() noexcept -> Result {}

    constexpr auto execute_8xyE() noexcept -> Result {}

    constexpr auto execute_9xy0() noexcept -> Result {}

    constexpr auto execute_Annn() noexcept -> Result {}

    constexpr auto execute_Bnnn() noexcept -> Result {}

    constexpr auto execute_Cxkk() noexcept -> Result {}

    constexpr auto execute_Dxyn() noexcept -> Result {}

    constexpr auto execute_Ex9E() noexcept -> Result {}

    constexpr auto execute_ExA1() noexcept -> Result {}

    constexpr auto execute_Fx07() noexcept -> Result {}

    constexpr auto execute_Fx0A() noexcept -> Result {}

    constexpr auto execute_Fx15() noexcept -> Result {}

    constexpr auto execute_Fx18() noexcept -> Result {}

    constexpr auto execute_Fx1E() noexcept -> Result {}

    constexpr auto execute_Fx29() noexcept -> Result {}

    constexpr auto execute_Fx33() noexcept -> Result {}

    constexpr auto execute_Fx55() noexcept -> Result {}

    constexpr auto execute_Fx65() noexcept -> Result {}

    constexpr auto fetch_next_instruction() noexcept -> Result
    {
        if (m_program_counter < m_memory.size() - 2)
        {
            m_instruction_register =
                static_cast<Word>((m_memory[m_program_counter + 1] << 8) |
                                  m_memory[m_program_counter]);
            m_program_counter += 2;
            return Result::success;
        }
        else
        {
            return Result::out_of_bounds_program_counter;
        }
    }

    constexpr auto execute_instruction() noexcept -> Result
    {
        switch ((m_instruction_register & 0xF000) >> 12)
        {
        case 0x0:
            if (m_instruction_register == 0x00E0)
                return execute_00E0();
            else if (m_instruction_register == 0x00EE)
                return execute_00EE();
            else
                return execute_0nnn();
        case 0x1:
            return execute_1nnn();
        case 0x2:
            return execute_2nnn();
        case 0x3:
            return execute_3xkk();
        case 0x4:
            return execute_4xkk();
        case 0x5:
            return execute_5xy0();
        case 0x6:
            return execute_6xkk();
        case 0x7:
            return execute_7xkk();
        case 0x8:
            switch (m_instruction_register & 0x000F)
            {
            case 0x0:
                return execute_8xy0();
            case 0x1:
                return execute_8xy1();
            case 0x2:
                return execute_8xy2();
            case 0x3:
                return execute_8xy3();
            case 0x4:
                return execute_8xy4();
            case 0x5:
                return execute_8xy5();
            case 0x6:
                return execute_8xy6();
            case 0x7:
                return execute_8xy7();
            case 0xE:
                return execute_8xyE();
            default:
                return Result::invalid_opcode;
            }
        case 0x9:
            return execute_9xy0();
        case 0xA:
            return execute_Annn();
        case 0xB:
            return execute_Bnnn();
        case 0xC:
            return execute_Cxkk();
        case 0xD:
            return execute_Dxyn();
        case 0xE:
            if ((m_instruction_register & 0x00FF) == 0x9E)
                return execute_Ex9E();
            else if ((m_instruction_register & 0x00FF) == 0xA1)
                return execute_ExA1();
            else
                return Result::invalid_opcode;
        case 0xF:
            switch (m_instruction_register & 0x00FF)
            {
            case 0x07:
                return execute_Fx07();
            case 0x0A:
                return execute_Fx0A();
            case 0x15:
                return execute_Fx15();
            case 0x18:
                return execute_Fx18();
            case 0x1E:
                return execute_Fx1E();
            case 0x29:
                return execute_Fx29();
            case 0x33:
                return execute_Fx33();
            case 0x55:
                return execute_Fx55();
            case 0x65:
                return execute_Fx65();
            default:
                return Result::invalid_opcode;
            }
        default:
            return Result::invalid_opcode;
        }
    }

    std::array<Byte, 4096> m_memory {};
    std::array<Word, 16> m_stack {};
    std::array<Byte, 16> m_registers {};
    Word m_address_register {};
    Word m_program_counter {512};
    Word m_instruction_register {};
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
