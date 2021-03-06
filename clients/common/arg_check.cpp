/* ************************************************************************
 * Copyright 2016 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#include <iostream>
#include "rocblas.h"
#include "arg_check.h"

#define PRINT_IF_HIP_ERROR(INPUT_STATUS_FOR_CHECK)                \
    {                                                             \
        hipError_t TMP_STATUS_FOR_CHECK = INPUT_STATUS_FOR_CHECK; \
        if(TMP_STATUS_FOR_CHECK != hipSuccess)                    \
        {                                                         \
            fprintf(stderr,                                       \
                    "hip error code: %d at %s:%d\n",              \
                    TMP_STATUS_FOR_CHECK,                         \
                    __FILE__,                                     \
                    __LINE__);                                    \
        }                                                         \
    }

/* ========================================Gtest Arg Check
 * ===================================================== */

/*! \brief Template: checks if arguments are valid
*/

void set_get_matrix_arg_check(rocblas_status status,
                              rocblas_int rows,
                              rocblas_int cols,
                              rocblas_int lda,
                              rocblas_int ldb,
                              rocblas_int ldc)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(status, rocblas_status_invalid_size);
#else
    if(status != rocblas_status_invalid_size)
    {
        std::cerr << "rocBLAS TEST ERROR in arguments rows, cols, lda, ldb, ldc: ";
        std::cerr << rows << ',' << cols << ',' << lda << ',' << ldb << ',' << ldc << std::endl;
    }
#endif
}

void set_get_vector_arg_check(
    rocblas_status status, rocblas_int M, rocblas_int incx, rocblas_int incy, rocblas_int incd)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(status, rocblas_status_invalid_size);
#else
    if(status != rocblas_status_invalid_size)
    {
        std::cerr << "rocBLAS TEST ERROR in arguments M, incx, incy, incd: ";
        std::cerr << M << ',' << incx << ',' << incy << ',' << incd << std::endl;
    }
#endif
}

void gemv_ger_arg_check(rocblas_status status,
                        rocblas_int M,
                        rocblas_int N,
                        rocblas_int lda,
                        rocblas_int incx,
                        rocblas_int incy)
{
#ifdef GOOGLE_TEST
    if(M < 0 || N < 0 || lda < M || lda < 1 || 0 == incx || 0 == incy)
    {
        ASSERT_EQ(status, rocblas_status_invalid_size);
    }
    else if(0 == M || 0 == N)
    {
        ASSERT_EQ(status, rocblas_status_success);
    }
    else
    {
        EXPECT_TRUE(false) << "error in gemv_ger_arg_check";
    }
#else
    if(M < 0 || N < 0 || lda < M || lda < 1 || 0 == incx || 0 == incy)
    {
        if(status != rocblas_status_invalid_size)
        {
            std::cerr << "rocBLAS TEST ERROR: (M < 0 || N < 0 || lda < M || lda < 1 || 0 == incx "
                         "|| 0 == incy) "
                      << std::endl;
            std::cerr << "rocBLAS TEST ERROR: and (status != rocblas_status_invalid_size)"
                      << std::endl;
            std::cerr << "rocBLAS TEST ERROR: status = " << status << std::endl;
        }
    }
    else if(0 == M || 0 == N)
    {
        if(status != rocblas_status_success)
        {
            std::cerr << "rocBLAS TEST ERROR: (0 == M || 0 == N)" << std::endl;
            std::cerr << "rocBLAS TEST ERROR: and (status != rocblas_status_success)" << std::endl;
            std::cerr << "rocBLAS TEST ERROR: status = " << status << std::endl;
        }
    }
#endif
}

void syr_arg_check(rocblas_status status, rocblas_int N, rocblas_int lda, rocblas_int incx)
{
#ifdef GOOGLE_TEST
    if(N < 0 || lda < N || lda < 1 || 0 == incx)
    {
        ASSERT_EQ(status, rocblas_status_invalid_size);
    }
    else if(0 == N)
    {
        ASSERT_EQ(status, rocblas_status_success);
    }
    else
    {
        EXPECT_TRUE(false) << "error in syr_arg_check";
    }
#else
    if(N < 0 || lda < N || lda < 1 || 0 == incx)
    {
        if(status != rocblas_status_invalid_size)
        {
            std::cerr << "rocBLAS TEST ERROR: (N < 0 || lda < N || lda < 1 || 0 == incx)"
                      << std::endl;
            std::cerr << "rocBLAS TEST ERROR: and (status != rocblas_status_invalid_size)"
                      << std::endl;
            std::cerr << "rocBLAS TEST ERROR: status = " << status << std::endl;
        }
    }
    else if(0 == N)
    {
        if(status != rocblas_status_success)
        {
            std::cerr << "rocBLAS TEST ERROR: (0 == N)" << std::endl;
            std::cerr << "rocBLAS TEST ERROR: and (status != rocblas_status_success)" << std::endl;
            std::cerr << "rocBLAS TEST ERROR: status = " << status << std::endl;
        }
    }
#endif
}

