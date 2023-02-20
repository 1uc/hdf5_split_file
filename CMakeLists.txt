cmake_minimum_required(VERSION 3.13.0)
project(SplitFile VERSION 0.1.0)

# Find all dependencies
find_package(MPI REQUIRED)
find_package(HDF5 REQUIRED)

# HDF5 reproducer ---------------------------------------------------------------
add_executable(hdf5_fapl_split
  "${CMAKE_CURRENT_SOURCE_DIR}/src/hdf5_fapl_split_main.cpp"
)
target_include_directories(hdf5_fapl_split PUBLIC ${HDF5_INCLUDE_DIRS})
target_link_libraries(hdf5_fapl_split PUBLIC ${HDF5_LIBRARIES} MPI::MPI_C)
