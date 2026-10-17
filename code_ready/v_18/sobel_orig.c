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

unsigned char input[SIZE*SIZE], output[SIZE*SIZE], golden[SIZE*SIZE];

int size_of_image = SIZE*SIZE;
int size_of_proc_image = SIZE*(SIZE-1);

double sobel(unsigned char *input, unsigned char *output, unsigned char *golden) {
	
	double PSNR = 0;
	int i, j;
	unsigned int p;
	int res;
	struct timespec  tv1, tv2;
	FILE *f_in, *f_out, *f_golden;

	/* The first and last row of the output array, as well as the first  *
     * and last element of each column are not going to be filled by the *
     * algorithm, therefore make sure to initialize them with 0s.		 */
	memset(output, 0, SIZE*sizeof(unsigned char));
	memset(&output[SIZE*(SIZE-1)], 0, SIZE*sizeof(unsigned char));

	for (i = SIZE; i < SIZE*(SIZE-1); i+=SIZE) {
		output[i] = 0;
		output[i + SIZE - 1] = 0;
	}

	/* Open the input, output, golden files, read the input and golden    *
     * and store them to the corresponding arrays.						  */
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

	int p1 = 0, p2 = 0, diff = 0;
	const unsigned char *restrict row_center, *restrict row_above, *restrict row_below, *restrict g_ptr;
	unsigned char *restrict out_ptr;

	for (int i = SIZE + 1; i < size_of_proc_image; i += SIZE) {
		
		row_center = input + i;
		row_above  = row_center - SIZE;
		row_below  = row_center + SIZE;
		g_ptr = golden + i;
		out_ptr = output + i;
		
		const unsigned char *row_end = input + i + SIZE - 2;

		for (; row_center < row_end; ++row_center, ++row_above, ++row_below, ++g_ptr, ++out_ptr) {

			p1 = (
				(row_above[-1] * (-1)) + (row_above[1]) +
				(row_center[-1] * (-2) + row_center[1] * 2) + 
				(row_below[-1] * (-1) + row_below[1])
			);

			p2 = (
				(row_above[0] * 2 + row_above[-1]) + (row_above[1]) + 
				(row_below[0] * (-2) + row_below[-1]*(-1) + row_below[1] * (-1))
			);

			p = p1 * p1 + p2 * p2;
			res = (int)sqrt(p);

			if (res > 255)
				*out_ptr = 255;
			else
				*out_ptr = (unsigned char)res;

			diff = (int)(*out_ptr) - (int)(*g_ptr);
			PSNR += diff * diff;
		}
	}
  
	PSNR /= (double)(size_of_image);
	PSNR = 10*log10(65536/PSNR);

	clock_gettime(CLOCK_MONOTONIC_RAW, &tv2);

	printf ("Total time = %10g seconds\n",
			(double) (tv2.tv_nsec - tv1.tv_nsec) / 1000000000.0 +
			(double) (tv2.tv_sec - tv1.tv_sec));

  
	/* Write the output file */
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

