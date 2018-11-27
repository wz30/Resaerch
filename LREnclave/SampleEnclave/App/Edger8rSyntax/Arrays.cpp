/*
 * Copyright (C) 2011-2018 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include "../App.h"
#include "Enclave_u.h"
#include <iostream>
#include <math.h>
#include <fstream>
using namespace std;
/* edger8r_lr
 *   Invokes ECALLs declared with logistic regression.
 */
void edger8r_lr(int N, int M)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    ////////////////////////////////
    //read the data
    ifstream inputFileX;
    inputFileX.open("ex4x.dat");
    ifstream inputFileY;
    inputFileY.open("ex4y.dat");
    cout << "reading data" << endl;
    float* x;
    float* y;
    int rows = N;
    if (!inputFileX)
        cout << "Error opening file!" << endl;
    else
    {
        // determines number of items in input file.
        x = new float  [rows * (M+1)]; // declare 2D array for data in file. // 33!
        y=  new float [rows];
        // write data into 2D array
        for (int indexrow=0; indexrow<rows; indexrow++)
        {
            x[indexrow*(M+1) + 0] = 1;
            cout << x[indexrow * (M+1) + 0] << " ";
            for (int indexcol=1; indexcol< M+1;indexcol++)
            {
                inputFileX >> x[indexrow * (M+1) + indexcol];
                cout << x[indexrow * (M+1) + indexcol] << " ";
            }
            inputFileY >> y[indexrow];
            cout << y[indexrow] << endl;
        }
    }
    //donw with reading , x and y are the two matrix
   
printf("matrxi before enclave\n"); 
    for(int i = 0; i<N; i++)
{
	for(int j = 0; j<M+1; j++) {printf("%f ", x[i*(M+1) + j]);}
	printf("\n");
}
    //why multiply 4
    ret = ecall_lr(global_eid, x, y, N, M+1, 4*(N* (M+1)),4* N);
    
    
    if (ret != SGX_SUCCESS)
        abort();
    
}
