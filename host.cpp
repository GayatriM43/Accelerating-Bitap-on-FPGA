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

Description:

    This is a matrix multiplication which showcases the "Systolic Array" based
    algorithm design. Systolic array type of implementation is well suited for
    FPGAs. It is a good coding practice to convert base algorithm into Systolic
    Array implementation if it is feasible to do so.

*******************************************************************************/
#include "xcl2.hpp"
#include <vector>
#include <cmath>
#include <climits>

// Array Size to access
#define DATA_SIZE 16

// Maximum Array Size
#define MAX_SIZE 16

// Software implementation of Matrix Multiplication
// The inputs are of the size (DATA_SIZE x DATA_SIZE)
std::vector<long long int> gen_text_bits(std::string text)
{
    std::vector<long long int> text_bits;
    int sz = 64.0;
    int i = 0;
    while(i < ceil(text.size()/32.0))
    {
        text_bits.push_back(0);
        int j;
        for(j = 0;j < sz/2 && (i*32+j)<text.size();j++)
        {
            long long int cur_bit = 0;
            if(text[i+j] == 'C')
                cur_bit = 1;
            else if(text[i+j] == 'G')
                cur_bit = 2;
            else if(text[i+j] == 'T')
                cur_bit = 3;
            
            text_bits[i] = (text_bits[i]<<2)|cur_bit;
           // std::cout<<text_bits[i]<<" "<<text[i+j]<<"\n";
        }
        if(j<sz/2)
            text_bits[i] = text_bits[i]<<(sz-2*j);
        i++;
    }
//    std::cout<<"Converting text into bits "<<text_bits.size()<<"\n";
//    for(int i=0;i<text.size();i++)
//        std::cout<<text_bits[i]<<" ";
    return text_bits;
}
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

std::vector<long long int> gen_pat_bitmask(std::string pattern)
{
    int count = ceil((pattern.size())/64.0);
    std::vector<long long int> patternBitmasks(4*count);
    for (int i=0; i < 4*count; i++)
    {
        patternBitmasks[i] = ULLONG_MAX;
    }
    int index;
    int m = pattern.size();
    for (int i=0; i < m; i++)
    {
        index = count - ((m-i-1) / 64) - 1;
        if ((pattern[i] == 'A') || (pattern[i] == 'a'))
        {
            patternBitmasks[0*count + index] &= ~(1ULL << ((m-i-1) % 64));
        }
        else if ((pattern[i] == 'C') || (pattern[i] == 'c'))
        {
            patternBitmasks[1*count + index] &= ~(1ULL << ((m-i-1) % 64));
        }
        else if ((pattern[i] == 'G') || (pattern[i] == 'g'))
        {
            patternBitmasks[2*count + index] &= ~(1ULL << ((m-i-1) % 64));
        }
        else if ((pattern[i] == 'T') || (pattern[i] == 't'))
        {
            patternBitmasks[3*count + index] &= ~(1ULL << ((m-i-1) % 64));
        }

    }
    for(int i =0;i<patternBitmasks.size();i++)
    {
    	bin(patternBitmasks[i]);  std::cout<<"\n";
    	// std::cout<<"\n"<<patternBitmasks[i]<<"\n";
    }


    return patternBitmasks;
}

std::string read_text_file(std::string filename)
{
    std::ifstream input(filename);
    std::string DNA_sequence;
    std::string line;
    while (std::getline(input, line)) {
        if(line.empty())
            continue;
        DNA_sequence += line;
    }
    //std::cout<<"Reading text file "<<DNA_sequence.size()<<"\n";
    return DNA_sequence;
}

std::vector<std::string> read_pat_file(std::string filename)
{
    std::ifstream input(filename);
    std::vector<std::string> reads;
    std::string line;
    while (std::getline(input, line)) {
        if(line.empty())
            continue;
        reads.push_back(line);
    }

    return reads;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string binaryFile = argv[1];
    std::string text_file_name = "text.txt";
    std::string reads_file_name = "reads.txt";
    std::string text = read_text_file(text_file_name);
    std::vector<std::string> reads = read_pat_file(reads_file_name);
    int n = text.size();//text size
    int num_pat = reads.size();//number of patterns
    int m = reads.size()>0?reads[0].size():0;//pattern size
    int pat_size = ceil(m/64.0); //number of ints required to store the pattern

    cl_int err;
    cl::CommandQueue q;
    cl::Context context;
    cl::Kernel krnl_systolic_array;

    std::vector<long long int, aligned_allocator<long long int> > genome(2*n);
    std::vector<long long int, aligned_allocator<long long int> > patterns(4*pat_size*reads.size());
    std::vector<long long int, aligned_allocator<int> > startloc(n-m+1);
    std::vector<long long int, aligned_allocator<int> > pattern_num(n-m+1);

    // Create the test data and Software Result
    std::vector<long long int> text_bits = gen_text_bits(text);
    for (int i = 0; i < text_bits.size(); i++) {
        genome[i] = text_bits[i];
    }

    for(int i = 0; i< reads.size(); i++)
    {
        std::vector<long long int> bitmask = gen_pat_bitmask(reads[i]);
        for(int j = 0; j<bitmask.size();j++)
            patterns[i*bitmask.size()+j]=bitmask[j];
    }


    for(int i =0;i<startloc.size();i++)
    {
        startloc[i] = -1;
        pattern_num[i] = -1;
    }

    // OPENCL HOST CODE AREA START
    auto devices = xcl::get_xil_devices();

    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));

        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context, {device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            OCL_CHECK(err, krnl_systolic_array = cl::Kernel(program, "bitap", &err));
            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }

    // Allocate Buffer in Global Memory
    OCL_CHECK(err, cl::Buffer buffer_in1(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, 2*n*sizeof(long long int),
                                         genome.data(), &err));
    OCL_CHECK(err, cl::Buffer buffer_in2(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, 4*pat_size*reads.size()*sizeof(long long int),
                                         patterns.data(), &err));
    OCL_CHECK(err, cl::Buffer buffer_output1(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, (n-m+1)*sizeof(int),
                                            startloc.data(), &err));
     OCL_CHECK(err, cl::Buffer buffer_output2(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, (n-m+1)*sizeof(int),
                                             pattern_num.data(), &err));

    int len = pat_size*4;
    std::cout<<"total text ength in host is "<<n<<"\n";
    OCL_CHECK(err, err = krnl_systolic_array.setArg(0, buffer_in1));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(1, buffer_in2));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(2, buffer_output1));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(3, buffer_output2));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(4, n));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(5, m));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(6, pat_size));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(7, num_pat));

    // Copy input data to device global memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in1, buffer_in2}, 0 /* 0 means from host*/));

    // Launch the Kernel
    OCL_CHECK(err, err = q.enqueueTask(krnl_systolic_array));
    q.finish();

    // Copy Result from Device Global Memory to Host Local Memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output1, buffer_output2}, CL_MIGRATE_MEM_OBJECT_HOST));
    q.finish();
//    std::cout<<"\n start locs \n";
//    for(int i = 0; i <startloc.size(); i++)
//        std::cout<<startloc[i]<<" "<<pattern_num[i]<<"\n";

    return 0;
}
