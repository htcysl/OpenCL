/************************************************
        CENG443 - PARALLEL PROGRAMMING II
        Take Home - Final Exam 

        Hatice Uysal 
        20050111076 
***********************************************/

#define CL_TARGET_OPENCL_VERSION 120

#include <stdlib.h>
#include <stdio.h> 
#include <math.h> 
#include <CL/cl.h> 

#define MAX_ITERATION 10 
#define SIZE 12

const char* programSource =
    "__kernel void matrixVectorMul( __global const double* matrix, \n"
    "                               __global const double* vector, \n"
    "                                const int row,                     \n"   // ??? row 
    "                               __global double* result)       \n" 
    "{                                                             \n"
    "    int i = get_global_id(0);                                 \n"
    "    double sum = 0.0;                                         \n"
    "    for (int j = 0; j < row; j++)                             \n"
    "        sum += matrix[i * row + j] * vector[j];               \n"
    "    result[i] = sum;                                          \n"
    "}                                                             \n"
    "                                                              \n"
    "__kernel void calculateNorm(__global float* vector, __global float* result, const int size) { \n"
    "  int global_id = get_global_id(0); \n"
    "  if (global_id < size) {            \n"
    "    float value = vector[global_id];  \n"
    "    atomic_add(result, value * value); \n"  // *result += value * value; 
    "  }                                  \n"
    " barrier(CLK_GLOBAL_MEM_FENCE);       \n"
    " if (global_id == 0)                  \n"
    "    *result = sqrt(*result);          \n"
    "}                                    \n"




    "__kernel void vectorScalarDiv(__global float* vector, float scalar)\n"
    "{\n"
    "    int i = get_global_id(0);\n"
    "    vector[i] /= scalar;\n"
    "}\n"
    "\n"
    "__kernel void matrixVectorSub(__global const float* matrix, __global const float* vector, __global float* result,const int row, float scalar)\n"
    "{\n"
    "    int i = get_global_id(0);\n"
    "    float sum = 0.0;\n"
    "    for (int j = 0; j < row; j++)\n"
    "        sum += matrix[i * row + j] * vector[j];\n"
    "    result[i] = vector[i] - scalar * sum;\n"
    "}\n";


void readFile(char* fileName, double** A, int *row, int *col ) ;
void initVector(double* vec, int row) ;


