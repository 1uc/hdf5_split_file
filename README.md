# HDF5 split file with MPI I/O
This repository contains a reproducer for a deadlock that occurs when opening a
split HDF5 with MPI I/O. This reproducer has been extracted from an application
that attempts to first allocate datasets sequentially; and then fills the
datasets in parallel using MPI I/O.

Because the resulting file contains many groups and datasets; and profiling
shows that `H5Gopen` accounts for 40 to 60 percent of the runtime, we'd like to
investigate splitting meta- and raw-data into separate files.

## Dependencies
This requires

  - MPI
  - parallel version of HDF5

Concrete versions tested:
  - ArchLinux system packages: OpenMPI (4.1.4-4) & hdf5-openmpi (1.12.2-3)
  - Spack: `hdf5@1.14.0 +mpi` with `openmpi@4.1.4`.
  - Spack: `hdf5@develop-1.15 +mpi` with `openmpi@4.1.4`.
  - Cluster: `hdf5/1.10.7` with `hpe-mpi/2.25.hmpt`.

## Compiling & Running
Then one can configure the build as follows:

    cmake -DCMAKE_BUILD_TYPE=Debug -B build-debug .

When using Spack:

    cmake -DCMAKE_PREFIX_PATH="$(spack location --install-dir hdf5);$(spack location --install-dir openmpi)" -DCMAKE_BUILD_TYPE=Debug -B build-debug .

and compile:

     cmake --build build-debug/ --parallel

then repeatedly (usually deadlock occurs within 5 attempts):

     rm foo.h5.{meta,raw} ; mpirun -n 4 build-debug/hdf5_fapl_split

## Backtraces
We can obtain backtraces of where the program deadlocks by interrupting the
application in GDB after 10 seconds.

    rm foo.h5.{meta,raw} ; mpirun -n 4 xterm -e gdb build-debug/hdf5_fapl_split

Some MPI ranks gets stuck at:

    (gdb) bt
    #0  0x00007ffff4d9bfb4 in ompi_coll_libnbc_progress () from /usr/lib/openmpi/mca_coll_libnbc.so
    #1  0x00007ffff7865724 in opal_progress () from /usr/lib/libopen-pal.so.40
    #2  0x00007ffff7ec016e in ompi_request_default_wait_all () from /usr/lib/libmpi.so.40
    #3  0x00007ffff7f2a641 in ompi_coll_base_barrier_intra_basic_linear () from /usr/lib/libmpi.so.40
    #4  0x00007ffff5395bdd in mca_io_ompio_file_sync () from /usr/lib/openmpi/mca_io_ompio.so
    #5  0x00007ffff7edf8e9 in PMPI_File_sync () from /usr/lib/libmpi.so.40
    #6  0x00007ffff7d2e13b in ?? () from /usr/lib/libhdf5.so.200
    #7  0x00007ffff7b20e73 in H5FD_flush () from /usr/lib/libhdf5.so.200
    #8  0x00007ffff7b20f86 in H5FDflush () from /usr/lib/libhdf5.so.200
    #9  0x00007ffff7b2b5df in ?? () from /usr/lib/libhdf5.so.200
    #10 0x00007ffff7b20e73 in H5FD_flush () from /usr/lib/libhdf5.so.200
    #11 0x00007ffff7b0ef34 in H5F_flush_tagged_metadata () from /usr/lib/libhdf5.so.200
    #12 0x00007ffff7b0b889 in H5F_open () from /usr/lib/libhdf5.so.200
    #13 0x00007ffff7cfd308 in H5VL__native_file_open () from /usr/lib/libhdf5.so.200
    #14 0x00007ffff7ce16d1 in ?? () from /usr/lib/libhdf5.so.200
    #15 0x00007ffff7ceeae7 in H5VL_file_open () from /usr/lib/libhdf5.so.200
    #16 0x00007ffff7b0042c in H5Fopen () from /usr/lib/libhdf5.so.200
    #17 0x0000555555556569 in main (argc=1, argv=0x7fffffffd4c8) at /home/lucg/git/hdf5_split_file/src/hdf5_fapl_split_main.cpp:43


Other MPI ranks get stuck at (or similar):

    (gdb) bt
    #0  0x00007ffff7865710 in opal_progress () from /usr/lib/libopen-pal.so.40
    #1  0x00007ffff4de3c06 in mca_pml_ob1_recv () from /usr/lib/openmpi/mca_pml_ob1.so
    #2  0x00007ffff7f2a524 in ompi_coll_base_barrier_intra_basic_linear () from /usr/lib/libmpi.so.40
    #3  0x00007ffff4049958 in mca_sharedfp_sm_file_close () from /usr/lib/openmpi/mca_sharedfp_sm.so
    #4  0x00007ffff4d76647 in mca_common_ompio_file_close () from /usr/lib/libmca_common_ompio.so.41
    #5  0x00007ffff53997a1 in mca_io_ompio_file_close () from /usr/lib/openmpi/mca_io_ompio.so
    #6  0x00007ffff7eb63fe in ?? () from /usr/lib/libmpi.so.40
    #7  0x00007ffff7eb7031 in ompi_file_close () from /usr/lib/libmpi.so.40
    #8  0x00007ffff7ee2a9a in PMPI_File_close () from /usr/lib/libmpi.so.40
    #9  0x00007ffff7d2af07 in ?? () from /usr/lib/libhdf5.so.200
    #10 0x00007ffff7b1f836 in H5FD_close () from /usr/lib/libhdf5.so.200
    #11 0x00007ffff7b1f948 in H5FDclose () from /usr/lib/libhdf5.so.200
    #12 0x00007ffff7b2b01e in ?? () from /usr/lib/libhdf5.so.200
    #13 0x00007ffff7b1f836 in H5FD_close () from /usr/lib/libhdf5.so.200
    #14 0x00007ffff7b088b2 in ?? () from /usr/lib/libhdf5.so.200
    #15 0x00007ffff7b0b37a in H5F_open () from /usr/lib/libhdf5.so.200
    #16 0x00007ffff7cfd308 in H5VL__native_file_open () from /usr/lib/libhdf5.so.200
    #17 0x00007ffff7ce16d1 in ?? () from /usr/lib/libhdf5.so.200
    #18 0x00007ffff7ceeae7 in H5VL_file_open () from /usr/lib/libhdf5.so.200
    #19 0x00007ffff7b0042c in H5Fopen () from /usr/lib/libhdf5.so.200
    #20 0x0000555555556569 in main (argc=1, argv=0x7fffffffd4c8) at /home/lucg/git/hdf5_split_file/src/hdf5_fapl_split_main.cpp:43

Running the experiment three times prints the same backtraces. In the first run
only one MPI rank was stuck in the first backtrace. In the following to runs
two MPI ranks were stuck in the first backtrace.

