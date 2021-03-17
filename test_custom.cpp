//
// Created by fred1 on 2020/11/25.
//

#include <cstdio>
#include <iostream>
#include <chrono>
#include <vector>
#include "prof.h"

#define PROF_USER_EVENTS_ONLY
#define PROF_EVENT_LIST \
       PROF_EVENT_CACHE(L1D, READ, MISS) \
       PROF_EVENT_CACHE(L1D, WRITE, MISS)


using namespace std;

//求和函数，行优先
int sum_row_first(const vector<vector<int>> &data) {
    int sum = 0;
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            sum += data[i][j];
        }
    }
    return sum;
}

//求和函数，列优先
int sum_col_first(const vector<vector<int>> &data) {
    int sum = 0;
    for (int j = 0; j < 1024; j++) {
        for (int i = 0; i < 1024; i++) {
            sum += data[i][j];
        }
    }
    return sum;
}

int main() {

    // Data initial.
    vector<vector<int>> data(1024, vector<int>(1024, 0));
    //初始化二维数组
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            data[i][j] = rand() % 10;
        }
    }

    chrono::steady_clock::time_point start_time = chrono::steady_clock::now();
    sum_row_first(data);  // 计算行优先求和函数耗时
    chrono::steady_clock::time_point stop_time = chrono::steady_clock::now();
    chrono::duration<double> time_span = chrono::duration_cast<chrono::microseconds>(stop_time - start_time);
    std::cout << "行优先耗时：" << time_span.count() << " ms" << endl;

    start_time = chrono::steady_clock::now();
    sum_col_first(data); //计算列优先求和函数耗时
    stop_time = chrono::steady_clock::now();
    time_span = chrono::duration_cast<chrono::microseconds>(stop_time - start_time);
    std::cout << "列优先耗时：" << time_span.count() << " ms" << endl;

    // Perf setting.
    uint64_t faults_slow[2] = {0, 0};

    PROF_START();
    // slow code goes here...
    sum_row_first(data);
    PROF_DO(faults_slow[index] += counter);

    // fast or uninteresting code goes here...

    // PROF_START();
    // slow code goes here...
    PROF_DO(faults_slow[index] += counter);

    printf("Total L1 faults_slow: R = %lu; W = %lu\n", faults_slow[0], faults_slow[1]);


    uint64_t faults_fast[2] = {0, 0};

    PROF_START();
    // slow code goes here...
    sum_col_first(data);
    PROF_DO(faults_fast[index] += counter);

    // fast or uninteresting code goes here...

    // PROF_START();
    // slow code goes here...
    PROF_DO(faults_fast[index] += counter);

    printf("Total L1 faults_fast: R = %lu; W = %lu\n", faults_fast[0], faults_fast[1]);
}