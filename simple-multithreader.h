#ifndef SIMPLE_MULTITHREADER_H
#define SIMPLE_MULTITHREADER_H

#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <chrono>
#include <cstdio> 

struct ThreadArgs1D {
  int low;
  int high;
  std::function<void(int)>* lambda;
};

void* worker1D(void* arg) {
  ThreadArgs1D* data = (ThreadArgs1D*)arg;
  for (int i = data->low; i < data->high; ++i) {
    (*(data->lambda))(i);
  }
  return NULL;
}


void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {
  auto start_time = std::chrono::high_resolution_clock::now();

  if (numThreads <= 0) {
    std::cerr << "Error: numThreads must be greater than 0\n";
    return;
  }

  if (low >= high) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
    printf("Execution time: %f ms\n", elapsed.count());
    return;
  }

  int total = high - low;

  if (numThreads == 1) {
    for (int i = low; i < high; ++i) {
      lambda(i);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
    printf("Execution time: %f ms\n", elapsed.count());
    return;
  }

  int numWorkers = numThreads - 1;
  pthread_t* threads = new pthread_t[numWorkers];
  ThreadArgs1D* args = new ThreadArgs1D[numWorkers];

  int chunkSize = total / numThreads;
  int remainder = total % numThreads;

  for (int i = 0; i < numWorkers; ++i) {
    int start = low + i * chunkSize + (i < remainder ? i : remainder);
    int end = start + chunkSize + (i < remainder ? 1 : 0);

    args[i].low = start;
    args[i].high = end;
    args[i].lambda = &lambda;
  }

  int createdCount = 0;
  for (int i = 0; i < numWorkers; ++i) {
    int rc = pthread_create(&threads[i], NULL, worker1D, &args[i]);
    if (rc != 0) {
      perror("Error: pthread_create failed");
      break;
    }
    createdCount++;
  }

  
  int mainStart = low + (numThreads - 1) * chunkSize + remainder;
  int mainEnd = high;

  for (int i = mainStart; i < mainEnd; ++i) {
    lambda(i);
  }

  for (int i = createdCount; i < numWorkers; ++i) {
    for (int j = args[i].low; j < args[i].high; ++j) {
      lambda(j);
    }
  }

  for (int i = 0; i < createdCount; ++i) {
    int rc = pthread_join(threads[i], NULL);
    if (rc != 0) {
      perror("Error: pthread_join failed");
    }
  }

  delete[] threads;
  delete[] args;

  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
  printf("Execution time: %f ms\n", elapsed.count());
}

struct ThreadArgs2D {
  int low1;
  int high1;
  int low2;
  int high2;
  std::function<void(int, int)>* lambda;
};

void* worker2D(void* arg) {
  ThreadArgs2D* data = (ThreadArgs2D*)arg;
  for (int i = data->low1; i < data->high1; ++i) {
    for (int j = data->low2; j < data->high2; ++j) {
      (*(data->lambda))(i, j);
    }
  }
  return NULL;
}

void parallel_for(int low1, int high1, int low2, int high2,
                  std::function<void(int, int)> &&lambda, int numThreads) {
  auto start_time = std::chrono::high_resolution_clock::now();

  if (numThreads <= 0) {
    std::cerr << "Error: numThreads must be greater than 0\n";
    return;
  }

  if (low1 >= high1 || low2 >= high2) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
    printf("Execution time: %f ms\n", elapsed.count());
    return;
  }

  int total1 = high1 - low1;

  if (numThreads == 1) {
    for (int i = low1; i < high1; ++i) {
      for (int j = low2; j < high2; ++j) {
        lambda(i, j);
      }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
    printf("Execution time: %f ms\n", elapsed.count());
    return;
  }

  int numWorkers = numThreads - 1;
  pthread_t* threads = new pthread_t[numWorkers];
  ThreadArgs2D* args = new ThreadArgs2D[numWorkers];

  int chunkSize = total1 / numThreads;
  int remainder = total1 % numThreads;

  for (int i = 0; i < numWorkers; ++i) {
    int start = low1 + i * chunkSize + (i < remainder ? i : remainder);
    int end = start + chunkSize + (i < remainder ? 1 : 0);

    args[i].low1 = start;
    args[i].high1 = end;
    args[i].low2 = low2;
    args[i].high2 = high2;
    args[i].lambda = &lambda;
  }

  int createdCount = 0;
  for (int i = 0; i < numWorkers; ++i) {
    int rc = pthread_create(&threads[i], NULL, worker2D, &args[i]);
    if (rc != 0) {
      perror("Error: pthread_create failed");
      break;
    }
    createdCount++;
  }
  int mainStart = low1 + (numThreads - 1) * chunkSize + remainder;
  int mainEnd = high1;

  for (int i = mainStart; i < mainEnd; ++i) {
    for (int j = low2; j < high2; ++j) {
      lambda(i, j);
    }
  }

  for (int i = createdCount; i < numWorkers; ++i) {
    for (int p = args[i].low1; p < args[i].high1; ++p) {
      for (int q = args[i].low2; q < args[i].high2; ++q) {
        lambda(p, q);
      }
    }
  }

  for (int i = 0; i < createdCount; ++i) {
    int rc = pthread_join(threads[i], NULL);
    if (rc != 0) {
      perror("Error: pthread_join failed");
    }
  }

  delete[] threads;
  delete[] args;

  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
  printf("Execution time: %f ms\n", elapsed.count());
}

int user_main(int argc, char **argv);


void demonstration(std::function<void()> && lambda) {
  lambda();
}

int main(int argc, char **argv) {

  int x=5,y=1;
  auto lambda1 = [x,&y](void) {
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
  };
  demonstration(lambda1);
  int rc = user_main(argc, argv);
  auto lambda2 = []() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main

#endif