void gemm_arg_check(rocblas_status status,
                    rocblas_int M,
                    rocblas_int N,
                    rocblas_int K,
                    rocblas_int lda,
                    rocblas_int ldb,
                    rocblas_int ldc)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(status, rocblas_status_invalid_size);
#else
    std::cerr << "rocBLAS TEST ERROR in arguments M, N, K, lda, ldb, ldc: ";
    std::cerr << M << ',' << N << ',' << K << ',' << lda << ',' << ldb << ',' << ldc << std::endl;
#endif
}

void gemm_strided_batched_arg_check(rocblas_status status,
                                    rocblas_int M,
                                    rocblas_int N,
                                    rocblas_int K,
                                    rocblas_int lda,
                                    rocblas_int ldb,
                                    rocblas_int ldc,
                                    rocblas_int batch_count)
{
#ifdef GOOGLE_TEST
    if(M == 0 || N == 0 || K == 0 || batch_count == 0)
    {
        ASSERT_EQ(status, rocblas_status_success);
    }
    else
    {
        ASSERT_EQ(status, rocblas_status_invalid_size);
    }
#else
    std::cerr << "rocBLAS TEST ERROR in arguments M, N, K, lda, ldb, ldc, batch_count: ";
    std::cerr << M << ',' << N << ',' << K << ',' << lda << ',' << ldb << ',' << ldc << batch_count
              << std::endl;
#endif
}

void geam_arg_check(rocblas_status status,
                    rocblas_int M,
                    rocblas_int N,
                    rocblas_int lda,
                    rocblas_int ldb,
                    rocblas_int ldc)
{
#ifdef GOOGLE_TEST
    if(M == 0 || N == 0)
    {
        ASSERT_EQ(status, rocblas_status_success);
    }
    else
    {
        ASSERT_EQ(status, rocblas_status_invalid_size);
    }
#else
    std::cerr << "rocBLAS TEST ERROR in arguments M, N, lda, ldb, ldc: ";
    std::cerr << M << ',' << N << ',' << lda << ',' << ldb << ',' << ldc << std::endl;
#endif
}

void trsm_arg_check(
    rocblas_status status, rocblas_int M, rocblas_int N, rocblas_int lda, rocblas_int ldb)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(status, rocblas_status_invalid_size);
#else
    std::cerr << "rocBLAS TEST ERROR in arguments M, N, lda, ldb: ";
    std::cerr << M << ',' << N << ',' << lda << ',' << ldb << std::endl;
#endif
}

void symv_arg_check(
    rocblas_status status, rocblas_int N, rocblas_int lda, rocblas_int incx, rocblas_int incy)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(status, rocblas_status_invalid_size);
#else
    std::cerr << "rocBLAS TEST ERROR in arguments N, lda, incx, incy: ";
    std::cerr << N << ',' << lda << ',' << incx << ',' << incy << std::endl;
#endif
}

void iamax_iamin_arg_check(rocblas_status status, rocblas_int* d_rocblas_result)
{
    rocblas_int h_rocblas_result;
    if(rocblas_pointer_to_mode(d_rocblas_result) == rocblas_pointer_mode_device)
    {
        PRINT_IF_HIP_ERROR(hipMemcpy(
            &h_rocblas_result, d_rocblas_result, sizeof(rocblas_int), hipMemcpyDeviceToHost));
    }
    else
    {
        h_rocblas_result = *d_rocblas_result;
    }

#ifdef GOOGLE_TEST
    ASSERT_EQ(h_rocblas_result, 0);
    ASSERT_EQ(status, rocblas_status_success);
#else
    if(h_rocblas_result != 0)
    {
        std::cerr << "result should be 0, result =  " << h_rocblas_result << std::endl;
    }
#endif
}

template <>
void asum_arg_check(rocblas_status status, float* d_rocblas_result)
{
    float h_rocblas_result;
    if(rocblas_pointer_to_mode(d_rocblas_result) == rocblas_pointer_mode_device)
    {
        PRINT_IF_HIP_ERROR(
            hipMemcpy(&h_rocblas_result, d_rocblas_result, sizeof(float), hipMemcpyDeviceToHost));
    }
    else
    {
        h_rocblas_result = *d_rocblas_result;
    }

#ifdef GOOGLE_TEST
    ASSERT_EQ(h_rocblas_result, 0.0);
    ASSERT_EQ(status, rocblas_status_success);
#else
    if(h_rocblas_result != 0.0)
    {
        std::cerr << "result should be 0.0, result =  " << h_rocblas_result << std::endl;
    }
#endif
}

template <>
void asum_arg_check(rocblas_status status, double* d_rocblas_result)
{
    double h_rocblas_result;
    if(rocblas_pointer_to_mode(d_rocblas_result) == rocblas_pointer_mode_device)
    {
        PRINT_IF_HIP_ERROR(
            hipMemcpy(&h_rocblas_result, d_rocblas_result, sizeof(double), hipMemcpyDeviceToHost));
    }
    else
    {
        h_rocblas_result = *d_rocblas_result;
    }

#ifdef GOOGLE_TEST
    ASSERT_EQ(h_rocblas_result, 0.0);
#else
    if(h_rocblas_result != 0.0)
    {
        std::cerr << "result should be 0.0, result =  " << h_rocblas_result << std::endl;
    }
#endif
}

