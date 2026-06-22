#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>

#define NUM_THREADS 32
#define ITERATIONS 100000
#define FIRST_PHASE_TRIES 10    //first phase에서 CAS spin을 시도하는 횟수

volatile int lock_flag = 0;
long long counter = 0;

long long lock_count = 0;
long long unlock_count = 0;
long long total_lock_wait_ns = 0;
long long sleep_count = 0;

//condition variable 대기/깨우기 흐름을 위한 보조 mutex와 condition variable
pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wait_cond = PTHREAD_COND_INITIALIZER;

//두 time 사이의 시간 ns 단위로 계산
long long get_ns_between(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
}

//timeval 구조체 값을 초 단위로 변환
double timeval_to_sec(struct timeval tv)
{
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

//CPU 사용 시간 계산
double get_cpu_time(struct rusage start, struct rusage end)
{
    double start_cpu = timeval_to_sec(start.ru_utime) + timeval_to_sec(start.ru_stime);
    double end_cpu = timeval_to_sec(end.ru_utime) + timeval_to_sec(end.ru_stime);
    return end_cpu - start_cpu; //사용자 및 커널 영역에서의 time으로 계산
}

//expected와 같으면 new로 바꾸고, 실제 기존 값을 actual로 반환
int CompareAndSwap(volatile int* ptr, int expected, int new)
{
    int actual = __sync_val_compare_and_swap(ptr, expected, new);
    return actual;
}

//Two-phase lock: 먼저 spin을 시도하고, 실패하면 condition variable에서 대기
void lock(void)
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) {
        //first phase: CompareAndSwap으로 lock 획득을 10번 시도
        for (int i = 0; i < FIRST_PHASE_TRIES; i++) {
            if (CompareAndSwap(&lock_flag, 0, 1) == 0) {
                clock_gettime(CLOCK_MONOTONIC, &end);
                __sync_fetch_and_add(&lock_count, 1);
                //lock 진입 시점부터 실제 획득 시점까지의 시간을 누적 저장
                __sync_fetch_and_add(&total_lock_wait_ns, get_ns_between(start, end));
                return;
            }
        }

        //second phase: lock이 풀릴 때까지 sleep 상태로 대기
        __sync_fetch_and_add(&sleep_count, 1);

        pthread_mutex_lock(&wait_mutex);
        while (lock_flag == 1) {
            pthread_cond_wait(&wait_cond, &wait_mutex);
        }
        pthread_mutex_unlock(&wait_mutex);
    }
}

//lock 해제하고 unlock 횟수를 기록
void unlock(void)
{
    __sync_fetch_and_add(&unlock_count, 1);
    __sync_lock_release(&lock_flag);

    //lock이 풀렸으므로 대기 중인 스레드 하나를 깨움
    pthread_mutex_lock(&wait_mutex);
    pthread_cond_signal(&wait_cond);
    pthread_mutex_unlock(&wait_mutex);
}

//counter를 ITERATIONS번 증가시키는 함수
void* worker(void* arg)
{
    (void)arg;

    for (int i = 0; i < ITERATIONS; i++) {
        lock();
        //critical section
        counter++;
        unlock();
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];
    struct timespec start_time, end_time;
    struct rusage cpu_start, cpu_end;

    counter = 0;
    lock_flag = 0;
    lock_count = 0;
    unlock_count = 0;
    total_lock_wait_ns = 0;
    sleep_count = 0;

    //전체 수행 시간과 CPU 사용 시간을 측정하기 위한 측정 시작 시점
    getrusage(RUSAGE_SELF, &cpu_start);
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    //스레드 생성
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, worker, NULL) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    //Join
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }
    }

    //측정 종료 시점
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    getrusage(RUSAGE_SELF, &cpu_end);

    //분석값 계산
    double total_runtime = (double)get_ns_between(start_time, end_time) / 1000000000.0;
    double cpu_sec = get_cpu_time(cpu_start, cpu_end);
    double cpu_usage = total_runtime > 0.0 ? (cpu_sec / total_runtime) * 100.0 : 0.0;
    double total_wait_sec = (double)total_lock_wait_ns / 1000000000.0;

    //분석 결과 출력
    printf("Lock Type: TWO\n");
    printf("NUM_THREADS: %d\n", NUM_THREADS);
    printf("Counter: %lld\n", counter);
    printf("Total Runtime(sec): %.9f\n", total_runtime);
    printf("CPU Usage(%%): %.2f\n", cpu_usage);
    printf("Lock Count: %lld\n", lock_count);
    printf("Unlock Count: %lld\n", unlock_count);
    printf("Total Lock Wait Time(sec): %.9f\n", total_wait_sec);
    printf("Sleep Count: %lld\n", sleep_count);

    return EXIT_SUCCESS;
}