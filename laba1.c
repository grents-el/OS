#include <stdio.h>
#include <pthread.h>
#include <unistd.h> 


pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER; // Условная переменная для сигнализации
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // Мьютекс для защиты общих данных
int ready = 0; // Флаг, указывающий, готов ли объект к обработке


void* producer(void* arg) {
    // Производим 5 элементов, каждый из которых сопровождается сигналом
    for (int i = 0; i <= 5; i++) { 
        sleep(1); 
        pthread_mutex_lock(&lock); 

        // Если производитель уже передал объект, продолжаем к следующей итерации
        if (ready == 1) {
            pthread_mutex_unlock(&lock);  
            continue;
        }

        
        if (i == 5) {
            ready = -1; // Финальный сигнал, чтобы указать на завершение
            printf("Производитель: Отправка последнего сигнала.\n");
        } else {
            ready = 1; // Устанавливаем флаг готовности
            printf("Производитель: Сигнал %d отправлен.\n", i + 1);
        }

        
        pthread_cond_signal(&cond1); 
        pthread_mutex_unlock(&lock); 
    }
    return NULL;
}

// Функция потребителя
void* consumer(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock); 

        
        while (ready == 0) {
            pthread_cond_wait(&cond1, &lock); // Ожидаем сигнал и освобождаем мьютекс
        }

        // Если флаг готовности равен -1, производитель завершил работу, потребитель завершает свою работу
        if (ready == -1) {
            printf("Потребитель: Обработка последнего сигнала. Завершение работы.\n");
            pthread_mutex_unlock(&lock); 
            break; 
        }

        
        printf("Потребитель: Обработка сигнала.\n");
        ready = 0; // Сбрасываем флаг готовности, чтобы указать, что объект был обработан

        pthread_mutex_unlock(&lock); 
    }
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread;

    // Создаем потоки производителя и потребителя
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // Ожидаем завершения работы обоих потоков
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // Очищаем мьютекс и условную переменную
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond1);

    // Финальное сообщение о завершении программы
    printf("Программа завершена.\n");
    return 0;
}