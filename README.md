# heckenbach-strikeslip-modeling-dead-sea-paper

to rerun the models from this paper you will need access to a cluster. Then:

1. compile deal.II 10.0.0-pre (master, 18a3861) available at https://github.com/dealii/dealii

2. compile fastscape from this repository in a build directory using

    cmake -DBUILD_FASTSCAPELIB_SHARED=ON \
    /path/to/fastscape/

    make

3. compile aspect with the following additional flag FASTSCAPE\_DIR pointing to your build folder with the .so file
   further information about how to install aspect can be found here: https://aspect-documentation.readthedocs.io/en/latest/user/install/index.html

    cmake -DFASTSCAPE_DIR=/path/to/fastscape/build \
    -DDEAL_II_DIR=/path/to/dealii \
    -DCMAKE_BUILD_TYPE=Release \
    /path/to/aspect

    make 

4. compile the additional shared library from this repository. You might need to change the paths in CMakeLists.txt
    cmake -DAspect_DIR=/path/to/aspect .
    make
   
5. Run the input file of your choice. Note that these models were run on 3840 MPI processes for about 5 days
