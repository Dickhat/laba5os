#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <iostream>
#include <queue>

// Определение структуры semun, если она отсутствует
union semun {
    int val;                // Значение для SETVAL
    struct semid_ds *buf;   // Буфер для IPC_STAT, IPC_SET
    unsigned short *array;  // Массивы для GETALL, SETALL
    struct seminfo *__buf;  // Буфер для IPC_INFO (Linux-specific)
};


int wait_sem(int sem_id); // Занятие семафора
int pass_sem(int sem_id); // Освобождение семафора
void * generating_babuins(void * params);// Создание бабуинов в очереди

int main(int argc, char* argv[])
{
    std::string babuins;            // Бабуины левые либо правые

    if(argc != 2)
    {
        std::cout << " Укажите 2 аргумента для программы\n";
        return -1;
    }
    else
    {
        // Название бабуинов
        babuins = argv[1];
    }

    int sem_id;                     // Идентификатор семафора
    std::queue<int> queue_ready;    // Очередь бабуинов
    std::queue<int> queue_going;    // Очередь, которая уже идет через каньон

    // Создание семафора
    sem_id = semget(304, 1, 0666 | IPC_CREAT);

    union semun sem_union;
    sem_union.val = 1;
    semctl(sem_id, 0, SETVAL, sem_union);   // Инициализация значения семафора 1

    pthread_t * thread = new pthread_t;
    pthread_create(thread, nullptr, generating_babuins, (void*) &queue_ready);

    // 10 Проходов бабуинов
    for(int i = 0; i < 10; ++i)
    {
        // Пока бабуинов нет - ждать
        while(queue_ready.size() == 0)
        {
            //std::cout << " Очередь пуста\n";
            sleep(1);
        }

        // Занятие семафора
        if(!wait_sem(sem_id))
        {
            exit(-1);   // Ошибка в работе семафора
        }

        bool exist_babuin = true;  // Бабуины есть в очереди
        while(exist_babuin)
        {
            // Перемещение всех готовых бабуинов в очередь "перемещающихся"
            while (queue_ready.size() != 0)
            {
                queue_going.push(queue_ready.front());      // Бабуин входит в очередь "перемещающихся"
                queue_ready.pop();                          // Бабуин выходит из очереди готовых идти
                std::cout<< " Идет бабуин "<< queue_going.back() << babuins << std::endl;
            }

            sleep(3);

            while(queue_going.size() != 0)
            {
                std::cout<< " Бабуин "<< queue_going.front() <<", идущий "<< babuins << ", закончил движение\n";
                queue_going.pop();
            }

            // Очередь "готовых" пуста - освободить семафор
            if(queue_ready.size() == 0) exist_babuin = false;
        }
        
        // Освобождение семафора
        if(!pass_sem(sem_id))
        {
            exit(-1);   // Ошибка в работе семафора
        }
    }

    pthread_cancel(*thread);    // Завершение потока
    semctl(sem_id, 0, IPC_RMID);// Удаление семафора

    return 0;
}

// Занятие семафора
int wait_sem(int sem_id)
{
    struct sembuf sem_b;        // Структрура для изменения значения семафора
    
    sem_b.sem_num = 0;          // 0 - если не работаем с массивом семафоров
    sem_b.sem_op = -1;          // Изменение значения семафора
    sem_b.sem_flg = SEM_UNDO;   // Отслеживание семафора операционной системой

    // Изменение значения семафора на величину -1
    if(semop(sem_id, &sem_b, 1) == -1)
    {
        std::cout << "Изменение значения семафора на -1 не успешно\n";
        return 0;
    }

    return 1;
}

// Освобождение семафора
int pass_sem(int sem_id)
{
    struct sembuf sem_b;        // Структрура для изменения значения семафора
    
    sem_b.sem_num = 0;          // 0 - если не работаем с массивом семафоров
    sem_b.sem_op = 1;           // Изменение значения семафора
    sem_b.sem_flg = SEM_UNDO;   // Отслеживание семафора операционной системой

    // Изменение значения семафора на величину 1
    if(semop(sem_id, &sem_b, 1) == -1)
    {
        std::cout << "Изменение значения семафора на 1 не успешно\n";
        return 0;
    }

    return 1;
}

// Создание бабуинов в очереди
void * generating_babuins(void * params)
{
    std::queue<int> * queue_ready = (std::queue<int>*) params;

    int number_babuing = 1; // Номер текущего бабуина

    // Генерация бабуинов в очередь
    while(true)
    {
        int generate_count = rand()% 4;    // Число бабуинов на очередь "готовых"

        //std::cout<<" Число "<< generate_count << "\n";
        //sleep(6);

        // Заполение очереди
        for(int i = 0; i < generate_count; ++i)
        {
            queue_ready->push(number_babuing);
            number_babuing++;
            //std::cout << " В очередь закинут бабуин\n";
        }
        
        sleep(5);
    }
}