int main(int argc, char* argv[]) {

  
   double* A=NULL ;
   int row,col;
   
   readFile(argv[1], &A, &row, &col ) ;
  

  /* for(int i=0;i<row;i++) {
    for(int j=0;j<col;j++ )
       printf("%lf ",A[i*col+j]) ;
    printf("\n") ;
   }
*/

   double *eigVec1 = (double*)malloc(sizeof(double)*row);
   double *eigVec2 = (double*)malloc(sizeof(double)*row);
   double eigenVal1, eigenVal2 ;
   
   // ---------- Initialize the eigenvector values to be 1 -------
   initVector(eigVec1,row) ;
   initVector(eigVec2,row) ; 


   /*  for(int i=0;i<row;i++) 
      printf("%lf ", eigVec1[i]) ;

    printf("\n") ;
    for(int i=0;i<row;i++) 
      printf("%lf ", eigVec2[i]) ;*/
   // ------------------------------------------
     
    cl_kernel mulKernel, divKernel, subKernel ;
    //cl_double A[SIZE*SIZE], vec1[SIZE], vec2[SIZE] ;
    cl_double scalar = 2.0;
    
    // -------------------- OpenCL Context ------------------------
    cl_platform_id platform ; 
    cl_int err = clGetPlatformIDs(1, &platform, NULL) ;
    if (err != CL_SUCCESS) {printf("Error getting platform IDs: %d\n", err) ;exit(1) ;}
    
    cl_device_id device ;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL) ;
    if (err != CL_SUCCESS) {printf("Error getting GPU device ID: %d\n", err) ;exit(1);}

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err) ; 
    if (err != CL_SUCCESS) {printf("Error creating context: %d\n",err) ;exit(1) ;}
   
    // ------------------- OpenCL Command Queueu ---------------------  
    cl_command_queue  queue = clCreateCommandQueue(context, device, 0, &err) ;
    if (err != CL_SUCCESS) {printf("Error creating command queue: %d\n", err) ; clReleaseContext(context) ;return 1 ;} 
   
    // -------------------- Memory Buffer on the Device ----------------


    // -------------------- OpenCL Buffers (matrix A, eigen vectors 1 & 2) ------------------------
    cl_mem bufA =  clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_double)*SIZE*SIZE, A, &err ) ;
    cl_mem bufEigvec1 =  clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_double)*SIZE, eigVec1, &err ) ;
    cl_mem bufEigvec2 =  clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_double)*SIZE, eigVec2, &err ) ;
    cl_mem bufRow = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &row, &err ) ;
    
   // --------------------OpenCL Program Buildig -------------------
    cl_program program = clCreateProgramWithSource(context, 1, &programSource, NULL, &err) ;
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL) ;


    char build_log[4096];
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, NULL);
    printf("Build Log:\n%s\n", build_log);

  


  /*
   // ----------------------- OpenCL KErnel -------------------------
   mulKernel = clCreateKernel(program, "matrixVectorMul", &err) ;
   divKernel = clCreateKernel(program, "vectorScalarDiv", &err) ;
   subKernel = clCreateKernel(program, "matrixVectorSub", &err) ;

   // ------------------OpenCL KErnel Arguments --------------------- 
   err = clSetKernelArg(mulKernel, 0, sizeof(cl_mem), &memObjects[0]) ; 
   err = clSetKernelArg(mulKernel, 1, sizeof(cl_mem), &memObjects[4]) ;
   err = clSetKernelArg(mulKernel, 2, sizeof(cl_mem), &memObjects[1]) ; 

   err = clSetKernelArg(divKernel, 0, sizeof(cl_mem), &memObjects[1]) ;
   err = clSetKernelArg(divKernel, 1, sizeof(cl_double),&scalar) ;

   err = clSetKernelArg(subKernel, 0, sizeof(cl_mem), &memObjects[0]) ;
   err = clSetKernelArg(subKernel, 1, sizeof(cl_mem), &memObjects[4]) ;
   err = clSetKernelArg(subKernel, 2, sizeof(cl_mem), &memObjects[5]) ;
   err = clSetKernelArg(subKernel, 3, sizeof(cl_double), &scalar) ; 

   // -----------------OpenCL Enqueue The KErnels for Execution ---------- 
   size_t globalWorkSize[1] = {SIZE} ;
   for (int i=0; i<MAX_ITERATION; i++) {
      err = clEnqueueNDRangeKernel(queue, mulKernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL) ;
      err = clEnqueueNDRangeKernel(queue, divKernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL) ;
      err = clEnqueueNDRangeKernel(queue, subKernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL) ;     
   }

   // ----------------- Read The Result From the Device ----------------
   err = clEnqueueReadBuffer(queue, memObjects[5], CL_TRUE, 0, sizeof(cl_double)*SIZE, vec2,0, NULL, NULL) ; 

   // ----------------- Print the Result ------------
   for (int i =0 ;i< SIZE ;i++) {
    printf(" %lf ",vec2[i]) ;
   } 
   printf("\n") ;

   // ---------------Clean Up -----------------
   err = clReleaseKernel(mulKernel) ;
   err = clReleaseKernel(divKernel) ;
   err = clReleaseKernel(subKernel) ;
   err = clReleaseProgram(program) ;
   err = clReleaseMemObject(memObjects[0]) ;
   err = clReleaseMemObject(memObjects[1]) ;
   err = clReleaseMemObject(memObjects[2]) ;
   err = clReleaseMemObject(memObjects[3]) ;
   err = clReleaseMemObject(memObjects[4]) ;
   err = clReleaseMemObject(memObjects[5]) ;
   err = clReleaseCommandQueue(queue) ;
   err = clReleaseContext(context) ;
   */   
   return 0 ;
}
void readFile(char* fileName, double** A, int *row, int *col ) {
    FILE* file = fopen(fileName, "r") ; 
   
    if (file==NULL) {
        printf("File cannot be open!\n");
        exit(1);
    }
    // ------ reading row and column values from the file ---------
    fscanf(file,"%d %d", row, col) ; 
   
    // ------ allocating memory for data array ----------
    double* data = (double*)malloc( sizeof(double)*(*row)*(*col)) ;
    
    //------- reading data values from the file into data array ---------
    for (int i=0; i<(*row); i++) {
      for (int j=0; j<(*col); j++) {
        fscanf(file, "%lf", &data[i*(*col) + j] ) ;      
      }
    }
    // ----- closing the file --------
    fclose(file) ;
    
    // ----- set the pointer to the allocated data array  ---------
    *A = data ;
 }
 // ------ Initialize the eigenvector values to be 1 ----------
 void initVector(double* vec, int row) { for(int i=0;i< row; i++) vec[i] = 1.0 ;}




















