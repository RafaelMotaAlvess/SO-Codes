#include <stdio.h>
#include <pthread.h>
#include "utils.h"

pthread_mutex_t stdout_mutex;

void logError(const char* message) {
    pthread_mutex_lock(&stdout_mutex);
    perror(message);
    pthread_mutex_unlock(&stdout_mutex);
}
