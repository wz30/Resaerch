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

/* Test Array Attributes */

#include "sgx_trts.h"
#include "../Enclave.h"
#include "Enclave_t.h"
//#include "string.h"
//#include "math.h"

//m is row
//n is column
int maxsize = 0;  //max number of rows
int actualsize = 0;

//same as matrix
//return data
float*  new_matrix(int newmaxsize, int newactualsize)
{
  //printf("test in matrix\n");

  if (newmaxsize <= 0) newmaxsize = 5;
    maxsize = newmaxsize; 
  if ((newactualsize <= newmaxsize)&&(newactualsize>0))
    actualsize = newactualsize;
  else 
    actualsize = newmaxsize;
  float* data = 0;
  delete[] data;
  data = new float[maxsize*maxsize];
  return data;
}

bool setvalue(int row, int column, float newvalue, float* data)  
{
    //printf("test in set value\n");

    if ( (row >= maxsize) || (column >= maxsize) 
        || (row<0) || (column<0) ) return false;
    data[ row * maxsize + column ] = newvalue;
    return true;
}

void invert(float *data)  {
    if (actualsize <= 0) return;  // sanity check
    if (actualsize == 1) return;  // must be of dimension >= 2
    for (int i=1; i < actualsize; i++) data[i] /= data[0]; // normalize row 0
    for (int i=1; i < actualsize; i++)  { 
      for (int j=i; j < actualsize; j++)  { // do a column of L
        float sum = 0.0;
        for (int k = 0; k < i; k++)  
            sum += data[j*maxsize+k] * data[k*maxsize+i];
        data[j*maxsize+i] -= sum;
        }
      if (i == actualsize-1) continue;
      for (int j=i+1; j < actualsize; j++)  {  // do a row of U
        float sum = 0.0;
        for (int k = 0; k < i; k++)
            sum += data[i*maxsize+k]*data[k*maxsize+j];
        data[i*maxsize+j] = 
           (data[i*maxsize+j]-sum) / data[i*maxsize+i];
        }
      }
    for ( int i = 0; i < actualsize; i++ )  // invert L
      for ( int j = i; j < actualsize; j++ )  {
        float x = 1.0;
        if ( i != j ) {
          x = 0.0;
          for ( int k = i; k < j; k++ ) 
              x -= data[j*maxsize+k]*data[k*maxsize+i];
          }
        data[j*maxsize+i] = x / data[j*maxsize+j];
        }
    for ( int i = 0; i < actualsize; i++ )   // invert U
      for ( int j = i; j < actualsize; j++ )  {
        if ( i == j ) continue;
        float sum = 0.0;
        for ( int k = i; k < j; k++ )
            sum += data[k*maxsize+j]*( (i==k) ? 1.0 : data[i*maxsize+k] );
        data[i*maxsize+j] = -sum;
        }
    for ( int i = 0; i < actualsize; i++ )   // final inversion
      for ( int j = 0; j < actualsize; j++ )  {
        float sum = 0.0;
        for ( int k = ((i>j)?i:j); k < actualsize; k++ )  
            sum += ((j==k)?1.0:data[j*maxsize+k])*data[k*maxsize+i];
        data[j*maxsize+i] = sum;
        }
}

void getvalue(int row, int column, float& returnvalue, bool& success, float* data)   {
    if ( (row>=maxsize) || (column>=maxsize) 
      || (row<0) || (column<0) )
      {  success = false;
         return;    }
    returnvalue = data[ row * maxsize + column ];
    success = true;
}

void ecall_lr(float* x, float *y,int m, int n, size_t len1, size_t len2)
{
  printf("lr tets in enclave\n");
  printf("test global size: %d %d\n", maxsize, actualsize); 
  int MAX_ITR = 7;
  float * z = new float[m];
  float * h = new float[m];
  float * grad = new float[n];
  float * H = new float[n*n];
  float * invH = new float[n*n];
  float * theta = new float[n];
  memset(theta, 0, n*sizeof(float));
       
  printf("%d %d %d\n", m,n, len1);
  //print x
 /* for(int i = 0; i<m; i++)
{
	printf("%d: ",i);
	for(int j = 0; j<n; j++)
	{printf("%f ", x[i*n+j]);}
	printf("\n");
}*/
 
  for (int i = 0; i <MAX_ITR; i++){
       memset(z, 0, m*sizeof(float));
       for(int j = 0; j < m; j++){
           for(int k = 0; k < n; k++){
               z[j] += x[j * n + k] * theta[k];
           }
            h[j] = 1.0 / (1.0 + exp(-z[j]));
       }
       memset(grad, 0, n*sizeof(float));
       memset(H, 0, n*n*sizeof(float));
       printf("\n ================== \n");
       printf("iter: %d grad vector \n", i);
       //cout << endl << "=================" << endl << "iter: " << i << "  grad vector" << endl;
       for(int k = 0; k < n; k++){
           for(int j = 0; j < m; j++){
               grad[k] += x[j * n + k] * (h[j] - y[j]);
	       printf("%f ", grad[k]);	       
           }printf("\n");

           grad[k] /= m;
           for(int kk = 0; kk < n; kk++){
               //H[k*n+kk] =
               for(int j = 0; j < m; j++){
                  H[k*n + kk] += x[j *n + k] * h[j]*(1-h[j]) * x[j *n + kk];
               }
               H[k*n + kk] /= m;
           }
	   printf("%f\n", grad[k]);
           //cout << grad[k] << endl;
       }

       //same as matrix<float> H_n(n,n)
       float* data = new_matrix(n, n);
        
       //matrix <float> H_n(n, n);
       printf("\n iter: %d  H matrix\n", i);
       //cout << endl << "iter: " << i << "  H matrix" << endl;
       for(int k = 0; k < n; k++){
           for(int kk = 0; kk < n; kk++){
               //H_n.setvalue(k, kk, H[k*n + kk]);
 	       setvalue(k,kk,H[k*n + kk], data);

	       printf("%f ", H[k*n + kk]);
               //cout << H[k*n + kk] << " ";
           }
	   printf("\n");
           //cout << endl;
       }

       invert(data);
       
       bool success;
       printf("\niter: %d  inv H matrix\n", i);
       //cout << endl << "iter: " << i << "  inv H matrix" << endl;
       for(int k = 0; k < n; k++){
          for(int kk = 0; kk < n; kk++){
               //H_n.getvalue(k, kk, invH[k*n + kk], success);
	       getvalue(k, kk, invH[k*n + kk], success, data);
	       printf("%f ", invH[k*n + kk]);
               //cout << invH[k*n + kk] << " ";
           }
	   printf("\n");
           //cout << endl;
       }
       printf("\niter: %d  theta\n", i);
       //cout << endl << "iter: " << i << "  theta" << endl;
       for(int k = 0; k < n; k++){
           for(int kk = 0; kk < n; kk++){
              theta[k] -= invH[k*n + kk]*grad[kk];
           }
	  printf("%f\n", theta[k]);
          //cout << theta[k] << endl;
       }
    }
}



