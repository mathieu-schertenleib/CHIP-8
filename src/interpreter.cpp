#include "interpreter.hpp"

#include <chrono>

chip_8::Interpreter::Interpreter()
    : m_random_engine(static_cast<unsigned int>(
          std::chrono::system_clock::now().time_since_epoch().count()))
{
}

void chip_8::Interpreter::run(unsigned int cycles)
{
    for (unsigned int i {0}; i < cycles; ++i)
    {
        execute(fetch_next_instruction());
    }
}

void chip_8::Interpreter::run_debug(unsigned int cycles)
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

void chip_8::Interpreter::op_00E0() noexcept
{
    m_display = {};
}

void chip_8::Interpreter::op_00EE()
{
    --m_stack_pointer;
    m_program_counter = m_stack.at(m_stack_pointer);
}

void chip_8::Interpreter::op_1nnn(Word instruction) noexcept
{
    m_program_counter = field_nnn(instruction);
}

void chip_8::Interpreter::op_2nnn()
{
    m_stack.at(m_stack_pointer) = m_program_counter;
    ++m_stack_pointer;
}

void chip_8::Interpreter::op_3xkk(Word instruction) noexcept
{
    if (m_registers[field_x(instruction)] == field_kk(instruction))
    {
        m_program_counter = static_cast<Word>(m_program_counter + 2);
    }
}

void chip_8::Interpreter::op_4xkk(Word instruction) noexcept
{
    if (m_registers[field_x(instruction)] != field_kk(instruction))
    {
        m_program_counter = static_cast<Word>(m_program_counter + 2);
    }
}

void chip_8::Interpreter::op_5xy0(Word instruction) noexcept
{
    if (m_registers[field_x(instruction)] == m_registers[field_y(instruction)])
    {
        m_program_counter = static_cast<Word>(m_program_counter + 2);
    }
}

void chip_8::Interpreter::op_6xkk(Word instruction) noexcept
{
    m_registers[field_x(instruction)] = field_kk(instruction);
}

void chip_8::Interpreter::op_7xkk(Word instruction) noexcept
{
    m_registers[field_x(instruction)] = static_cast<Byte>(
        m_registers[field_x(instruction)] + field_kk(instruction));
}

void chip_8::Interpreter::op_8xy0(Word instruction) noexcept
{
    m_registers[field_x(instruction)] = m_registers[field_y(instruction)];
}

void chip_8::Interpreter::op_8xy1(Word instruction) noexcept
{
    m_registers[field_x(instruction)] = static_cast<Byte>(
        m_registers[field_x(instruction)] | m_registers[field_y(instruction)]);
}

void chip_8::Interpreter::op_8xy2(Word instruction) noexcept
{
    m_registers[field_x(instruction)] = static_cast<Byte>(
        m_registers[field_x(instruction)] & m_registers[field_y(instruction)]);
}

void chip_8::Interpreter::op_8xy3(Word instruction) noexcept
{
    m_registers[field_x(instruction)] = static_cast<Byte>(
        m_registers[field_x(instruction)] ^ m_registers[field_y(instruction)]);
}

void chip_8::Interpreter::op_8xy4(Word instruction) noexcept
{
    const auto old_value {m_registers[field_x(instruction)]};
    m_registers[field_x(instruction)] = static_cast<Byte>(
        m_registers[field_x(instruction)] + m_registers[field_y(instruction)]);
    m_registers[0xF] = m_registers[field_x(instruction)] < old_value;
}

void chip_8::Interpreter::op_8xy5(Word instruction) noexcept
{
    const auto old_value {m_registers[field_x(instruction)]};
    m_registers[field_x(instruction)] = static_cast<Byte>(
        m_registers[field_x(instruction)] - m_registers[field_y(instruction)]);
    m_registers[0xF] = m_registers[field_x(instruction)] < old_value;
}

