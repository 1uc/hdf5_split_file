# hi5_split_file

## Dependencies
This requires

  - MPI
  - parallel version of HDF5

Concrete versions tested:
  - ArchLinux system packages: OpenMPI (4.1.4-4) & hdf5-openmpi (1.12.2-3)
  - Spack: `hdf5@1.14.0 +mpi` with `openmpi@4.1.4`.
  - Spack: `hdf5@develop-1.15 +mpi` with `openmpi@4.1.4`.

## Compiling & Running
Then one can configure the build as follows:

    cmake -DCMAKE_BUILD_TYPE=Debug -B build-debug .

When using Spack:

    cmake -DCMAKE_PREFIX_PATH="$(spack location --install-dir hdf5);$(spack location --install-dir openmpi)" -DCMAKE_BUILD_TYPE=Debug -B build-debug .

and compile:

     cmake --build build-debug/ --parallel

then repeatedly (usually deadlock occurs within 5 attempts):

     rm foo.h5.{meta,raw} ; mpirun -n 4 build-debug/hdf5_fapl_split
