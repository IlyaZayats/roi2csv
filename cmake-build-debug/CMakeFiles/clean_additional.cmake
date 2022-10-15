# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/roi2csv_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/roi2csv_autogen.dir/ParseCache.txt"
  "roi2csv_autogen"
  )
endif()
