/*
 * rabin_polynomial_main.c
 * 
 * Created by Joel Lawrence Tucci on 09-March-2011.
 * 
 * Copyright (c) 2011 Joel Lawrence Tucci
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the project's author nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "rabin_polynomial.h"
#include "rabin_polynomial_constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#ifdef __SDSCC__
#include <ff.h>
#include <sds_lib.h>
#endif

//File for creating a command line application for the rabin polynomial library

void print_usage() {
    
    fprintf(stderr,"Usage: rabinfingerprint -o binary output file -w window size -a average block size -m minimum block size -x maximum block size \n");
    fprintf(stderr,"Window size must be between %u and %u\n",RAB_POLYNOMIAL_MIN_WIN_SIZE,RAB_POLYNOMIAL_MAX_WIN_SIZE);
}

/**
 * convenience function, gets the unsigned value for the given
 * parameter.  Returns 0 if there was an error(TODO: make this better!)
 */
unsigned int get_uintval_from_arg(int argc, int index,  char **argv, unsigned int lower_bound, unsigned int upper_bound) {
    
    unsigned int return_val=0;
    
    if(index + 1 <= argc - 1) {
        return_val=(unsigned int)strtoll(argv[index+1],NULL,10);
        
        if(errno == EINVAL || errno== ERANGE) {
            fprintf(stderr,"Could not parse argument %s for switch %s!\n",argv[index],argv[index+1]);
            return 0;
        }
    } else {
        fprintf(stderr,"too few arguments for option %s!\n",argv[index]);
        print_usage();
        return 0;
    }
    
    if(return_val < lower_bound || return_val > upper_bound) {
        fprintf(stderr,"%s must be between %u and %u!\n",argv[index],lower_bound,upper_bound);
        print_usage();
        return 0;
    }
    
    return return_val;
    
}


/**
 *  Checks to see if the values inputted by the user make sense
 */
int check_arg_sanity() {
    
    if(rabin_polynomial_max_block_size <= rabin_polynomial_min_block_size) {
        fprintf(stderr, "Minimum block size must be greater than maximum cache size!\n");
        return 0;
    }
    
    if(rabin_polynomial_average_block_size < rabin_polynomial_min_block_size || rabin_polynomial_average_block_size > rabin_polynomial_max_block_size) {
        fprintf(stderr, "Average block size must be between min and maximum block size(%u and %u)\n",rabin_polynomial_min_block_size,rabin_polynomial_max_block_size);
        return 0;
    }
    
    return 1;
}

void close_file_if_open(FILE *file_to_close) {
    if(file_to_close != NULL)
        fclose(file_to_close);
}


////////////////////////////////////////////////////////////////////////////////////////////
void Check_error(int Error, const char * Message)
{
  if (Error)
  {
    fputs(Message, stderr);
    exit(EXIT_FAILURE);
  }
}

void Exit_with_error(void)
{
  perror(NULL);
  exit(EXIT_FAILURE);
}

unsigned char * Allocate(int Size)
{
  unsigned char * Frame = (unsigned char *)
#ifdef __SDSCC__
      sds_alloc(Size);
#else
      malloc(Size);
#endif
  Check_error(Frame == NULL, "Could not allocate memory.\n");

  return Frame;
}

void Free(unsigned char * Frame)
{
#ifdef __SDSCC__
  sds_free(Frame);
#else
  free(Frame);
#endif
}

void Load_data(unsigned char * Data)
{
  unsigned int Size = 10 * 960 * 540;

#ifdef __SDSCC__
  FIL File;
  unsigned int Bytes_read;

  FRESULT Result = f_open(&File, "Input.bin", FA_READ);
  Check_error(Result != FR_OK, "Could not open input file.");

  Result = f_read(&File, Data, Size, &Bytes_read);
  Check_error(Result != FR_OK || Bytes_read != Size, "Could not read input file.");

  Check_error(f_close(&File) != FR_OK, "Could not close input file.");
#else
  FILE * File = fopen("C:/Users/ylxiao/workspace/HW7_2a/src/Input.bin", "rb");
  if (File == NULL)
    Exit_with_error();

  if (fread(Data, 1, Size, File) != Size)
    Exit_with_error();

  if (fclose(File) != 0)
    Exit_with_error();
#endif
}

void Store_data(const char * Filename, unsigned char * Data, unsigned int Size)
{
#ifdef __SDSCC__
  FIL File;
  unsigned int Bytes_written;

  FRESULT Result = f_open(&File, Filename, FA_WRITE | FA_CREATE_ALWAYS);
  Check_error(Result != FR_OK, "Could not open output file.");

  Result = f_write(&File, Data, Size, &Bytes_written);
  Check_error(Result != FR_OK || Bytes_written != Size, "Could not write output file.");

  Check_error(f_close(&File) != FR_OK, "Could not close output file.");
#else
  FILE * File = fopen(Filename, "wb");
  if (File == NULL)
    Exit_with_error();

  if (fwrite(Data, 1, Size, File) != Size)
    Exit_with_error();

  if (fclose(File) != 0)
    Exit_with_error();
#endif
}


int main() {
	FILE *bin_out=NULL;

    int i;
    //rabin_sliding_window_size is from 17 to 63
    rabin_sliding_window_size=17;

    bin_out=fopen("C:/Users/ylxiao/workspace/Project01_cdc/src/Output.txt","wb+");

    if(bin_out == NULL) {
    	fprintf(stderr,"Could not open file %s for writing\n","Output.bin");
    }
    
    if(!check_arg_sanity()) {
        //Bad params, close the file we may have read and exit
        close_file_if_open(bin_out);
        return -1;
    }
    
    FILE *input_file=fopen("C:/Users/ylxiao/workspace/Project01_cdc/src/Input.bin", "r+");
    
    if(input_file == NULL) {
        fprintf(stderr, "Could not open file %s for reading\n","Input.bin");
        close_file_if_open(bin_out);
        return -1;
    }
    
    struct rabin_polynomial *head=get_file_rabin_polys(input_file);
    fclose(input_file);
    
    if(bin_out != NULL) {
        write_rabin_fingerprints_to_binary_file(bin_out,head);
        fclose(bin_out);
    } else {
            print_rabin_poly_list_to_file(stdout,head);
    }
    free_rabin_fingerprint_list(head);

    return 0;
}


