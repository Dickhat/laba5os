#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <queue>

int wait_sem(int sem_id); // Занятие семафора
int pass_sem(int sem_id); // Освобождение семафора

int main()
{
    int sem_id;                     // Идентификатор семафора
    std::queue<int> queue_ready;    // Очередь бабуинов
    std::queue<int> queue_going;    // Очередь, которая уже идет через каньон

    // Создание семафора
    sem_id = semget(304, 1, 0666 | IPC_CREAT);

    // 10 Проходов бабуинов
    for(int i = 0; i < 10; ++i)
    {
        // Пока бабуинов нет - ждать
        while(queue_ready.size() == 0)
        {
            //sleep(1);
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
                std::cout<< " Идет бабуин "<< queue_going.back() << " слева\n";
            }
            //sleep(2);
            std::cout<< " Бабуин (number) слева закончил движение\n";
            // Очередь пуста - освободить семафор
            if(queue_ready.size() == 0) exist_babuin = false;
        }
        
        // Освобождение семафора
        if(!pass_sem(sem_id))
        {
            exit(-1);   // Ошибка в работе семафора
        }
    }

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