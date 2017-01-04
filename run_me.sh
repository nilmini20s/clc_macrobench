#!/bin/bash

########################
### Write a file of size $i using the arbitrary_memcpy.c program
########################

#for (( i=1150; i <= 1500; i+=50 ))
#do 
#    ./a.out <<< $i >> output.txt
#    sleep 5 
#done

########################
### Copy the following files from ramdisk to the SSD.
########################

#for (( i=850; i <=1100; i+=50 ))
#do
#    t0=`date +%s%3N`
#    cp /mnt/ramdisk/checkpoint_test_$i.txt /mnt/ssd/checkpoint_files/
#    t1=`date +%s%3N`
#    echo "$((t1 - t0))"
#    sleep 60
#done

# satisfy stuff
#HOSTS="simpool,m60-002,m60-004,m60-005,m60-006,m60-007,m60-008,m60-009"
#mpirun -H $HOSTS -np 64 satisfy 50

#for (( i=100; i <= 1500; i+=100 ))
#do
#    echo "mpirun -H $HOSTS -np 64 satisfy_nochkpnt $i >> stdout_nochkpnt_$i.txt"
#    mpirun -H $HOSTS -np 64 satisfy_nochkpnt $i >> stdout_nochkpnt_$i.txt
#    sleep 5
#done

#mpic++ -std=c++11 -o satisfy_ssd_chkpnt satisfy_mpi.cpp checkpoint_library.h 
#
#for (( i=100; i <= 1000; i+=100 ))
#do
#    echo "mpirun -np 8 satisfy_ssd_chkpnt $i >> stdout_ssd_chkpnt_$i.txt"
#    mpirun -np 8 satisfy_ssd_chkpnt $i >> stdout_ssd_chkpnt_$i.txt
#    rm -f /mnt/ssd/checkpoint_files/*
#    sleep 60
#done

#===================#
# Always RAM
#===================#

#for (( i=100; i <= 900; i+=100 ))
#do
#    echo "mpirun -np 8 satisfy_ram_chkpnt $i >> stdout_ram_chkpnt_$i.txt"
#    mpirun -np 8 satisfy_ram_chkpnt $i >> stdout_ram_chkpnt_$i.txt
#    rm -f /mnt/ramdisk/*
#    sleep 30 
#done
#mpirun -np 8 satisfy_ram_chkpnt 950 >> stdout_ram_chkpnt_950.txt
#rm -f /mnt/ramdisk/*


#===================#
# Lifetime Estimation
#===================#

#for (( i=100; i <= 900; i+=100 ))
#do
#    echo "mpirun -np 8 satisfy_life_chkpnt $i >> stdout_life_chkpnt_$i.txt"
#    mpirun -np 8 satisfy_life_chkpnt $i >> stdout_life_chkpnt_$i.txt
#    rm -f /mnt/ssd/checkpoint_files/*
#    rm -f /mnt/ramdisk/*
#    sleep 30 
#done
#mpirun -np 8 satisfy_life_chkpnt 950 >> stdout_life_chkpnt_950.txt
#rm -f /mnt/ssd/checkpoint_files/*
#rm -f /mnt/ramdisk/*


#===================#
# Performance Loss Estimation
#===================#

for (( i=600; i <= 900; i+=100 ))
do
    echo "mpirun -np 8 satisfy_perf10_chkpnt $i >> stdout_perf10_chkpnt_$i.txt"
    mpirun -np 8 satisfy_perf10_chkpnt $i >> stdout_perf10_chkpnt_$i.txt
    rm -f /mnt/ssd/checkpoint_files/*
    rm -f /mnt/ramdisk/*
    sleep 30 
done
mpirun -np 8 satisfy_perf10_chkpnt 950 >> stdout_perf10_chkpnt_950.txt
rm -f /mnt/ssd/checkpoint_files/*
rm -f /mnt/ramdisk/*

#===================#
# Size Estimation: With both lifetime estimation and performance estimation
# enabled
#===================#

#for (( i=100; i <= 900; i+=100 ))
#do
#    echo "mpirun -np 8 satisfy_size_perf10_chkpnt $i >> stdout_size_perf10_chkpnt_$i.txt"
#    mpirun -np 8 satisfy_size_perf10_chkpnt $i >> stdout_size_perf10_chkpnt_$i.txt
#    rm -f /mnt/ssd/checkpoint_files/*
#    rm -f /mnt/ramdisk/*
#    sleep 30 
#done
#mpirun -np 8 satisfy_size_perf10_chkpnt 950 >> stdout_size_perf10_chkpnt_950.txt
#rm -f /mnt/ssd/checkpoint_files/*
#rm -f /mnt/ramdisk/*

#===================#
# Fixed 10
#===================#

#for (( i=100; i <= 900; i+=100 ))
#do
#    echo "mpirun -np 8 satisfy_fixed10_chkpnt $i >> stdout_fixed10_chkpnt_$i.txt"
#    mpirun -np 8 satisfy_fixed10_chkpnt $i >> stdout_fixed10_chkpnt_$i.txt
#    rm -f /mnt/ssd/checkpoint_files/*
#    rm -f /mnt/ramdisk/*
#    sleep 30 
#done
#mpirun -np 8 satisfy_fixed10_chkpnt 950 >> stdout_fixed10_chkpnt_950.txt
#rm -f /mnt/ssd/checkpoint_files/*
#rm -f /mnt/ramdisk/*
