#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define SIZE	4096
#define INPUT_FILE	"input.grey"
#define OUTPUT_FILE	"output_sobel.grey"
#define GOLDEN_FILE	"golden.grey"

int size_of_image = SIZE*SIZE;
int size_of_proc_image = SIZE*(SIZE-1);

unsigned char input[SIZE*SIZE], output[SIZE*SIZE], golden[SIZE*SIZE];


double sobel(unsigned char *input, unsigned char *output, unsigned char *golden) {
	
	double PSNR = 0;
	int i, j;
	unsigned int p;
	int res;
	struct timespec  tv1, tv2;
	FILE *f_in, *f_out, *f_golden;

	memset(output, 0, SIZE*sizeof(unsigned char));
	memset(&output[SIZE*(SIZE-1)], 0, SIZE*sizeof(unsigned char));

	for (i = SIZE; i < SIZE*(SIZE-1); i+=SIZE) {
		output[i] = 0;
		output[i + SIZE - 1] = 0;
	}

	f_in = fopen(INPUT_FILE, "r");
	if (f_in == NULL) {
		printf("File " INPUT_FILE " not found\n");
		exit(1);
	}
  
	f_out = fopen(OUTPUT_FILE, "wb");
	if (f_out == NULL) {
		printf("File " OUTPUT_FILE " could not be created\n");
		fclose(f_in);
		exit(1);
	}  
  
	f_golden = fopen(GOLDEN_FILE, "r");
	if (f_golden == NULL) {
		printf("File " GOLDEN_FILE " not found\n");
		fclose(f_in);
		fclose(f_out);
		exit(1);
	}    

	fread(input, sizeof(unsigned char), SIZE*SIZE, f_in);
	fread(golden, sizeof(unsigned char), SIZE*SIZE, f_golden);
	fclose(f_in);
	fclose(f_golden);
  
	clock_gettime(CLOCK_MONOTONIC_RAW, &tv1);

	int temp=size_of_proc_image, end_of_curr_row=0, p1=0, p2=0, diff=0;
	const unsigned char *row_center, *row_above, *row_below;
	
	for (i=SIZE+1; i<temp; i+=SIZE ) {

		end_of_curr_row = i + SIZE-2;
		
		for (j= i; j< end_of_curr_row; j+=1) {

			row_center = input + j;
			row_above  = row_center - SIZE;
			row_below  = row_center + SIZE;

			p1 = (
				(row_above[-1] * (-1)) + (row_above[1]) +
				(row_center[-1] * (-2) + row_center[1] * 2) + 
				(row_below[-1] * (-1) + row_below[1])
			);

			p2 = (
				(row_above[0] * 2 + row_above[-1]) + (row_above[1]) + 
				(row_below[0] * (-2) + row_below[-1]*(-1) + row_below[1] * (-1))
			);

			p = p1*p1 + p2*p2;

			res = (int)sqrt(p);

			if (res > 255)
				output[j] = 255;      
			else
				output[j] = (unsigned char)res;
			

			diff = output[j] - golden[j];
			PSNR += diff*diff;

		}
	}
  
	PSNR /= (double)(size_of_image);
	PSNR = 10*log10(65536/PSNR);

	clock_gettime(CLOCK_MONOTONIC_RAW, &tv2);

	printf ("Total time = %10g seconds\n",
			(double) (tv2.tv_nsec - tv1.tv_nsec) / 1000000000.0 +
			(double) (tv2.tv_sec - tv1.tv_sec));

  	fwrite(output, sizeof(unsigned char), size_of_image, f_out);
	fclose(f_out);
  
	return PSNR;
}

int main(int argc, char* argv[]) {
	double PSNR;
	PSNR = sobel(input, output, golden);
	printf("PSNR of original Sobel and computed Sobel image: %g\n", PSNR);
	printf("A visualization of the sobel filter can be found at " OUTPUT_FILE ", or you can run 'make image' to get the jpg\n");

	return 0;
}