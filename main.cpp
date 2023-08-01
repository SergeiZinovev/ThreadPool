#include <future>
#include <chrono>
#include <iostream>
#include <random>
#include "ThreadPool.h"

using namespace std;
bool make_thread = false;

RequestHandler pool;


void PrintArr(int* arr, unsigned int arrsize){
    cout << endl;
    for (int i = 0; i < arrsize; i+=1000000) {
        cout << arr[i] << " ";
}
    cout << endl;
}

void quicksort(int* array, int left, int right) {
    if (left >= right) return;

    int left_bound = left;
    int right_bound = right;

    int middle = array[(left_bound + right_bound) / 2];

    do {
        while (array[left_bound] < middle) {
            left_bound++;
        }
        while (array[right_bound] > middle) {
            right_bound--;
        }

        //Swapping elements
        if (left_bound <= right_bound) {
            std::swap(array[left_bound], array[right_bound]);
            left_bound++;
            right_bound--;
        }
    } while (left_bound <= right_bound);

    if (make_thread && (right_bound - left > 100000))
    {
        pool.pushRequest(quicksort, array, left, right_bound);
        quicksort(array, left_bound, right);
     
      
    }
    else {
        // run both parts synchronously
        quicksort(array, left, right_bound);
        quicksort(array, left_bound, right);
    }
}

int main() {
    cout << "Number of cores: " << thread::hardware_concurrency() << endl;

    time_t start, end;
    //single thread sorting
    time(&start);
   unsigned long arrsize = 40'000'000;
    int* arr = new int[arrsize];
    for (int i = 0; i < arrsize; i++) {
        arr[i] = rand() / 100;
    }
    cout << "array constructed" << endl;
    cout << endl;
   PrintArr(arr, arrsize);

    cout << "single thread sorting..." << endl;
    quicksort(arr, 0, arrsize - 1);

    cout << "array sorted by a single thread" << endl;
    PrintArr(arr, arrsize);
    //calculate the operation time
    time(&end);
    double seconds = difftime(end, start);
    printf("The time: %f seconds\n", seconds);

    //Multi-threaded sorting
    time(&start);
    make_thread = true;
    for (unsigned int i = 0; i < arrsize; i++) {
        arr[i] = rand() / 100;
    }
    cout << "array constructed" << endl;
   PrintArr(arr, arrsize);
    cout << endl;

    cout << "array pool thread sorting..." << endl;
   quicksort(arr, 0, arrsize - 1);
    cout << "array sorted by pool thread" << endl;
    
   PrintArr(arr, arrsize);
   //calculate the operation time
    time(&end);
    seconds = difftime(end, start);
    printf("The time: %f seconds\n", seconds);

    delete[] arr;
    return 0;
}
