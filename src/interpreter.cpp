#include "interpreter.hpp"

#include <random>
#include <chrono>

void chip_8::Interpreter::op_Cxkk(Word instruction)
{
    m_registers[field_x(instruction)] = static_cast<Byte>(
        std::default_random_engine(static_cast<unsigned int>(
            std::chrono::system_clock::now().time_since_epoch().count()))() &
        field_kk(instruction));
}