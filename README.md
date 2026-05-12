# Test_Yadro_Telecom
Yadro Telecom test task

## How to build

1. Before building on you system must be installed Boost C++ libraries headers. To install on Linux use `sudo apt install libboost-dev` 
2. Copy the repository
3. Go to the source tree directory (the one containing `CMakeLists.txt` file)
4. Generate a project buildsystem, then build the project through terminal. On linux just use

   `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build ./build`

   command.
   
5. Wait till the building is finished.

## How to test

1. In terminal use `ctest --test-dir build/tests` command.
2. See the results of the test.

## How to run

1. In the same directory use `build/main`.
2. Enjoy the program.