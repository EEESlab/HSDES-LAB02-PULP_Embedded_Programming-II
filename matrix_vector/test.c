/*
 * Copyright (C) 2020 University of Bologna
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Authors: Manuele Rusci, UniBO (manuele.rusci@unibo.it)
 *          Leonardo Ravaglia, UniBO (leonardo.ravaglia2@unibo.it)
 */
#include "pmsis.h"

/*  defines */
#define N 50
#define M 50
#define MAT_EL (2)  // matrix constant values
#define VEC_EL (4)  // vector constant values


/* 
    Allocation of IO variables into L2 memory
    Note: PI_L2 is an attribute for "forcing" allocation in L2 memory
    /pulp/pulp-sdk/rtos/pulpos/common/include/pos/data/data.h:54: 
       #define PI_L2 __attribute__((section(".l2_data")))
*/
// input variables
PI_L2 int matrix[N*M]; // the matrix as an array of size N*M
PI_L2 int vector[M];
// output variable
PI_L2 int output_vec[N];  // N*M x M*1 -> N*1

/* 
    generic matrix-vector multiplication
*/
int __attribute__((noinline)) gemv( int size_N, int size_M, int* mat_i, int*vec_i, int* vec_o ){
    for (int i=0; i<size_N; i++){
      for (int j=0; j<size_M; j++){
          // multiply accumulate operation (MAC)
          vec_o[i] += mat_i[i*size_M+j] * vec_i[j];
          //*(vec_o+i) += *(mat_i+i*M+j)*(*(vec_i+j)); // try to uncomment this and comment the above line
      }
    }
}


int main()
{
  // Initialization of operands and reset the ouput
  for (int i=0; i<(N*M); i++) {
    matrix[i] = MAT_EL;
  }
  for (int i=0; i<M; i++) {
    vector[i] = VEC_EL;
  }
  for (int i=0; i<N; i++) {
    output_vec[i] = 0;
  }

  // call the matrix-vector fucntion

  // enable the perf counters of interest
  pi_perf_conf(  	1 << PI_PERF_CYCLES | 
        1 << PI_PERF_INSTR ); 

  // reset the performance counters
  pi_perf_reset(); 
  // start the performance counters
  pi_perf_start(); 

  // task to profile
  gemv(N, M, matrix, vector, output_vec);

  // stop the performance counters
  pi_perf_stop(); 

  // collect and print statistics
  int mac = N*M;
  uint32_t instr_cnt = pi_perf_read(PI_PERF_INSTR);
  uint32_t cycles_cnt = pi_perf_read(PI_PERF_CYCLES);
  float CPI = (float) cycles_cnt / (float) instr_cnt;
  float cyc_mac = (float) cycles_cnt / (float) mac;
  float inst_mac = (float) instr_cnt / (float) mac;

  printf("Number of instr = %d and num of clk cyc = %d\n", 
            instr_cnt, cycles_cnt);
  printf("CPI = %f\n", CPI);
  printf("Number of instr/MAC = %f and num of clk cyc / MAC = %f\n", 
        inst_mac, cyc_mac);


  // print and check the results
  printf("The %d output elements are: ", N);
  for (int i=0; i<N; i++) {
    printf("%d, ", output_vec[i]);
  }
  printf("\n");

  // check here the results
  int correctness = 1;
  for (int i=0; i<N; i++) {
    if ( output_vec[i] != (M*MAT_EL*VEC_EL)) 
    {
      correctness = 0;
      break;
    }
  }
  printf( correctness ? 
          "Results OK\n" : 
          "Results NOT OK\n"
      );
}
