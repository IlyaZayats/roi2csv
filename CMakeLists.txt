cmake_minimum_required(VERSION 3.23)
project(roi2csv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/usr/local/Cellar/qt@5/5.15.5_2")

find_package(DCMTK REQUIRED)

find_package(Qt5 COMPONENTS
        Core
        REQUIRED)

add_executable(roi2csv main.cpp)

target_link_libraries(roi2csv PRIVATE
        Qt5::Core
        )

include_directories(${DCMTK_INCLUDE_DIRS})
target_link_libraries(roi2csv PRIVATE ${DCMTK_LIBRARIES})