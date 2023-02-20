#include <hdf5.h>
#include <mpi.h>

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char ** argv) {
    MPI_Init(&argc, &argv);

    auto comm = MPI_COMM_WORLD;
    int comm_rank;
    MPI_Comm_rank(comm, &comm_rank);

    std::string filename = "foo.h5";
    if(comm_rank == 0) {
        auto fcpl = H5P_DEFAULT;
        auto fapl = H5Pcreate(H5P_FILE_ACCESS);

        H5Pset_libver_bounds(fapl, H5F_LIBVER_V110, H5F_LIBVER_V110);
        H5Pset_fapl_split(
            fapl,
            ".meta", H5P_DEFAULT,
            ".raw", H5P_DEFAULT
        );

        hid_t fid = H5Fcreate(filename.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, fapl);

        H5Pclose(fapl);
        H5Fclose(fid);
    }
    MPI_Barrier(comm);

    {
        auto fapl = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_libver_bounds(fapl, H5F_LIBVER_V110, H5F_LIBVER_V110);

        auto mpio_fapl = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_fapl_mpio(mpio_fapl, comm, MPI_INFO_NULL);

        H5Pset_fapl_split(fapl, ".meta", mpio_fapl, ".raw", mpio_fapl);

        hid_t fid = H5Fopen(filename.c_str(), H5F_ACC_RDWR, fapl);

        H5Pclose(mpio_fapl);
        H5Pclose(fapl);
        H5Fclose(fid);
    }


    MPI_Finalize();
    return 0;
}
