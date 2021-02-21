#include "interpreter.hpp"

auto main() -> int
{
    try
    {
        chip_8::Interpreter i;
        constexpr std::array<chip_8::Word, 3> program {
            0x6003, // V0 = 3
            0x6105, // V1 = 5
            0x8014, // V0 += V1
        };
        i.load_program(program);
        i.run_debug(program.size());
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}