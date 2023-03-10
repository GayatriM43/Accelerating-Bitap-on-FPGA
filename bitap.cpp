/**
* Copyright (C) 2019-2021 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

/*******************************************************************************

Vitis Key Concept :

    This is a matrix multiplication example which showcases the "Systolic Array"
    based algorithm design. Systolic array type of implementation is well suited
    for FPGAs.

*******************************************************************************/

/*

Kernel Description :

    This kernel is a systolic array based matrix multiplication. Though the
    maximum size of the input matrices are restricted to a smaller MAX_SIZE, it
    is still possible to use this approach and get better performance for larger
    matrices by using tiling.

    Arguments :

        int *a     (input )  --> Input  Matrix A
        int *b     (input )  --> Input  Matrix B
        int *c     (output)  --> Output Matrix
        int  a_row (input )  --> Row Size Matrix A
        int  a_col (input )  --> Col Size Matrix A
        int  b_col (input )  --> Col Size Matrix B

    Kernel Configuration :

        Max Size    --> 16

    Note :
        Max Size is dependent on the available DSP resources in the FPGA
*/

#include <stdio.h>

// Maximum Array Size
#define MAX_SIZE 16

// TRIPCOUNT identifier
const unsigned int c_size = MAX_SIZE;

extern "C" {
void mmult(const int* text, // Text 0-A 1-G 2-C 3-T
           const int* pm, // Pattern bitmask of query
           int k, //Edit distance < MAX_SIZE
           int n,//length of text <= MAX_SIZE
           int m,//length of pattern <= MAX_SIZE
           int* startloc,// Starting location of alignments within k edit distance
           int* editdist,//Edit distance of corresponding alignments
           int size
           ) {

    // Local memory to store input and output matrices

    int pmLocal[4][MAX_SIZE];
#pragma HLS ARRAY_PARTITION variable = pmLocal dim = 1 complete

    int R[64];
#pragma HLS ARRAY_PARTITION variable = R dim = 0 complete



// Burst reads on input matrices from global memory
// Read Input A
// Auto-pipeline is going to apply pipeline to these loops
// Read Input B
readB:
    for (int i=0,j=0,loc=0;i<m&&j<4&&loc<m*4;i++,loc++) {
#pragma HLS LOOP_TRIPCOUNT min = c_size* c_size max = c_size * c_size
        if(i%m==0)
        {
            i=0;
            j++;
        }
        pmLocal[j][i] = pm[loc];
    }

// Perform systolic matrix multiply
// local matrices localA and localB have been partitioned in dimensions
// 1 and 2 respectively. local matrix C has been partitioned completely

// This partitioning enables to access MAX_SIZE elements in parallel in
// the local matrices. Because of the mode of access of array elements,
// we are able to perform MAX_SIZE*MAX_SIZE operations in parallel.

// Note : i, j and k loops are interchanged.

// The top loop systolic1 runs only for a_col iterations instead of
// MAX_SIZE like the inner loops. The inner loops have fixed loop
// iteration counts to enable complete unroll

// The following diagram explains how the matrix multiply happens
//
//        B_0        B_1        B_2        B_3
//         |          |          |          |
//         v          v          v          v
//        ___        ___        ___        ___
//       |   |      |   |      |   |      |   |
//  A0_->|C00| ---- |C01| ---- |C02| ---- |C03|
//       |___|      |___|      |___|      |___|
//         |          |          |          |
//        ___        ___        ___        ___
//       |   |      |   |      |   |      |   |
//  A1_->|C10| ---- |C11| ---- |C12| ---- |C13|
//       |___|      |___|      |___|      |___|
//         |          |          |          |
//        ___        ___        ___        ___
//       |   |      |   |      |   |      |   |
//  A2_->|C20| ---- |C21| ---- |C21| ---- |C21|
//       |___|      |___|      |___|      |___|
//         |          |          |          |
//        ___        ___        ___        ___
//       |   |      |   |      |   |      |   |
//  A3_->|C30| ---- |C31| ---- |C32| ---- |C33|
//       |___|      |___|      |___|      |___|

int align_num=0;
    for (int text_pos = 0; text_pos <= 5; text_pos++)
     {
        #pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
        for (int d = 0; d <= k; d++) 
        {
            #pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
            for(int num_pat=0;num_pat<2;num_pat++)
            {
                #pragma HLS unroll
                for (int que_pos = 0; que_pos <= 4; que_pos++) 
                {
                    #pragma HLS unroll
                    int D,S,M,I,result;
                    result=que_pos==0?1:text_pos==0?0:0;
                    if(d==0 && text_pos>0 && que_pos>0)
                    {
                        int ind=num_pat*2*(k+1)*(m+1)+((text_pos-1)%2)*(k+1)*(m+1)+d*(m+1)+(que_pos-1);
                        result=R[ind]&&pattern[text[text_pos-1]][num_pat*4+que_pos-1];
                    }
                    else if(text_pos>0 && que_pos>0)
                    {
                            int ind1=num_pat*2*(k+1)*(m+1) + ((text_pos-1)%2)*(k+1)*(m+1) + d*(m+1) + (que_pos-1);
                            int ind2=num_pat*2*(k+1)*(m+1) + ((text_pos-1)%2)*(k+1)*(m+1) + (d-1)*(m+1) + (que_pos-1);
                            int ind3=num_pat*2*(k+1)*(m+1) + ((text_pos)%2)*(k+1)*(m+1) + (d-1)*(m+1) + (que_pos-1);
                            int ind4=num_pat*2*(k+1)*(m+1) + ((text_pos-1)%2)*(k+1)*(m+1) + (d-1)*(m+1) + (que_pos);
                            M=R[ind1]&&pattern[text[text_pos-1]][num_pat*4+que_pos-1];
                            S=R[ind2];
                            D=R[ind3];
                            I=R[ind4];
                            result=M||S||D||I;
                    }
                    int ind=num_pat*2*(k+1)*(m+1)+((text_pos)%2)*(k+1)*(m+1)+d*(m+1)+(que_pos);
                    R[ind]=result;
                    //cout<<R[ind];
            
                    if(que_pos==m && result==1)
                    {
                        startloc[align_num]=text_pos;
                        editdist[align_num]=d;
                        align_num++;
                    }
                }
            }
            //cout<<" ";
        }
        //cout<<endl;
    }

// Burst write from output matrices to global memory
// Burst write from matrix C
}
}
