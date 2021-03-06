#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <tuple>

typedef char 	BYTE;

#define MB      1024*1024
enum Location {TO_RAM, TO_SSD};

// Global variables for checkpointing
double ssd_lifetime_rating_in_pbw = 14.6;
double ssd_sequential_bandwidth_mBps = 250;
double ssd_guaranteed_years = 5;
int fd;

double lifetimePrediction(double PBw, double used_pcnt, double size_MB, double interval);
Location lifetimeEstimation(const double mbw_used, const long int chk_bytes,
        const double time_now, const double last_ssd_checkpoint_made_at); 
Location performanceEstimation(const double time_now, const double time_spent_checkpointing, 
        const double start_time);


//****************************************************************************80
// Using variadic templates, define a tuple
// that can have an arbitrary number of fields.

// the empty tuple - recursive defintion will
// stop here when there are no more types to peel off
template <class...>
struct write_file {};

template <>
struct write_file<std::tuple<>, std::tuple<> >
{
    void operator()() {}
};

// recursive definition
// template to write many data blocks. It will peel off T1 and T2 arguments
// (size and data), and then expand Ts argument until there are only two remaining.
// The last two arguments will go to the above write_file()
template <class T1, class T2, class... Size, class... Data>
struct write_file< std::tuple<T1, Size...>, std::tuple<T2*, Data...> >
{
	void operator()(T1 bytes, Size... s, T2* data, Data... d)
	{
		ssize_t success = write(fd, (void*)data, bytes);
        if (success == 0)
            ;
        auto impl = write_file< std::tuple<Size...>, std::tuple<Data...> >();
        impl(s..., d...);
	}
};
//****************************************************************************80

//****************************************************************************80
// Using variadic template to add up the sizes of all the data blocks to be
// checkpointed
unsigned addBytes() {return 0;}

template <class T, class... Ts>
unsigned addBytes(const T& head, const Ts&... tail)
{
    return head + addBytes(tail... );
}
//****************************************************************************80


template <class... Size, class... Data>
void checkpoint(int id, double start_time, double time_spent_checkpointing, 
        double &mbw_used, double &last_ssd_checkpoint_made_at, 
        int iteration, double time_now,
        Size... s, Data... d)

//****************************************************************************80
//
// Purpose: This is the checkpoint phase
// It checkpoints the data that was generated by GenerateMemoryBlock()
//
{
    // default selection is to checkpoint to the ramdisk
    Location location = TO_RAM;
    
    long int chk_bytes = addBytes(s...);
    Location life_location = lifetimeEstimation(mbw_used, chk_bytes, time_now, last_ssd_checkpoint_made_at);
    Location perf_location = performanceEstimation(time_now, time_spent_checkpointing, start_time);
    //Location perf_location = TO_SSD;
    Location size_location = (chk_bytes*8 > 4096) ? TO_SSD : TO_RAM;

    if (life_location == TO_SSD)
    {
        if (perf_location == TO_SSD)        location = TO_SSD;
        else if (perf_location == TO_RAM)   location = TO_RAM;
    }
    else if (life_location == TO_RAM)
        location = TO_RAM;

    //if ((location == TO_RAM) && (size_location == TO_SSD))
    //{
    //    printf("%d: choosing to skip\n", id);
    //    return;
    //}

    //if ((iteration % 10) == 0)
    //{
    //    location = TO_SSD;
    //} else 
    //{
    //    location = TO_RAM;
    //}

    // Override - always write to the RAM
    //location = TO_SSD;

    // Write the data to a file
    char filename[100];
    if (location == TO_RAM)
    {
        printf("%d %lf: choosing ramdisk\n", id, time_now);
        sprintf(filename, "/mnt/ramdisk/checkpoint_test_%ld_%d.txt", chk_bytes, id);

        fd = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
        auto impl = write_file< std::tuple<Size...>, std::tuple<Data...> >();
        impl(s..., d...);
        //lseek(fd, 0, SEEK_SET); /* seek to start of file */
        //impl(s..., d...);
        fsync(fd);
        close(fd);
    } 
    else if (location == TO_SSD)
    {
        printf("%d %lf: choosing ssd\n", id, time_now);
        sprintf(filename, "/home/nilmini/work/research/hpc_benchmarks/dram_chkpnt/chkpnt_dir/checkpoint_%ld_%d.txt", chk_bytes, id);
        last_ssd_checkpoint_made_at = time_now;
        mbw_used += (chk_bytes*8)/(float)(1024*1024);

        fd = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
        auto impl = write_file< std::tuple<Size...>, std::tuple<Data...> >();
        impl(s..., d...);
        fsync(fd);
        close(fd);
    }
    return;
}
//****************************************************************************80

Location lifetimeEstimation(const double mbw_used, const long int chk_bytes,
        const double time_now, const double last_ssd_checkpoint_made_at) 

//****************************************************************************80
{
    // Condition #1: Lifetime.
    // Decide to checkpoint to the SSD if it won't be prematurely worn out.
    double pbw_used = mbw_used/double(1024*1024*1024);
    //printf("pbw_used %lf\n", pbw_used);
    double expected_years = (ssd_lifetime_rating_in_pbw - pbw_used)*
        (ssd_guaranteed_years/ssd_lifetime_rating_in_pbw);
    //printf("expected_years %lf \n", expected_years);
    double used_pcnt = pbw_used/ssd_lifetime_rating_in_pbw;
    //printf("used_pcnt %lf \n", used_pcnt);
    //printf("last_ssd_checkpoint_made_at %lf\n", last_ssd_checkpoint_made_at);
    double chkpnt_interval = time_now - last_ssd_checkpoint_made_at;
    double chkpnt_size_MB = (chk_bytes*8)/(float)(1024*1024);
    //printf("chkpnt_size_MB %lf chkpnt_interval %lf\n", chkpnt_size_MB, chkpnt_interval);
    double predicted_years = lifetimePrediction(ssd_lifetime_rating_in_pbw, used_pcnt, 
            chkpnt_size_MB, chkpnt_interval);
    //printf("predicted_years %lf\n", predicted_years);

    if (predicted_years > expected_years)
    {
        //printf("Predicted years > expected years, can checkpoint to SSD\n");
        return TO_SSD;
    }

    return TO_RAM;
}
//****************************************************************************80

Location performanceEstimation(const double time_now, const double time_spent_checkpointing, 
        const double start_time)

//****************************************************************************80
{
    // Condition #2: Performance loss
    // At this point, condition #1 may have decided to checkpoint to the SSD.
    // We should determine if this will result in a performance loss, and
    // revert the decision back to the ramdisk
    double time_elapsed = time_now - start_time;
    if ((time_spent_checkpointing/time_elapsed) < 0.1)
        return TO_SSD;

    return TO_RAM;
}
//****************************************************************************80

double lifetimePrediction(double PBw, double used_pcnt, double size_MB, double interval)

//****************************************************************************80
{
    int SECS_IN_YEAR = 3600*24*365;
    double MBw = (PBw*1e9)*(1-used_pcnt);
    double bw_per_year = (size_MB/interval)*SECS_IN_YEAR;
    double lifetime = MBw/bw_per_year;
    return lifetime;
}
//****************************************************************************80

#endif
