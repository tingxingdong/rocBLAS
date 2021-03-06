/* ************************************************************************
 * Copyright 2016 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#include <stdlib.h>
#include <vector>

#include "rocblas.hpp"
#include "utility.h"
#include "cblas_interface.h"
#include "norm.h"
#include "unit.h"
#include "arg_check.h"
#include <complex.h>
#include <unistd.h>
#include <pwd.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <sys/param.h>

using namespace std;

// replaces X in string with s, d, c, z or h depending on typename T
template <typename T>
std::string replaceX(std::string input_string)
{
    if(std::is_same<T, float>::value)
    {
        std::replace(input_string.begin(), input_string.end(), 'X', 's');
    }
    else if(std::is_same<T, double>::value)
    {
        std::replace(input_string.begin(), input_string.end(), 'X', 'd');
    }
    else if(std::is_same<T, rocblas_float_complex>::value)
    {
        std::replace(input_string.begin(), input_string.end(), 'X', 'c');
    }
    else if(std::is_same<T, rocblas_double_complex>::value)
    {
        std::replace(input_string.begin(), input_string.end(), 'X', 'z');
    }
    else if(std::is_same<T, rocblas_half>::value)
    {
        std::replace(input_string.begin(), input_string.end(), 'X', 'h');
    }
    return input_string;
}

// test for files equal
template <typename InputIterator1, typename InputIterator2>
bool range_equal(InputIterator1 first1,
                 InputIterator1 last1,
                 InputIterator2 first2,
                 InputIterator2 last2)
{
    while(first1 != last1 && first2 != last2)
    {
        if(*first1 != *first2)
        {
            //          cout << std::endl << "----------<"<< *first1 << " " << *first2 <<
            //          ">----------" << std::endl;
            return false;
        }
        else
        {
            ++first1;
            ++first2;
        }
    }
    return (first1 == last1) && (first2 == last2);
}

template <typename T>
void testing_logging()
{
    rocblas_pointer_mode test_pointer_mode = rocblas_pointer_mode_host;

    // set environment variable ROCBLAS_LAYER to turn on logging. Note that putenv
    // only has scope for this executable, so it is not necessary to save and restore
    // this environment variable
    //
    // ROCBLAS_LAYER is a bit mask:
    // ROCBLAS_LAYER = 1 turns on log_trace
    // ROCBLAS_LAYER = 2 turns on log_bench
    // ROCBLAS_LAYER = 3 turns on log_trace and log_bench
    char env_rocblas_layer[80] = "ROCBLAS_LAYER=3";
    verify_equal(
        putenv(env_rocblas_layer), 0, "failed to set environment variable ROCBLAS_LAYER=3");

    // set environment variable to give pathname of for log_trace file
    std::string trace_name1             = "stream_trace.csv";
    char env_rocblas_log_trace_path[80] = "ROCBLAS_LOG_TRACE_PATH=stream_trace.csv";
    verify_equal(putenv(env_rocblas_log_trace_path),
                 0,
                 "failed to set environment variable ROCBLAS_LOG_TRACE_PATH=stream_trace.csv");

    // set environment variable to give pathname of for log_bench file
    std::string bench_name1             = "stream_bench.txt";
    char env_rocblas_log_bench_path[80] = "ROCBLAS_LOG_BENCH_PATH=stream_bench.txt";
    verify_equal(putenv(env_rocblas_log_bench_path),
                 0,
                 "failed to set environment variable ROCBLAS_LOG_BENCH_PATH=stream_bench.txt");

    //
    // call rocBLAS functions with log_trace and log_bench to output log_trace and log_bench files
    //

    rocblas_int m            = 1;
    rocblas_int n            = 1;
    rocblas_int k            = 1;
    rocblas_int incx         = 1;
    rocblas_int incy         = 1;
    rocblas_int lda          = 1;
    rocblas_int bsa          = 1;
    rocblas_int ldb          = 1;
    rocblas_int bsb          = 1;
    rocblas_int ldc          = 1;
    rocblas_int bsc          = 1;
    rocblas_int batch_count  = 1;
    T alpha                  = 1.0;
    T beta                   = 1.0;
    rocblas_operation transA = rocblas_operation_none;
    rocblas_operation transB = rocblas_operation_transpose;
    rocblas_fill uplo        = rocblas_fill_upper;
    rocblas_diagonal diag    = rocblas_diagonal_unit;
    rocblas_side side        = rocblas_side_left;

    rocblas_int safe_dim = ((m > n ? m : n) > k ? (m > n ? m : n) : k);
    rocblas_int size_x   = n * incx;
    rocblas_int size_y   = n * incy;
    rocblas_int size_a   = (lda > bsa ? lda : bsa) * safe_dim * batch_count;
    rocblas_int size_b   = (ldb > bsb ? ldb : bsb) * safe_dim * batch_count;
    rocblas_int size_c   = (ldc > bsc ? ldc : bsc) * safe_dim * batch_count;

    // allocate memory on device
    auto dx_managed = rocblas_unique_ptr{rocblas_test::device_malloc(sizeof(T) * size_x),
                                         rocblas_test::device_free};
    auto dy_managed = rocblas_unique_ptr{rocblas_test::device_malloc(sizeof(T) * size_y),
                                         rocblas_test::device_free};
    auto da_managed = rocblas_unique_ptr{rocblas_test::device_malloc(sizeof(T) * size_a),
                                         rocblas_test::device_free};
    auto db_managed = rocblas_unique_ptr{rocblas_test::device_malloc(sizeof(T) * size_b),
                                         rocblas_test::device_free};
    auto dc_managed = rocblas_unique_ptr{rocblas_test::device_malloc(sizeof(T) * size_c),
                                         rocblas_test::device_free};
    T* dx = (T*)dx_managed.get();
    T* dy = (T*)dy_managed.get();
    T* da = (T*)da_managed.get();
    T* db = (T*)db_managed.get();
    T* dc = (T*)dc_managed.get();
    if(!dx || !dy || !da || !db || !dc)
    {
        PRINT_IF_HIP_ERROR(hipErrorOutOfMemory);
        return;
    }

    rocblas_status status;

    // enclose in {} so rocblas_handle destructor called as it goes out of scope
    {
        int i_result;
        T result;
        rocblas_pointer_mode mode;

        // Auxiliary functions
        std::unique_ptr<rocblas_test::handle_struct> unique_ptr_handle(
            new rocblas_test::handle_struct);
        rocblas_handle handle = unique_ptr_handle->handle;

        status = rocblas_set_pointer_mode(handle, test_pointer_mode);
        status = rocblas_get_pointer_mode(handle, &mode);

        // BLAS1
        status = rocblas_iamax<T>(handle, n, dx, incx, &i_result);

        status = rocblas_iamin<T>(handle, n, dx, incx, &i_result);

        status = rocblas_asum<T, T>(handle, n, dx, incx, &result);

        status = rocblas_axpy<T>(handle, n, &alpha, dx, incx, dy, incy);

        status = rocblas_copy<T>(handle, n, dx, incx, dy, incy);

        status = rocblas_dot<T>(handle, n, dx, incx, dy, incy, &result);

        status = rocblas_nrm2<T, T>(handle, n, dx, incx, &result);

        status = rocblas_scal<T>(handle, n, &alpha, dx, incx);

        status = rocblas_swap<T>(handle, n, dx, incx, dy, incy);

        // BLAS2
        status = rocblas_ger<T>(handle, m, n, &alpha, dx, incx, dy, incy, da, lda);

        status = rocblas_syr<T>(handle, uplo, n, &alpha, dx, incx, da, lda);

        status = rocblas_gemv<T>(handle, transA, m, n, &alpha, da, lda, dx, incx, &beta, dy, incy);

        // BLAS3
        status =
            rocblas_geam<T>(handle, transA, transB, m, n, &alpha, da, lda, &beta, db, ldb, dc, ldc);

        if(BUILD_WITH_TENSILE)
        {
            /* trsm calls rocblas_get_stream and rocblas_dgemm, so test it by comparing files
                        status = rocblas_trsm<T>(handle, side, uplo, transA, diag, m, n, &alpha, da,
               lda, db, ldb);
            */
            status = rocblas_gemm<T>(
                handle, transA, transB, m, n, k, &alpha, da, lda, db, ldb, &beta, dc, ldc);

            status = rocblas_gemm_strided_batched<T>(handle,
                                                     transA,
                                                     transB,
                                                     m,
                                                     n,
                                                     k,
                                                     &alpha,
                                                     da,
                                                     lda,
                                                     bsa,
                                                     db,
                                                     ldb,
                                                     bsb,
                                                     &beta,
                                                     dc,
                                                     ldc,
                                                     bsc,
                                                     batch_count);
        }

        // exclude trtri as it is an internal function
        //      status = rocblas_trtri<T>(handle, uplo, diag, n, da, lda, db, ldb);

        // trmm
        // tritri
    }

    //
    // write "golden file"
    //

    // find cwd string
    char temp[MAXPATHLEN];
    std::string cwd_str = (getcwd(temp, MAXPATHLEN) ? std::string(temp) : std::string(""));

    // open files
    std::string trace_name2 = "rocblas_log_trace_gold.csv";
    std::string trace_path1 = cwd_str + "/" + trace_name1;
    std::string trace_path2 = cwd_str + "/" + trace_name2;

    std::string bench_name2 = "rocblas_log_bench_gold.txt";
    std::string bench_path1 = cwd_str + "/" + bench_name1;
    std::string bench_path2 = cwd_str + "/" + bench_name2;

    std::ofstream trace_ofs2;
    std::ofstream bench_ofs2;

    trace_ofs2.open(trace_path2);
    bench_ofs2.open(bench_path2);

    // Auxiliary function
    trace_ofs2 << "rocblas_create_handle";
    trace_ofs2 << "\nrocblas_set_pointer_mode,0";
    trace_ofs2 << "\nrocblas_get_pointer_mode,0";

    // BLAS1
    trace_ofs2 << "\n"
               << replaceX<T>("rocblas_iXamax") << "," << n << "," << (void*)dx << "," << incx;
    bench_ofs2 << "\n"
               << "./rocblas-bench -f iamax -r " << replaceX<T>("X") << " -n " << n << " --incx "
               << incx;

    trace_ofs2 << "\n"
               << replaceX<T>("rocblas_iXamin") << "," << n << "," << (void*)dx << "," << incx;
    bench_ofs2 << "\n"
               << "./rocblas-bench -f iamin -r " << replaceX<T>("X") << " -n " << n << " --incx "
               << incx;

    trace_ofs2 << "\n"
               << replaceX<T>("rocblas_Xasum") << "," << n << "," << (void*)dx << "," << incx;
    bench_ofs2 << "\n"
               << "./rocblas-bench -f asum -r " << replaceX<T>("X") << " -n " << n << " --incx "
               << incx;

    if(test_pointer_mode == rocblas_pointer_mode_host)
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xaxpy") << "," << n << "," << alpha << "," << (void*)dx
                   << "," << incx << "," << (void*)dy << "," << incy;
        bench_ofs2 << "\n"
                   << "./rocblas-bench -f axpy -r " << replaceX<T>("X") << " -n " << n
                   << " --alpha " << alpha << " --incx " << incx << " --incy " << incy;
    }
    else
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xaxpy") << "," << n << "," << (void*)&alpha << ","
                   << (void*)dx << "," << incx << "," << (void*)dy << "," << incy;
    }

    trace_ofs2 << "\n"
               << replaceX<T>("rocblas_Xcopy") << "," << n << "," << (void*)dx << "," << incx << ","
               << (void*)dy << "," << incy;
    bench_ofs2 << "\n"
               << "./rocblas-bench -f copy -r " << replaceX<T>("X") << " -n " << n << " --incx "
               << incx << " --incy " << incy;

    trace_ofs2 << "\n"
               << replaceX<T>("rocblas_Xdot") << "," << n << "," << (void*)dx << "," << incx << ","
               << (void*)dy << "," << incy;
    bench_ofs2 << "\n"
               << "./rocblas-bench -f dot -r " << replaceX<T>("X") << " -n " << n << " --incx "
               << incx << " --incy " << incy;

    trace_ofs2 << "\n"
               << replaceX<T>("rocblas_Xnrm2") << "," << n << "," << (void*)dx << "," << incx;
    bench_ofs2 << "\n"
               << "./rocblas-bench -f nrm2 -r " << replaceX<T>("X") << " -n " << n << " --incx "
               << incx;

    if(test_pointer_mode == rocblas_pointer_mode_host)
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xscal") << "," << n << "," << alpha << "," << (void*)dx
                   << "," << incx;
        bench_ofs2 << "\n"
                   << "./rocblas-bench -f scal -r " << replaceX<T>("X") << " -n " << n << " --incx "
                   << incx << " --alpha " << alpha;
    }
    else
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xscal") << "," << n << "," << (void*)&alpha << ","
                   << (void*)dx << "," << incx;
    }
    trace_ofs2 << "\n"
               << replaceX<T>("rocblas_Xswap") << "," << n << "," << (void*)dx << "," << incx << ","
               << (void*)dy << "," << incy;

    bench_ofs2 << "\n"
               << "./rocblas-bench -f swap -r " << replaceX<T>("X") << " -n " << n << " --incx "
               << incx << " --incy " << incy;

    // BLAS2
    std::string transA_letter;
    if(transA == rocblas_operation_none)
    {
        transA_letter = "N";
    }
    else if(transA == rocblas_operation_transpose)
    {
        transA_letter = "T";
    }
    else if(transA == rocblas_operation_conjugate_transpose)
    {
        transA_letter = "C";
    }

    std::string transB_letter;
    if(transB == rocblas_operation_none)
    {
        transB_letter = "N";
    }
    else if(transB == rocblas_operation_transpose)
    {
        transB_letter = "T";
    }
    else if(transB == rocblas_operation_conjugate_transpose)
    {
        transB_letter = "C";
    }

    std::string side_letter;
    if(side == rocblas_side_left)
    {
        side_letter = "L";
    }
    else if(side == rocblas_side_right)
    {
        side_letter = "R";
    }
    else if(side == rocblas_side_both)
    {
        side_letter = "B";
    }

    std::string uplo_letter;
    if(uplo == rocblas_fill_upper)
    {
        uplo_letter = "U";
    }
    else if(uplo == rocblas_fill_lower)
    {
        uplo_letter = "L";
    }
    else if(uplo == rocblas_fill_full)
    {
        uplo_letter = "F";
    }

    std::string diag_letter;
    if(diag == rocblas_diagonal_non_unit)
    {
        diag_letter = "N";
    }
    else if(diag == rocblas_diagonal_unit)
    {
        diag_letter = "U";
    }

    if(test_pointer_mode == rocblas_pointer_mode_host)
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xger") << "," << m << "," << n << "," << alpha << ","
                   << (void*)dx << "," << incx << "," << (void*)dy << "," << incy << ","
                   << (void*)da << "," << lda;
        bench_ofs2 << "\n"
                   << "./rocblas-bench -f ger -r " << replaceX<T>("X") << " -m " << m << " -n " << n
                   << " --alpha " << alpha << " --incx " << incx << " --incy " << incy << " --lda "
                   << lda;
    }
    else
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xger") << "," << m << "," << n << "," << (void*)&alpha
                   << "," << (void*)dx << "," << incx << "," << (void*)dy << "," << incy << ","
                   << (void*)da << "," << lda;
    }

    if(test_pointer_mode == rocblas_pointer_mode_host)
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xsyr") << "," << uplo << "," << n << "," << alpha << ","
                   << (void*)dx << "," << incx << "," << (void*)da << "," << lda;
        bench_ofs2 << "\n"
                   << "./rocblas-bench -f syr -r " << replaceX<T>("X") << " --uplo " << uplo_letter
                   << " -n " << n << " --alpha " << alpha << " --incx " << incx << " --lda " << lda;
    }
    else
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xsyr") << "," << uplo << "," << n << "," << (void*)&alpha
                   << "," << (void*)dx << "," << incx << "," << (void*)da << "," << lda;
    }

    if(test_pointer_mode == rocblas_pointer_mode_host)
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xgemv") << "," << transA << "," << m << "," << n << ","
                   << alpha << "," << (void*)da << "," << lda << "," << (void*)dx << "," << incx
                   << "," << beta << "," << (void*)dy << "," << incy;

        bench_ofs2 << "\n"
                   << "./rocblas-bench -f gemv -r " << replaceX<T>("X") << " --transposeA "
                   << transA_letter << " -m " << m << " -n " << n << " --alpha " << alpha
                   << " --lda " << lda << " --incx " << incx << " --beta " << beta << " --incy "
                   << incy;
    }
    else
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xgemv") << "," << transA << "," << m << "," << n << ","
                   << (void*)&alpha << "," << (void*)da << "," << lda << "," << (void*)dx << ","
                   << incx << "," << (void*)&beta << "," << (void*)dy << "," << incy;
    }

    // BLAS3

    if(test_pointer_mode == rocblas_pointer_mode_host)
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xgeam") << "," << transA << "," << transB << "," << m
                   << "," << n << "," << alpha << "," << (void*)da << "," << lda << "," << beta
                   << "," << (void*)db << "," << ldb << "," << (void*)dc << "," << ldc;

        bench_ofs2 << "\n"
                   << "./rocblas-bench -f geam -r " << replaceX<T>("X") << " --transposeA "
                   << transA_letter << " --transposeB " << transB_letter << " -m " << m << " -n "
                   << n << " --alpha " << alpha << " --lda " << lda << " --beta " << beta
                   << " --ldb " << ldb << " --ldc " << ldc;
    }
    else
    {
        trace_ofs2 << "\n"
                   << replaceX<T>("rocblas_Xgeam") << "," << transA << "," << transB << "," << m
                   << "," << n << "," << (void*)&alpha << "," << (void*)da << "," << lda << ","
                   << (void*)&beta << "," << (void*)db << "," << ldb << "," << (void*)dc << ","
                   << ldc;
    }

    if(BUILD_WITH_TENSILE)
    {
        /* trsm calls rocblas_get_stream and rocblas_dgemm, so test it by comparing files
                if(test_pointer_mode == rocblas_pointer_mode_host)
                {
                    trace_ofs2 << "\n"
                               << replaceX<T>("rocblas_Xtrsm") << "," << side << "," << uplo
                               << "," << transA << "," << diag << "," << m
                               << "," << n << "," << alpha << "," << (void*)da << "," << lda
                               << "," << (void*)db << "," << ldb;

                    bench_ofs2 << "\n"
                               << "./rocblas-bench -f trsm -r " << replaceX<T>("X")
                               << " --side " << side_letter << " --uplo " << uplo_letter
                               << " --transposeA " << transA_letter << " --diag " << diag_letter
                               << " -m " << m << " -n " << n << " --alpha " << alpha
                               << " --lda " << lda << " --ldb " << ldb;
                }
                else
                {
                    trace_ofs2 << "\n"
                               << replaceX<T>("rocblas_Xtrsm") << "," << side << "," << uplo
                               << "," << transA << "," << diag << "," << m
                               << "," << n << "," << (void*)&alpha << "," << (void*)da << "," << lda
                               << "," << (void*)db << "," << ldb;
                }
        */
        if(test_pointer_mode == rocblas_pointer_mode_host)
        {
            trace_ofs2 << "\n"
                       << replaceX<T>("rocblas_Xgemm") << "," << transA << "," << transB << "," << m
                       << "," << n << "," << k << "," << alpha << "," << (void*)da << "," << lda
                       << "," << (void*)db << "," << ldb << "," << beta << "," << (void*)dc << ","
                       << ldc;

            bench_ofs2 << "\n"
                       << "./rocblas-bench -f gemm -r " << replaceX<T>("X") << " --transposeA "
                       << transA_letter << " --transposeB " << transB_letter << " -m " << m
                       << " -n " << n << " -k " << k << " --alpha " << alpha << " --lda " << lda
                       << " --ldb " << ldb << " --beta " << beta << " --ldc " << ldc;
        }
        else
        {
            trace_ofs2 << "\n"
                       << replaceX<T>("rocblas_Xgemm") << "," << transA << "," << transB << "," << m
                       << "," << n << "," << k << "," << (void*)&alpha << "," << (void*)da << ","
                       << lda << "," << (void*)db << "," << ldb << "," << (void*)&beta << ","
                       << (void*)dc << "," << ldc;
        }

        if(test_pointer_mode == rocblas_pointer_mode_host)
        {
            trace_ofs2 << "\n"
                       << replaceX<T>("rocblas_Xgemm_strided_batched") << "," << transA << ","
                       << transB << "," << m << "," << n << "," << k << "," << alpha << ","
                       << (void*)da << "," << lda << "," << bsa << "," << (void*)db << "," << ldb
                       << "," << bsb << "," << beta << "," << (void*)dc << "," << ldc << "," << bsc
                       << "," << batch_count;

            bench_ofs2 << "\n"
                       << "./rocblas-bench -f gemm_strided_batched -r " << replaceX<T>("X")
                       << " --transposeA " << transA_letter << " --transposeB " << transB_letter
                       << " -m " << m << " -n " << n << " -k " << k << " --alpha " << alpha
                       << " --lda " << lda << " --bsa " << bsa << " --ldb " << ldb << " --bsb "
                       << bsb << " --beta " << beta << " --ldc " << ldc << " --bsc " << bsc
                       << " --batch " << batch_count;
        }
        else
        {
            trace_ofs2 << "\n"
                       << replaceX<T>("rocblas_Xgemm_strided_batched") << "," << transA << ","
                       << transB << "," << m << "," << n << "," << k << "," << (void*)&alpha << ","
                       << (void*)da << "," << lda << "," << bsa << "," << (void*)db << "," << ldb
                       << "," << bsb << "," << (void*)&beta << "," << (void*)dc << "," << ldc << ","
                       << bsc << "," << batch_count;
        }
    }
    // exclude trtri as it is an internal function
    //  trace_ofs2 << "\n" << replaceX<T>("rocblas_Xtrtri")  << "," << uplo << "," << diag << "," <<
    //  n
    //  << "," << (void*)da << "," << lda << "," << (void*)db << "," << ldb;

    // Auxiliary function
    trace_ofs2 << "\nrocblas_destroy_handle";

    trace_ofs2.close();
    bench_ofs2.close();

    //
    // check if rocBLAS output files same as "golden files"
    //

    // construct iterators that check if files are same
    std::ifstream trace_ifs1;
    std::ifstream trace_ifs2;
    trace_ifs1.open(trace_path1);
    trace_ifs2.open(trace_path2);

    std::istreambuf_iterator<char> begin1(trace_ifs1);
    std::istreambuf_iterator<char> begin2(trace_ifs2);

    std::istreambuf_iterator<char> end;

    // check that files are the same
    verify_equal(true, range_equal(begin1, end, begin2, end), "log_trace file corrupt");

    if(test_pointer_mode == rocblas_pointer_mode_host)
    {
        // construct iterators that check if files are same
        std::ifstream bench_ifs1;
        std::ifstream bench_ifs2;
        bench_ifs1.open(bench_path1);
        bench_ifs2.open(bench_path2);

        std::istreambuf_iterator<char> begin1(bench_ifs1);
        std::istreambuf_iterator<char> begin2(bench_ifs2);

        std::istreambuf_iterator<char> end;

        // check that files are the same
        verify_equal(true, range_equal(begin1, end, begin2, end), "log_bench file corrupt");
        bench_ifs1.close();
        bench_ifs2.close();
    }

    trace_ifs1.close();
    trace_ifs2.close();

    char env_close_string[80] = "ROCBLAS_LAYER=0";
    verify_equal(putenv(env_close_string), 0, "failed to set environment variable ROCBLAS_LAYER=0");

    return;
}
