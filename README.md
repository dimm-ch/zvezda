1   установить cmake (3.25 и выше)
2   создать папку build_arm32 (любое другео имя)
3   перейти в эту папку
4   выполнить cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm32-toolchain.cmake .. 
5   выполнить make -j8
6   скомпилированное находится в папке bin

---
изучить содержимое SETUP_FILES