template <>
void nrm2_dot_arg_check(rocblas_status status, double* d_rocblas_result)
{
    double h_rocblas_result;

    if(rocblas_pointer_to_mode(d_rocblas_result) == rocblas_pointer_mode_device)
    {
        PRINT_IF_HIP_ERROR(
            hipMemcpy(&h_rocblas_result, d_rocblas_result, sizeof(double), hipMemcpyDeviceToHost));
    }
    else
    {
        h_rocblas_result = *d_rocblas_result;
    }
#ifdef GOOGLE_TEST
    ASSERT_EQ(h_rocblas_result, 0.0);
    ASSERT_EQ(status, rocblas_status_success);
#else
    if(h_rocblas_result != 0.0)
    {
        std::cerr << "result should be 0.0, result =  " << h_rocblas_result << std::endl;
    }
#endif
}

template <>
void nrm2_dot_arg_check(rocblas_status status, float* d_rocblas_result)
{
    float h_rocblas_result;
    if(rocblas_pointer_to_mode(d_rocblas_result) == rocblas_pointer_mode_device)
    {
        PRINT_IF_HIP_ERROR(
            hipMemcpy(&h_rocblas_result, d_rocblas_result, sizeof(float), hipMemcpyDeviceToHost));
    }
    else
    {
        h_rocblas_result = *d_rocblas_result;
    }
#ifdef GOOGLE_TEST
    ASSERT_EQ(h_rocblas_result, 0.0);
#else
    if(h_rocblas_result != 0.0)
    {
        std::cerr << "result should be 0.0, result =  " << h_rocblas_result << std::endl;
    }
#endif
}

void verify_rocblas_status_invalid_pointer(rocblas_status status, const char* message)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(status, rocblas_status_invalid_pointer);
#else
    if(status != rocblas_status_invalid_pointer)
    {
        std::cerr << message << std::endl;
    }
#endif
}

void verify_rocblas_status_invalid_size(rocblas_status status, const char* message)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(status, rocblas_status_invalid_size);
#else
    if(status != rocblas_status_invalid_size)
    {
        std::cerr << "rocBLAS TEST ERROR: status != rocblas_status_invalid_size, ";
        std::cerr << message << std::endl;
    }
#endif
}

// void handle_check(rocblas_status status)
void verify_rocblas_status_invalid_handle(rocblas_status status)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(status, rocblas_status_invalid_handle);
#else
    if(status != rocblas_status_invalid_handle)
    {
        std::cerr << "rocBLAS TEST ERROR: handle is null pointer" << std::endl;
    }
#endif
}

void verify_rocblas_status_success(rocblas_status status, const char* message)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(status, rocblas_status_success);
#else
    if(status != rocblas_status_success)
    {
        std::cerr << message << std::endl;
        std::cerr << "rocBLAS TEST ERROR: status should be rocblas_status_success" << std::endl;
        std::cerr << "rocBLAS TEST ERROR: status = " << status << std::endl;
    }
#endif
}

template <>
void verify_not_nan(rocblas_half arg)
{
// check against 16 bit IEEE NaN immediate value, will work on machine without 16 bit IEEE
#ifdef GOOGLE_TEST
    ASSERT_TRUE(static_cast<uint16_t>(arg & 0X7ff) <= static_cast<uint16_t>(0X7C00));
#else
    if(static_cast<uint16_t>(arg & 0X7ff) > static_cast<uint16_t>(0X7C00))
    {
        std::cerr << "rocBLAS TEST ERROR: argument is NaN" << std::endl;
    }
#endif
}

template <>
void verify_not_nan(float arg)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(arg, arg);
#else
    if(arg != arg)
    {
        std::cerr << "rocBLAS TEST ERROR: argument is NaN" << std::endl;
    }
#endif
}

template <>
void verify_not_nan(double arg)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(arg, arg);
#else
    if(arg != arg)
    {
        std::cerr << "rocBLAS TEST ERROR: argument is NaN" << std::endl;
    }
#endif
}

template <>
void verify_equal(int arg1, int arg2, const char* message)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(arg1, arg2);
#else
    if(arg1 != arg2)
    {
        std::cerr << message << std::endl;
        std::cerr << "rocBLAS TEST ERROR: arguments not equal" << std::endl;
    }
#endif
}

template <>
void verify_equal(bool arg1, bool arg2, const char* message)
{
#ifdef GOOGLE_TEST
    ASSERT_EQ(arg1, arg2);
#else
    if(arg1 != arg2)
    {
        std::cerr << message << std::endl;
        std::cerr << "rocBLAS TEST ERROR: arguments not equal" << std::endl;
    }
#endif
}
