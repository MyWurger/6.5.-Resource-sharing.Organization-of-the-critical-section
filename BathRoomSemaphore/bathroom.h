#ifndef BATHROOM_H       // если макрос BATHROOM_H еще не определен
#define BATHROOM_H       // определить макрос BATHROOM_H

#include <semaphore.h>   // включение заголовочного файла для работы с семафорами
#define BUFFER_SIZE 3    // определение размера буфера - максимальное количество человек в душевой комнате


/**************************************************************************/
/*   Г Л О Б А Л Ь Н Ы Е    К О Н С Т А Н Т Ы   И   П Е Р Е М Е Н Н Ы Е   */
/**************************************************************************/

extern sem_t empty;      // семафор для контроля пустых мест в душевой комнате
extern sem_t full;       // семафор для контроля заполненных мест в душевой комнате
extern sem_t threadSemaphore;   // семафор для синхронизации запуска потоков
extern sem_t mutex;      // семафор для обеспечения доступа к критической секции входа и выхода человека из душевой комнаты


/**************************************************************************/
/*                  П Р О Т О Т И П Ы   Ф У Н К Ц И Й                     */
/**************************************************************************/

// каждый мужчина и каждая женщина - отдельный поток
void* male(void* arg);   // функция для потока мужчины
void* female(void* arg); // функция для потока женщины
void print_start();      // печать приветственной надписи на экран
void print_end();        // печать прощальной надписи на экран
void clearInputBuffer(); // очистка буфера ввода

#endif /* BATHROOM_H */  // завершение условия препроцессора