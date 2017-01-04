/* This is a mock HPC application 
 * This mock HPC application uses a program called satisfy_mpi.cpp released by
 * John Burkardt from Florida State University. 
 * 
 * This application does 2 things: compute and checkpoint.
 * It computes for a selected time period, about 5 seconds, 
 * and then make a checkpoint of the size specified by the user.
 *
 * The user can specify the size of the checkpoint to be made in Megabytes. The
 * application will computer for 5 seconds, and then write a checkpoint of the
 * specified size to the specified checkpointing location. This process will
 * repeat for 100 iterations, meaning 100 checkpoints will be made. The
 * interval between two checkpoints is 5 seconds. 
 */

# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <ctime>
#include <chrono> // compile with -std=c++11

# include <mpi.h>
#include "checkpoint_library.h"

#define MB      1024*1024
typedef double  real8;
typedef real8   Real_t; // floating-point representation
typedef char 	BYTE;

using namespace std;

int main ( int argc, char *argv[] );
BYTE* GenerateMemoryBlock(long int size);
void compute(int n, int id, int ilo2, int ihi2, int bvec[], int* solution_num);
int circuit_value ( int n, int bvec[] );
void i4_to_bvec ( int i4, int n, int bvec[] );
void timestamp ( );

//****************************************************************************80

int main ( int argc, char *argv[] )

//****************************************************************************80
//
//  Purpose:
//
//    MAIN is the main program for SATISFY_MPI.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    21 March 2009
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    Michael Quinn,
//    Parallel Programming in C with MPI and OpenMP,
//    McGraw-Hill, 2004,
//    ISBN13: 978-0071232654,
//    LC: QA76.73.C15.Q55.
//
{
# define N 23

  int bvec[N];
  int i;
  int id;
  int ihi;
  int ihi2;
  int ilo;
  int ilo2;
  int j;
  int n = N;
  int p;
  int solution_num;

  // Variables added for checkpointing
  double time_spent_checkpointing = 0.0;
  double mbw_used = 0.0;
  double last_ssd_checkpoint_made_at = 0.0;

//
//  Initialize MPI.
//
  MPI::Init ( argc, argv );
//
//  Determine the rank of this processor.
//
  id = MPI::COMM_WORLD.Get_rank ( );
//
//  Determine the number of processors.
//
  p = MPI::COMM_WORLD.Get_size ( );
//
//  Let process 0 print the opening remarks.
//
  if ( id == 0 ) 
  {
    cout << "\n";
    timestamp ( );
    cout << "\n";
    cout << "SATISFY_MPI\n";
    cout << "  C++/MPI version\n";
    cout << "  We have a logical function of N logical arguments.\n";
    cout << "  We do an exhaustive search of all 2^N possibilities,\n";
    cout << "  seeking those inputs that make the function TRUE.\n";
  }
  using std::chrono::high_resolution_clock;
  using std::chrono::milliseconds;
  auto t0 = high_resolution_clock::now();

  double start_time = MPI_Wtime();
//
//  The BIG calculation goes from 0 = ILO <= I < IHI = 2*N.
//  Compute the upper limit.
//
  ilo = 0;

  ihi = 1;
  for ( i = 1; i <= n; i++ )
  {
    ihi = ihi * 2;
  }

  if ( id == 0 )
  {
    cout << "\n";
    cout << "  The number of logical variables is N = " << n << "\n";;
    cout << "  The number of input vectors to check is " << ihi << "\n";
    cout << "\n";
    cout << "   # Processor       Index    ---------Input Values------------------------\n";
  }
//
//  Processor ID takes the interval ILO2 <= I < IHI2.
//  Using the formulas below yields a set of nonintersecting intervals
//  which cover the original interval [ILO,IHI).
//
  ilo2 = ( ( p - id     ) * ilo 
         + (     id     ) * ihi ) 
         / ( p          );

  ihi2 = ( ( p - id - 1 ) * ilo 
         + (     id + 1 ) * ihi ) 
         / ( p          );

  cout << "\n";
  cout << "  Processor " << id << " iterates from " << ilo2 << " <= I < " << ihi2 << "\n";

// Generate data
  long int chk_bytes = atoi(argv[1]);
// Figure out the size of the checkpoint
  printf("Generating a random data set of size: %ld\n", chk_bytes*MB);
  BYTE* pData = GenerateMemoryBlock(chk_bytes*MB);
  MPI::COMM_WORLD.Barrier();

  last_ssd_checkpoint_made_at = start_time;
  for ( int i = 0; i < 10; i++)
  {
      cout << "Iteration: " << i << endl;
      double compute_time = 0.0;
      do 
      {
        double begin_time = MPI::Wtime();
        compute(n, id, ilo2, ihi2, bvec, &solution_num);
        compute_time += MPI::Wtime() - begin_time;
      } while (compute_time < 5.0);
      MPI::COMM_WORLD.Barrier();

      // Checkpoint
      double clock_before_chkpnt = MPI_Wtime();
      //checkpoint<long int, long int>(id, start_time, time_spent_checkpointing, 
      //        mbw_used, last_ssd_checkpoint_made_at, i, MPI_Wtime(), chk_bytes*MB, chk_bytes*MB, pData, pData);
      checkpoint<long int>(id, start_time, time_spent_checkpointing, 
              mbw_used, last_ssd_checkpoint_made_at, i, MPI_Wtime(), chk_bytes*MB, pData);
      time_spent_checkpointing += MPI_Wtime() - clock_before_chkpnt;
  }
  printf("Petabytes used up: %.10lf\n", mbw_used/double(1024*1024*1024));

// Remove allocated memory
  delete pData;
  cout << std::flush;
//
//  Terminate MPI.
//
  MPI::Finalize ( );

  auto t1 = high_resolution_clock::now();
  milliseconds total_ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
  printf("The application took %ld ms\n", total_ms.count() );

//
//  Terminate.
//
  if ( id == 0 )
  {
    cout << "\n";
    cout << "SATISFY_MPI\n";
    cout << "  Normal end of execution.\n";
    cout << "\n";
    timestamp ( );
  }
  return 0;
# undef N
}
//****************************************************************************80

