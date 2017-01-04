## This is a macrobenchmark to test out the Checkpoint Location Library (CLC)
## function.

## Each MPI rank calls checkpoint() to invoke CLC.

## Before running the simulation check for the following things

1. The program is running for 100 iterations
2. The ssd checkpoint are written with fsync()
3. There are no unneccesary "printf" statements
4. The lifetime, performance decisions are being made correctly
5. The run_me.sh script has the correct parameters
6. The program is compiled. 
7. The ramdisk and SSD directories are empty
