#include <cstdio>
#include <vector>

#include "logger.hpp"

auto main() -> int
{
    insys::logger logger { insys::logger::format("{white+} |", "HEADER") };
    // insys::logger logger{ "", "..." };
    // insys::logger logger{};
    logger.enable_debug();

    logger.print("One string: test 1");
    logger.print("test 2");
    logger.println("test 3");

    logger.print("Value: {:.2}", 13.14);
    logger.println("Register: {green+:2x}", 14);
    logger.println("{red:04} {green:4x} {blue:08}", 11, 0x22, 33);
    logger.println("Test {magenta+} test", insys::logger::format("begin {:X} end", 0xF00D));

    string str { "+++" };
    logger.println("string&: {}", str);
    logger.println("char: {}", '*');
    logger.println("pointer: {yellow}", (uint8_t*)str.data());

    logger.println("Unicode String: {yellow}", L"WCHAR test");
    wchar_t test[] = L"WCHAR test";
    auto test_ptr = test;
    logger.println("Unicode String Pointer: {yellow:p}", test_ptr);

    auto arr = std::vector<uint16_t> { 1, 2, 3, 4 };
    logger.println("Array Pointer: {green} Size: {blue}", arr.data(), arr.size());
    logger.print("Array Items:");
    for (auto item : arr)
        logger.print("{green}", item);
    logger.println("");

    // std::cout << log.println("Super Test");
    // std::cout << log.println("begin {:x} {} {} end", uint8_t(0xF0));
    // std::cout << log.println("{} {} {}", true);
    // std::cout << log.println("{} {} {}", 1u, 2ull);
    // std::cout << log.println("{} {} {}", '*', double(0.111), 3l);
    // std::cout << log.println("{} end", 13.1415, 2, 3);
    // std::cout << log.println("begin {} {}", 1, false, 3);
    // std::cout << log.println("Value: {red}, Register: {green:X}", 13.1415, 0x13141516);

    // std::cout << log.print("{magenta}", "String");
    // std::cout << log.print(" {yellow}", "String");
    // std::cout << log.println(" {cyan}", "String");
    // std::cout << log.println("Red {red+}, Green {green+:8X}, Blue {blue+}", 13, 14, 15);
    // std::cout << log.println("{yellow+}, Green {green+}, Blue {blue+}", insys::logger::format("Red: {red+} test", 13), 14, 15);

    // println("Value: {red}, Register: {green:X} end text", 13.1415, 0x13141516);
    // println("Register: {green+:b}", uint16_t(0xF00F));
}
