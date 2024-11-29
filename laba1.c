#include <stdio.h>
#include <pthread.h>
#include <unistd.h> 


pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER; // �������� ���������� ��� ������������
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // ������� ��� ������ ����� ������
int ready = 0; // ����, �����������, ����� �� ������ � ���������


void* producer(void* arg) {
    // ���������� 5 ���������, ������ �� ������� �������������� ��������
    for (int i = 0; i <= 5; i++) { 
        sleep(1); 
        pthread_mutex_lock(&lock); 

        // ���� ������������� ��� ������� ������, ���������� � ��������� ��������
        if (ready == 1) {
            pthread_mutex_unlock(&lock);  
            continue;
        }

        
        if (i == 5) {
            ready = -1; // ��������� ������, ����� ������� �� ����������
            printf("�������������: �������� ���������� �������.\n");
        } else {
            ready = 1; // ������������� ���� ����������
            printf("�������������: ������ %d ���������.\n", i + 1);
        }

        
        pthread_cond_signal(&cond1); 
        pthread_mutex_unlock(&lock); 
    }
    return NULL;
}

// ������� �����������
void* consumer(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock); 

        
        while (ready == 0) {
            pthread_cond_wait(&cond1, &lock); // ������� ������ � ����������� �������
        }

        // ���� ���� ���������� ����� -1, ������������� �������� ������, ����������� ��������� ���� ������
        if (ready == -1) {
            printf("�����������: ��������� ���������� �������. ���������� ������.\n");
            pthread_mutex_unlock(&lock); 
            break; 
        }

        
        printf("�����������: ��������� �������.\n");
        ready = 0; // ���������� ���� ����������, ����� �������, ��� ������ ��� ���������

        pthread_mutex_unlock(&lock); 
    }
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread;

    // ������� ������ ������������� � �����������
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // ������� ���������� ������ ����� �������
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // ������� ������� � �������� ����������
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond1);

    // ��������� ��������� � ���������� ���������
    printf("��������� ���������.\n");
    return 0;
}