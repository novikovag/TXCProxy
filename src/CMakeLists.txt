# CMakeList.txt: проект CMake для txcproxy; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (txcproxy  txcproxy.h master.c server.c txcproxy.c)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
