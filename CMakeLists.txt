cmake_minimum_required(VERSION 3.16)

project(net_course_hw_template
    VERSION 1.0.0
    DESCRIPTION "Шаблон домашнего задания по курсу сетевых технологий"
    # CXX включён ТОЛЬКО для компиляции тестов на GoogleTest.
    # Реализация задания — на чистом C.
    LANGUAGES C CXX)

# Стандарты: реализация на C11, тестовая обвязка на C++17, без расширений.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# ---------------------------------------------------------------------------
# Библиотека с кодом студента (чистый C).
#
# Студент реализует логику в src/ только на C. Интерфейсы в include/ и тесты
# в tests/ менять нельзя (защищено CI).
# ---------------------------------------------------------------------------
add_library(hw STATIC
    src/mac.c
    # Список источников фиксирует преподаватель (файл защищён CI).
    # Если студенту разрешены дополнительные .c файлы, добавьте их сюда заранее.
)
target_compile_options(hw PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(hw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ---------------------------------------------------------------------------
# Тесты (GTest). Прогоняются через ctest локально и в CI.
# ---------------------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
  # Сначала пробуем системный GTest (удобно за прокси/офлайн), иначе качаем.
  find_package(GTest 1.14 QUIET)
  if(NOT GTest_FOUND)
    message(STATUS "GTest не найден в системе — скачиваю через FetchContent")
    include(FetchContent)
    FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.15.2
            GIT_SHALLOW    TRUE
        )
    # Для Windows/MSVC: gtest как DLL не собираем.
    set(gtest_force_shared_crt ON CACHE INTERNAL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()

  add_executable(hw_tests
        tests/test_mac.cpp
    )
  target_compile_options(hw_tests PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic>
    )
  target_link_libraries(hw_tests PRIVATE hw GTest::gtest_main)
  target_include_directories(hw_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  include(GoogleTest)
  gtest_discover_tests(hw_tests DISCOVERY_TIMEOUT 60)
endif()
