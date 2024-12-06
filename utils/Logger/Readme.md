## Библиотека для вывода цветных форматированных логов

Подключение через Cmake:
```
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Logger)
target_link_libraries(${PROJECT_NAME} logger)
```

Пример использования:
```cpp
#include <logger.hpp>

insys::logger logger { insys::logger::format("{white+} |", "HEADER") };
logger.enable_debug();

logger.print("Value: {:.2}", 13.14);
logger.println("Register: {green+:2x}", 14);
logger.println("{red:04} {green:4x} {blue:08}", 11, 0x22, 33);
logger.println("Test {magenta+} test", insys::logger::format("begin {:x} end", 0xF00D));
```

```
HEADER | Value: 13.14 Register: 0x0E
HEADER | 0011 0x0022 00000033
HEADER | Test begin 0xF00D end test
```
