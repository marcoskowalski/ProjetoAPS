cmake_minimum_required(VERSION 3.21)
project(BookNest VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# --- Núcleo de regras (C++ puro, sem Qt) + testes -------------------------------------
add_library(booknest_core STATIC src/core/ReadingRules.cpp)
target_include_directories(booknest_core PUBLIC src)

# Rastreamento opcional das chamadas (confere a ordem dos diagramas de sequência). Desligado por padrão.
# Para ligar: CMake options  -DBOOKNEST_TRACE=ON  (escreve no console e em trace.log)
option(BOOKNEST_TRACE "Escrever o rastreamento das chamadas (Classe::metodo)" OFF)
if(BOOKNEST_TRACE)
    target_compile_definitions(booknest_core PUBLIC BOOKNEST_TRACE)
endif()

enable_testing()
add_executable(test_core tests/test_core.cpp)
target_link_libraries(test_core PRIVATE booknest_core)
add_test(NAME core_rules COMMAND test_core)

# --- Aplicação Qt 6 --------------------------------------------------------------------
# No CLion: Settings > Build > CMake > CMake options:
#   -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64     (ajuste para a sua instalação do Qt)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network)

add_executable(BookNest WIN32
    src/main.cpp
    src/db/DatabaseManager.cpp
    src/db/LivroRepository.cpp
    src/db/LeituraRepository.cpp
    src/db/ListaRepository.cpp
    src/db/EstatisticaRepository.cpp
    src/net/GoogleBooksClient.cpp
    src/ui/MainWindow.cpp
    src/ui/BookDialog.cpp
    src/ui/ProgressDialog.cpp
    src/ui/ReviewDialog.cpp
    src/ui/ListasDialog.cpp
    src/ui/EstatisticasDialog.cpp
    src/ui/GoogleBooksDialog.cpp
    resources/resources.qrc)
target_include_directories(BookNest PRIVATE src)
target_link_libraries(BookNest PRIVATE booknest_core Qt6::Widgets Qt6::Sql Qt6::Network)