BYTE* GenerateMemoryBlock(long int size)

//****************************************************************************80
//
// Purpose: Generate checkpoint data. This data has nothing to do with what's
// being computed. This is mock data. 
{
    BYTE* pBytes = new BYTE[size];

    // Fill random value
    srand(255);

    for (int i = 0; i < size; i++)
    {
        pBytes[i] = (BYTE)rand();
    }

    return pBytes;
}
//****************************************************************************80

void compute(int n, int id, int ilo2, int ihi2, int bvec[], int* solution_num)

//****************************************************************************80
//
// Purpose: This is the compute phase
//
{
//
//  Check if BVEC is a solution.  Then "increment" BVEC.
//
  int value;
  int solution_num_local;
  int i, j;

  solution_num_local = 0;

  for ( i = ilo2; i < ihi2; i++ )
  {
    i4_to_bvec ( i, n, bvec );

    value = circuit_value ( n, bvec );

    //if (value == 1 )
    //{
    //  solution_num_local = solution_num_local + 1;

    //  cout << "  " << setw(2) << solution_num_local
    //       << "  " << setw(8) << id
    //       << "  " << setw(10) << i;

    //  for ( j = 0; j < n; j++ )
    //  {
    //    cout << " " << bvec[j];
    //  }
    //  cout << "\n";
    //}
  }
//
//  Process 0 gathers the local solution totals.
//
  //MPI::COMM_WORLD.Reduce ( &solution_num_local, &solution_num, 1, MPI::INT, MPI::SUM, 0 );
  return;
}
//****************************************************************************80

int circuit_value ( int n, int bvec[] )

//****************************************************************************80
//
//  Purpose:
//
//    CIRCUIT_VALUE returns the value of a circuit for a given input set.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    20 March 2009
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    Michael Quinn,
//    Parallel Programming in C with MPI and OpenMP,
//    McGraw-Hill, 2004,
//    ISBN13: 978-0071232654,
//    LC: QA76.73.C15.Q55.
//
//  Parameters:
//
//    Input, int N, the length of the input vector.
//
//    Input, int BVEC[N], the binary inputs.
//
//    Output, int CIRCUIT_VALUE, the output of the circuit.
//
{
  int value;

  value = 
       (  bvec[0]  ||  bvec[1]  )
    && ( !bvec[1]  || !bvec[3]  )
    && (  bvec[2]  ||  bvec[3]  )
    && ( !bvec[3]  || !bvec[4]  )
    && (  bvec[4]  || !bvec[5]  )
    && (  bvec[5]  || !bvec[6]  )
    && (  bvec[5]  ||  bvec[6]  )
    && (  bvec[6]  || !bvec[15] )
    && (  bvec[7]  || !bvec[8]  )
    && ( !bvec[7]  || !bvec[13] )
    && (  bvec[8]  ||  bvec[9]  )
    && (  bvec[8]  || !bvec[9]  )
    && ( !bvec[9]  || !bvec[10] )
    && (  bvec[9]  ||  bvec[11] )
    && (  bvec[10] ||  bvec[11] )
    && (  bvec[12] ||  bvec[13] )
    && (  bvec[13] || !bvec[14] )
    && (  bvec[14] ||  bvec[15] )
    && (  bvec[14] ||  bvec[16] )
    && (  bvec[17] ||  bvec[1]  )
    && (  bvec[18] || !bvec[0]  )
    && (  bvec[19] ||  bvec[1]  )
    && (  bvec[19] || !bvec[18] )
    && ( !bvec[19] || !bvec[9]  )
    && (  bvec[0]  ||  bvec[17] )
    && ( !bvec[1]  ||  bvec[20] )
    && ( !bvec[21] ||  bvec[20] )
    && ( !bvec[22] ||  bvec[20] )
    && ( !bvec[21] || !bvec[20] )
    && (  bvec[22] || !bvec[20] );

  return value;
}
//****************************************************************************80

void i4_to_bvec ( int i4, int n, int bvec[] )

//****************************************************************************80
//
//  Purpose:
//
//    I4_TO_BVEC converts an integer into a binary vector.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    20 March 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int I4, the integer.
//
//    Input, int N, the dimension of the vector.
//
//    Output, int BVEC[N], the vector of binary remainders.
//
{
  int i;

  for ( i = n - 1; 0 <= i; i-- )
  {
    bvec[i] = i4 % 2;
    i4 = i4 / 2;
  }

  return;
}
//****************************************************************************80

void timestamp ( )

//****************************************************************************80
//
//  Purpose:
//
//    TIMESTAMP prints the current YMDHMS date as a time stamp.
//
//  Example:
//
//    31 May 2001 09:45:54 AM
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    24 September 2003
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    None
//
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  size_t len;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  len = strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  cout << time_buffer << "\n";

  return;
# undef TIME_SIZE
}
