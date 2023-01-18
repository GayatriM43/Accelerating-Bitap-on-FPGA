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
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <iomanip>

// This extension file is required for stream APIs
#include "CL/cl_ext_xilinx.h"
// This file is required for OpenCL C++ wrapper APIs
#include "xcl2.hpp"

// Array Size to access
#define DATA_SIZE 16

// Maximum Array Size
#define MAX_SIZE 32

// Software implementation of Matrix Multiplication
// The inputs are of the size (DATA_SIZE x DATA_SIZE)
/*void m_softwareGold(std::vector<int, aligned_allocator<int> >& in1, // Input Matrix 1
                    std::vector<int, aligned_allocator<int> >& in2, // Input Matrix 2
                    std::vector<int, aligned_allocator<int> >& out  // Output Matrix
                    ) {
    // Perform Matrix multiply Out = In1 x In2
    for (int i = 0; i < DATA_SIZE; i++) {
        for (int j = 0; j < DATA_SIZE; j++) {
            for (int k = 0; k < DATA_SIZE; k++) {
                out[i * DATA_SIZE + j] += in1[i * DATA_SIZE + k] * in2[k * DATA_SIZE + j];
            }
        }
    }
}*/

int main(int argc, char** argv) {
	std::cout<<"Started program execution\n";
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string binaryFile = argv[1];

    // Allocate Memory in Host Memory
    if (DATA_SIZE > MAX_SIZE) {
        std::cout << "Size is bigger than internal buffer size, please use a "
                     "size smaller than "
                  << MAX_SIZE << "!" << std::endl;
        return EXIT_FAILURE;
    }

    size_t text_size = 5;
    size_t text_size_bytes = sizeof(int) * text_size;
    size_t pattern_size = 32;
    size_t pattern_size_bytes = sizeof(int) * (pattern_size);
    cl_int err;
    cl::CommandQueue q;
    cl::Context context;
    cl::Kernel krnl_systolic_array;

    std::vector<int, aligned_allocator<int> > text(text_size);
    std::vector<int, aligned_allocator<int> > pattern(pattern_size);
    std::vector<int, aligned_allocator<int> > startloc(10);
    std::vector<int, aligned_allocator<int> > editdist(10);
    //std::vector<int, aligned_allocator<int> > R(130);
    std::vector<int, aligned_allocator<int> > pat_num(10);

    // Create the test data and Software Result
    for (size_t i = 0; i < text_size; i++) {

        startloc[i] = 0;
        editdist[i] = 0;
        pat_num[i] = 0;
    }
    text={0,2,3,2,1};
    pattern={1,0,0,0,0,0,0,0,0,1,0,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,1,0};


    // OPENCL HOST CODE AREA START
    auto devices = xcl::get_xil_devices();
    std::cout<<"Got device\n";
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
    int size;

    // Allocate Buffer in Global Memory
    OCL_CHECK(err, cl::Buffer buffer_text(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, text_size_bytes,
                                         text.data(), &err));
    OCL_CHECK(err, cl::Buffer buffer_pattern(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, pattern_size_bytes,
                                         pattern.data(), &err));
    OCL_CHECK(err, cl::Buffer buffer_startloc(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, 10*sizeof(int),
                                            startloc.data(), &err));
    OCL_CHECK(err, cl::Buffer buffer_editdist(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, 10*sizeof(int),
                                            editdist.data(), &err));
    //OCL_CHECK(err, cl::Buffer buffer_R(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, 130*sizeof(int),
    //                                            R.data(), &err));
    OCL_CHECK(err, cl::Buffer buffer_pat_num(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, 10*sizeof(int),
                                                    pat_num.data(), &err));
    //OCL_CHECK(err, cl::Buffer buffer_size(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,sizeof(int),
     //                                           &size, &err));

    int k=1,n=5,m=4,num_patt=2;
    OCL_CHECK(err, err = krnl_systolic_array.setArg(0, buffer_text));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(1, buffer_pattern));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(2, k));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(3, n));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(4, m));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(5, num_patt));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(6, buffer_startloc));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(7, buffer_editdist));
    //OCL_CHECK(err, err = krnl_systolic_array.setArg(8, buffer_R));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(8, size));
    OCL_CHECK(err, err = krnl_systolic_array.setArg(9, buffer_pat_num));

    std::cout<<"Set arguments\n";
    // Copy input data to device global memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_text, buffer_pattern}, 0 /* 0 means from host*/));
    std::cout<<"sent text and pattern to kernel\n";
    // Launch the Kernel
    OCL_CHECK(err, err = q.enqueueTask(krnl_systolic_array));
    q.finish();

    std::cout<<"Sending startloc and edit distance back\n";
    // Copy Result from Device Global Memory to Host Local Memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_startloc,buffer_editdist,buffer_pat_num}, CL_MIGRATE_MEM_OBJECT_HOST));
    q.finish();
    // OPENCL HOST CODE AREA END

    // Compute Software Results
    //m_softwareGold(source_in1, source_in2, source_sw_results);

    // Compare the results of the Device to the simulation
    int match = 0;
   /* for(int i=0;i<k;i++)
    	std::cout<<R_0[i]<<" ";
    std::cout<<"\n";
    for (int i = 0; i <startloc.size(); i++) {
        std::cout<<startloc[i]<<" ";
       // std::cout<<editdist[i]<<"\n";
        match=1;
    }*/
   //std::cout<<size<<"\n";
    for (int i = 0; i <16; i++) {
            std::cout<<startloc[i]<<" ";
            match=1;
        }
    std::cout<<"\n";
    for(int i=0;i<16;i++)
    	std::cout<<editdist[i]<<" ";
    std::cout<<"\n";
        for(int i=0;i<16;i++)
        	std::cout<<pat_num[i]<<" ";
        std::cout<<"\n";



    return (match ? EXIT_SUCCESS:EXIT_FAILURE);
}
