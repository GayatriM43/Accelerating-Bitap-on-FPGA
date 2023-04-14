

#include <stdio.h>
#include<cmath>
#include <climits>
#include<iostream>
// Maximum Array Size
#define MAX_SIZE 16

// TRIPCOUNT identifier
const unsigned int c_size = MAX_SIZE;

extern "C" {
void bin(long long int n)
{
	int count =0;
	while(n!=0 && count<64)
	{
		std::cout<<(n&1);
		n=n>>1;
		count++;
	}

}
void bitap(const long long int* text, // text packed into integers
           const long long int* pattern, // pattern bitmasks of all patterns packed into integers
           int* startloc,       // starting location of alignments
           int* pattern_num,   //edit distance of alignments
           int n,    // len of text
           int m,    // len of pattern
           int len,     // number of array elements for pattern bitmask of each pattern
           int num_pat //number of patterns
           ) {

    // Local memory to store input and output matrices
long long int R[num_pat][len];
//#pragma HLS ARRAY_PARTITION variable=R dim=1 complete
//#pragma HLS bind_storage variable=R type=FIFO impl=SRL

   long long int max = ULLONG_MAX;
    for(int pat = 0;pat < num_pat;pat++)
        {
            for(int pat_size = 0;pat_size<len;pat_size++)
            {
                R[pat][pat_size] = max;
            }
        }

long long int localPM[num_pat][len*4];
//#pragma HLS ARRAY_PARTITION variable=localPM dim=1 complete
//#pragma HLS bind_storage variable=localPM type=RAM_1WNR impl=BRAM

for(int i = 0,loc=0;i<num_pat;i++)
{
    for(int j=0;j<len*4;j++,loc++)
    {
       // #pragma HLS pipeline II=1
        localPM[i][j] = pattern[loc];
    }
}
int localpat[MAX_SIZE];
//#pragma HLS ARRAY_PARTITION variable = localpat dim = 0 complete
#pragma HLS bind_storage variable=localpat type=RAM_1WNR impl=URAM

int localstart[MAX_SIZE];
//#pragma HLS ARRAY_PARTITION variable = localstart dim = 0 complete
#pragma HLS bind_storage variable=localstart type=RAM_1WNR impl=URAM

long long int localText[MAX_SIZE];
//#pragma HLS ARRAY_PARTITION variable = localText dim = 0 complete
#pragma HLS bind_storage variable=localText type=RAM_1WNR impl=URAM

int total_itr = ceil(n/float(MAX_SIZE));
int cur = 0;
for(int itr = 0;itr < total_itr; itr++)
{

    for (int i = 0; i < MAX_SIZE && i+itr*MAX_SIZE<n ;i++) {
   // #pragma HLS LOOP_TRIPCOUNT min = c_size* c_size max = c_size * c_size
    //#pragma HLS pipeline II=1
        localText[i] = text[i+itr*MAX_SIZE];
        bin(localText[i]);
    }
    std::cout<<"\n";

    for(int txt_pos = 0;txt_pos<MAX_SIZE*32;txt_pos++)
    {
        //#pragma HLS pipeline II=2
        for(int pat = 0;pat<num_pat;pat++)
        {
            //#pragma HLS unroll
            //shift R
            R[pat][0] = R[pat][0] << 1;
            for(int i=1;i <len;i++)
            {
                //#pragma HLS pipeline II=1
                R[pat][i-1] |= (R[pat][i] >> 63);
                R[pat][i] = R[pat][i] << 1;
            }

            //OR with patternbitmask
            int p = 31 - ((txt_pos%32)*2);
            int cur_char = (3 & ( localText[txt_pos>>5]>> (p - 1)));
            for(int i=0;i < len;i++)
            {
                //#pragma HLS pipeline II=1
                R[pat][i] = R[pat][i]|localPM[pat][i+cur_char*len];
            }

            //Check and store output
            int msb = R[pat][0]>>63;
            if(msb == 0)
            {
                localstart[cur%MAX_SIZE] = txt_pos;
                localpat[cur%MAX_SIZE] = pat;
                cur++;
                if(cur % MAX_SIZE == 0)
                {
                    for(int i = 0;i < MAX_SIZE;i++)
                    {
                        startloc[cur] = localstart[i];
                        pattern_num[cur] = localpat[i];
                    }
                }
            }


        }
        if(cur % MAX_SIZE)
        {
        for(int i = 0;i < MAX_SIZE;i++)

        {
            startloc[cur] = localstart[i];
            pattern_num[cur] = localpat[i];

        }
        cur++;
        }
    }
    }
}




    }