void chip_8::Interpreter::op_8xy6(Word instruction) noexcept
{
    m_registers[0xF] = m_registers[field_x(instruction)] & 0x1;
    m_registers[field_x(instruction)] =
        static_cast<Byte>(m_registers[field_x(instruction)] >> 1);
}

void chip_8::Interpreter::op_8xy7(Word instruction) noexcept
{
    const auto diff {static_cast<Byte>(m_registers[field_y(instruction)] -
                                       m_registers[field_x(instruction)])};
    m_registers[field_x(instruction)] = diff;
    m_registers[0xF] = diff < m_registers[field_y(instruction)];
}

void chip_8::Interpreter::op_8xyE(Word instruction) noexcept
{
    m_registers[0xF] = (m_registers[field_x(instruction)] & 0x80) >> 7;
    m_registers[field_x(instruction)] =
        static_cast<Byte>(m_registers[field_x(instruction)] << 1);
}

void chip_8::Interpreter::op_9xy0(Word instruction) noexcept
{
    if (m_registers[field_x(instruction)] != m_registers[field_y(instruction)])
    {
        m_program_counter = static_cast<Word>(m_program_counter + 2);
    }
}

void chip_8::Interpreter::op_Annn(Word instruction) noexcept
{
    m_address_register = field_nnn(instruction);
}

void chip_8::Interpreter::op_Bnnn(Word instruction) noexcept
{
    m_program_counter = static_cast<Word>((instruction) + m_registers[0x0]);
}

void chip_8::Interpreter::op_Cxkk(Word instruction)
{
    m_registers[field_x(instruction)] = static_cast<Byte>(
        std::default_random_engine(static_cast<unsigned int>(
            std::chrono::system_clock::now().time_since_epoch().count()))() &
        field_kk(instruction));
}

void chip_8::Interpreter::op_Dxyn(Word instruction) noexcept {}

void chip_8::Interpreter::op_Ex9E(Word instruction) noexcept
{
    if (m_key_states[m_registers[field_x(instruction)]])
    {
        m_program_counter = static_cast<Word>(m_program_counter + 2);
    }
}

void chip_8::Interpreter::op_ExA1(Word instruction) noexcept
{
    if (!m_key_states[m_registers[field_x(instruction)]])
    {
        m_program_counter = static_cast<Word>(m_program_counter + 2);
    }
}

void chip_8::Interpreter::op_Fx07(Word instruction) noexcept
{
    m_registers[field_x(instruction)] = m_delay_timer;
}

void chip_8::Interpreter::op_Fx0A(Word instruction) noexcept {}

void chip_8::Interpreter::op_Fx15(Word instruction) noexcept
{
    m_delay_timer = m_registers[field_x(instruction)];
}

void chip_8::Interpreter::op_Fx18(Word instruction) noexcept
{
    m_sound_timer = m_registers[field_x(instruction)];
}

void chip_8::Interpreter::op_Fx1E(Word instruction) noexcept
{
    m_address_register = static_cast<Word>(m_address_register +
                                           m_registers[field_x(instruction)]);
}

void chip_8::Interpreter::op_Fx29(Word instruction) noexcept {}

void chip_8::Interpreter::op_Fx33(Word instruction) noexcept {}

void chip_8::Interpreter::op_Fx55(Word instruction) noexcept
{
    for (std::size_t i {0}; i < field_x(instruction); ++i)
    {
        m_memory[m_address_register + i] = m_registers[i];
    }
}

void chip_8::Interpreter::op_Fx65(Word instruction) noexcept
{
    for (std::size_t i {0}; i < field_x(instruction); ++i)
    {
        m_registers[i] = m_memory[m_address_register + i];
    }
}

[[nodiscard]] auto chip_8::Interpreter::fetch_next_instruction() noexcept
    -> Word
{
    const auto instruction {static_cast<Word>(
        (m_memory[m_program_counter + 1] << 8) | m_memory[m_program_counter])};
    m_program_counter = static_cast<Word>(m_program_counter + 2);
    return instruction;
}

void chip_8::Interpreter::execute(Word instruction)
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
