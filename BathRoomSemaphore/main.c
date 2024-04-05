#include <stdio.h>                         // подключение стандартной библиотеки ввода/вывода                   
#include <unistd.h>                        // содержит символические константы и структуры, которые еще не были описаны в каких-либо других включаемых файлах
#include <pthread.h>                       // заголовочный файл, предназначенный для работы с потоками в многопоточном программировании
#include <stdlib.h>                        // подключаем заголовочный файл, который содержит в себе функции, занимающиеся выделением памяти, контролем процесса выполнения программы
#include "bathroom.h"                      // подключаем заголовочный файл с семафорами и прототипами                  

// свели к классической задаче синхронизации доступа к критическим секциям - "Задача производителя и потребителя"



/**************************************************************************/
/*   Г Л О Б А Л Ь Н Ы Е     П Е Р Е М Е Н Н Ы Е   */
/***************************************************/

sem_t empty;                               // семафор, указывающий на количество свободных мест в буфере-ванной комнате
sem_t full;                                // семафор, указывающий на количество занятых мест в буфере-ванной комнате
sem_t threadSemaphore;                     // семафор для синхронизации запуска потоков
sem_t mutex;                               // cемафор для обеспечения взаимоисключающего доступа к критической секции

int buffer[BUFFER_SIZE];                   // буфер для имитации ванной комнаты



/*------------------------------------------------------------------------*/
/*                Функции               */
/*--------------------------------------*/

/*----------------------------------------------------------*/
/*    Мужчина и ванная комната    */
/*--------------------------------*/
// функция для потока мужчины
void* male(void* arg)                      // аргументы, передаваемы в поток мужчины
{
    // ожидание разрешения на запуск потока
    // если планировщик выберет первым поток мужчины - он запустится после создания всех потоков
    sem_wait(&threadSemaphore);
    // далее разрешит запускаться всем остальным потокам
    sem_post(&threadSemaphore);

    // данные, переданные в аргументе функции
    int* data = (int*)arg; 
    int* in = &data[0];                    // указатель на индекс для добавления элементов в буфер-ванная комната
    int* out = &data[1];                   // указатель на индекс для удаления элементов из буфера-ванной комнаты
    int* count = &data[2];                 // указатель на количество людей в ванной комнате
    int* male_inside = &data[3];           // указатель на количество мужчин в ванной комнате 
    int* female_inside = &data[4];         // указатель на количество женщин в ванной комнате
    int* male_total = &data[5];            // указатель на общее количество мужчин
    int* female_total = &data[6];          // указатель на общее количество женщин
    int* turn = &data[7];                  // указатель на флаг очереди (0 - мужчины, 1 - женщины)

    // основной цикл работы потока отдельного мужчины
    while (1) 
    {
        // ожидание свободного места в буфере-ванной комнате
        sem_wait(&empty);
        // вход в критическую секцию - только один поток на данном этапе имеет к ней доступ
        sem_wait(&mutex);

        // если в ванной комнате есть женщины или если очередь за женщинами и ванной комнате нет никого
        if (*female_inside > 0 || (*turn == 1 && *female_inside == 0 && *male_inside == 0)) 
        {
            sem_post(&mutex);              // освобождаем критическую секцию - мужчина зайти не может
            sem_post(&empty);              // освобождаем зарезервированное место под мужчину в ванной комнате
            usleep(100);                   // кратковременная задержка для возможности переключения на другой поток и уменьшения вероятности зацикливания на одном и том же мужчине
            continue;                      // переходим к началу цикла для повторной возможности этого мужчины войти в ванную комнату
        }// if

        // очередной мужчина может зайти в буфер-ванную комнату
        // увеличиваем количество мужчин в буфере-ванной комнате и уменьшаем общее количество мужчин, которое должно войти
        *male_inside = *male_inside + 1;
        *male_total  = *male_total - 1;

        // если в ванную комнату вошел первый мужчина, и есть ожидающие женщины, устанавливаем флаг очереди для женщин. При освобождении ванной комнаты начнут заходить женщины
        if (*male_inside == 1 && *female_total != 0)
        {
            *turn = 1;                     // устанавливаем флаг, что при освобождении буфера-ванной комнаты в неё должны начать заходить женщины
        }// if

        // в ванной комнате не было людей и зашёл в неё мужчина - подкрашиваем это
        if (*male_inside == 1)
        {
            printf("\n\033[38;5;135mМ У Ж Ч И Н Ы   В Х О Д Я Т:\033[0m\n");
        }// if

        // добавляем элемент в буфер - очередной мужчина зашёл в ванную комнату
        buffer[*in] = 1; 
        // подкрашиваем, что мужчина вошел в ванную комнату
        printf("Мужчина вошел в ванную. Мужчин в ванной: %d, Женщин в ванной: %d\n", ++(*count), *female_inside);
        // обновляем индекс добавления - буфер кольцевой, то есть мы делим полученный индекс на размер буфера и берём остаток
        *in = (*in + 1) % BUFFER_SIZE;

        sem_post(&mutex);                  // после добавления очередного мужчины в ванную комнату разрешаем доступ к критической секции
        sem_post(&full);                   // сигнализируем о наличии элемента в буфере-ванной комнате

        usleep(400000);                    // имитация времени пребывания в ванной

        sem_wait(&full);                   // мужчина выходит из буфера-ванной комнаты. Уменьшаем его содержимое на 1. Если никого нет в ванной комнате из мужчин, то блокируем поток
        sem_wait(&mutex);                  // вход в критическую секцию. Только один поток может войти. Исключение гонок сигналов
 
        buffer[*out] = 0;                  // удаляем элемент из буфера-ванной комнаты
        // подкрашиваем, что мужчина вышел из ванной комнаты
        printf("Мужчина вышел из ванной. Мужчин в ванной: %d, Женщин в ванной: %d\n", --(*count), *female_inside);
        *male_inside = *male_inside - 1;   // уменьшаем количество мужчин в буфере-ванной комнате
        
        // если последний мужчина вышел из ванной комнаты и есть ожидающие женщины, устанавливаем флаг очереди для женщин
        if (*male_inside == 0 && *female_total != 0)
        {
            printf("\n\033[38;5;198mЖенщин\033[38;5;122m осталось в очереди: %i\033[0m\n", *female_total); // выводим, сколько женщин ожидают ванную комнату
            *turn = 1;                     // устанавливаем флаг, что при освобождении буфера-ванной комнаты в неё должны начать заходить женщины
        }// if

        *out = (*out + 1) % BUFFER_SIZE;   // обновляем индекс удаления с учетом того, что буфер-кольцевой

        sem_post(&mutex);                  // после ухода очередного мужчины из ванной комнаты разрешаем доступ к критической секции
        sem_post(&empty);                  // увеличиваем число свободных мест в буфере-ванной комнате
        break;                             // поток мужчины отработал - завершаем цикл

    }// while(1)  

    pthread_exit(NULL);                    // завершаем работу потока
    return NULL;
}



/*----------------------------------------------------------*/
/*    Женщина и ванная комната    */
/*--------------------------------*/
// функция для потока женщины
void* female(void* arg)                    // аргументы, передаваемы в поток женщины
{
    // ожидание разрешения на запуск потока
    // если планировщик выберет первым поток женщины - он запустится после создания всех потоков
    sem_wait(&threadSemaphore);
    // далее разрешит запускаться всем остальным потокам
    sem_post(&threadSemaphore);

    // данные, переданные в аргументе функции
    int* data = (int*)arg;
    int* in = &data[0];                    // указатель на индекс для добавления элементов в буфер-ванная комната
    int* out = &data[1];                   // указатель на индекс для удаления элементов из буфера-ванной комнаты
    int* count = &data[2];                 // указатель на количество людей в ванной комнате
    int* male_inside = &data[3];           // указатель на количество мужчин в ванной комнате 
    int* female_inside = &data[4];         // указатель на количество женщин в ванной комнате
    int* male_total = &data[5];            // указатель на общее количество мужчин
    int* female_total = &data[6];          // указатель на общее количество женщин
    int* turn = &data[7];                  // указатель на флаг очереди (0 - мужчины, 1 - женщины)

    // основной цикл работы потока отдельной женщины
    while (1) 
    {
        // ожидание свободного места в буфере-ванной комнате
        sem_wait(&empty);
        // вход в критическую секцию - только один поток на данном этапе имеет к ней доступ
        sem_wait(&mutex);

        // если в ванной комнате есть мужчины или если очередь за мужчинами и ванной комнате нет никого
        if (*male_inside > 0 || (*turn == 0 && *male_inside == 0 && *female_inside == 0)) 
        {
            sem_post(&mutex);              // освобождаем критическую секцию - женщина зайти не может
            sem_post(&empty);              // освобождаем зарезервированное место под женщину в ванной комнате
            usleep(100);                   // кратковременная задержка для возможности переключения на другой поток и уменьшения вероятности зацикливания на одной и той же женщине
            continue;                      // переходим к началу цикла для повторной возможности этой женщины войти в ванную комнату
        }// if

        // очередная женщина может зайти в буфер-ванную комнату
        // увеличиваем количество женщин в буфере-ванной комнате и уменьшаем общее количество женщин, которое должно войти
        *female_inside = *female_inside + 1;
        *female_total  = *female_total - 1;
        
        // если в ванную вошла первая женщина и есть ожидающие мужчины, устанавливаем флаг очереди для мужчин. При освобождении ванной комнаты начнут заходить мужчины
        if (*female_inside == 1 && *male_total != 0)
        {
            *turn = 0;                     // устанавливаем флаг, что при освобождении буфера-ванной комнаты в неё должны начать заходить мужчины
        }// if

        // в ванной комнате не было людей и зашла в неё женщина - подкрашиваем это
        if (*female_inside == 1)
        {
            printf("\n\033[38;5;198mЖ Е Н Щ И Н Ы   В Х О Д Я Т:\033[0m\n");
        }// if

        // добавляем элемент в буфер - очередная женщина зашла в ванную комнату
        buffer[*in] = 2;
        // подкрашиваем, что женщина вошла в ванную комнату
        printf("Женщина вошла в ванную. Мужчин в ванной: %d, Женщин в ванной: %d\n", *male_inside, ++(*count));
        // обновляем индекс добавления - буфер кольцевой, то есть мы делим полученный индекс на размер буфера и берём остаток
        *in = (*in + 1) % BUFFER_SIZE;

        sem_post(&mutex);                  // после добавления очередной женщины в ванную комнату разрешаем доступ к критической секции
        sem_post(&full);                   // сигнализируем о наличии элемента в буфере-ванной комнате

        usleep(400000);                    // имитация времени пребывания в ванной

        sem_wait(&full);                   // женщина выходит из буфера-ванной комнаты. Уменьшаем его содержимое на 1. Если никого нет в ванной комнате из женщин, то блокируем поток
        sem_wait(&mutex);                  // вход в критическую секцию. Только один поток может войти. Исключение гонок сигналов

        buffer[*out] = 0;                  // удаляем элемент из буфера-ванной комнаты
        // подкрашиваем, что женщина вышла из ванной комнаты
        printf("Женщина вышла из ванной. Мужчин в ванной: %d, Женщин в ванной: %d\n", *male_inside, --(*count));
        // уменьшаем количество женщин в буфере-ванной комнате
        *female_inside = *female_inside - 1;
        
        // если последняя женщина вышла из ванной комнаты и есть ожидающие мужчины, устанавливаем флаг очереди для мужчин
        if (*female_inside == 0 && *male_total != 0)
        {
            printf("\n\033[38;5;135mМужчин\033[38;5;122m осталось в очереди: %i\033[0m\n", *male_total);   // выводим, сколько мужчин ожидают ванную комнату
            *turn = 0;                     // устанавливаем флаг, что при освобождении буфера-ванной комнаты в неё должны начать заходить мужчины
        }// if

        *out = (*out + 1) % BUFFER_SIZE;   // обновляем индекс удаления с учетом того, что буфер-кольцевой

        sem_post(&mutex);                  // после ухода очередной женщины из ванной комнаты разрешаем доступ к критической секции
        sem_post(&empty);                  // увеличиваем число свободных мест в буфере-ванной комнате
        break;                             // поток женщины отработал - завершаем цикл
    
    }// while(1) 

    pthread_exit(NULL);                    // завершаем работу потока
    return NULL;
}



/*----------------------------------------------------------*/
/*    Привестственная надпись     */
/*--------------------------------*/
// печать приветственного ASCII-арта на экран
void print_start()
{
    // выводим ASCII-арт начала
    printf("\n\033[38;5;219m    ,-,--.       ,----.          ___      ,---.           _ __     ,--.-,,-,--,     _,.---._                        ,----.\n");
    printf("  ,-.'-  _\\   ,-.--` , \\  .-._ .'=.'\\   .--.'  \\       .-`.' ,`.  /==/  /|=|  |   ,-.' , -  `.     .-.,.---.     ,-.--` , \\\n");
    printf(" /==/_ ,_.'  |==|-  _.-` /==/ \\|==|  |  \\==\\-/\\ \\     /==/, -   \\ |==|_ ||=|, |  /==/_,  ,  - \\   /==/  `   \\   |==|-  _.-`\n");
    printf(" \\==\\  \\     |==|   `.-. |==|,|  / - |  /==/-|_\\ |   |==| _ .=. | |==| ,|/=| _| |==|   .=.     | |==|-, .=., |  |==|   `.-.\n");
    printf("  \\==\\ -\\   /==/_ ,    / |==|  \\/  , |  \\==\\,   - \\  |==| , '=',| |==|- `-' _ | |==|_ : ;=:  - | |==|   '='  / /==/_ ,    /\n");
    printf("  _\\==\\ ,\\  |==|    .-'  |==|- ,   _ |  /==/ -   ,|  |==|-  '..'  |==|  _     | |==| , '='     | |==|- ,   .'  |==|    .-'\n");
    printf(" /==/\\/ _ | |==|_  ,`-._ |==| _ /\\   | /==/-  /\\ - \\ |==|,  |     |==|   .-. ,\\  \\==\\ -    ,_ /  |==|_  . ,'.  |==|_  ,`-._\n");
    printf(" \\==\\ - , / /==/ ,     / /==/  / / , / \\==\\ _.\\=\\.-' /==/ - |     /==/, //=/  |   '.='. -   .'   /==/  /\\ ,  ) /==/ ,     /\n");
    printf("  `--`---'  `--`-----``  `--`./  `--`   `--`         `--`---'     `--`-' `-`--`     `--`--''     `--`-`--`--'  `--`-----`` \033[0m\n\n");
    return;                                    // вернули обещанное функцией значение             
}



/*----------------------------------------------------------*/
/*    Прощальная надпись     */
/*---------------------------*/
// печать прощального ASCII-арта на экран
void print_end()
{
    // выводим ASCII-арт начала
    printf("\n\033[38;5;219m\t\t ,--.--------.   ,--.-,,-,--,      ,----.             ,----.   .-._\n");
    printf("\t\t/==/,  -   , -\\ /==/  /|=|  |   ,-.--` , \\         ,-.--` , \\ /==/ \\  .-._    _,..---._\n");
    printf("\t\t\\==\\.-.  - ,-./ |==|_ ||=|, |  |==|-  _.-`        |==|-  _.-` |==|, \\/ /, / /==/,   -  \\\n");
    printf("\t\t `--`\\==\\- \\    |==| ,|/=| _|  |==|   `.-.        |==|   `.-. |==|-  \\|  |  |==|   _   _\\\n");
    printf("\t\t      \\==\\_ \\   |==|- `-' _ | /==/_ ,    /       /==/_ ,    / |==| ,  | -|  |==|  .=.   |\n");
    printf("\t\t      |==|- |   |==|  _     | |==|    .-'        |==|    .-'  |==| -   _ |  |==|,|   | -|\n");
    printf("\t\t      |==|, |   |==|   .-. ,\\ |==|_  ,`-._       |==|_  ,`-._ |==|  /\\ , |  |==|  '='   /\n");
    printf("\t\t      /==/ -/   /==/, //=/  | /==/ ,     /       /==/ ,     / /==/, | |- |  |==|-,   _`/\n");
    printf("\t\t      `--`--`   `--`-' `-`--` `--`-----``        `--`-----``  `--`./  `--`  `-.`.____.'\033[0m\n\n");
    return;                                    // вернули обещанное функцией значение             
}



/*----------------------------------------------------------*/
/*         Очистка буфера ввода          */
/*---------------------------------------*/
// очистка буфера ввода
void clearInputBuffer() 
{
    int c;

    // чтение символов из входного буфера до тех пор, пока не будет достигнут символ новой строки ('\n') или конец файла (EOF) 
    while ((c = getchar()) != '\n' && c != EOF) { }

    // символ равен новой строке или концу файла, while-цикл завершается. Управление передаётся основной функции
    return;
}



/**************************************************************/
/*            О С Н О В Н А Я   П Р О Г Р А М М А             */
/**************************************************************/

int main() 
{
    system("clear");                       // очистка консоли перед началом работы программы
    print_start();                         // печатаем приветственную надпись в начале программы

    int num_males;                         // количество мужчин, которые хотят попасть в ванную комнату
    int num_females;                       // количество женщин, которые хотят попасть в ванную комнату

    // работа с пользователем
    printf("\n\033[38;5;122m\t\t\t\t\t П О Л Ь З О В А Т Е Л Ь С К И Й   В В О Д\033[0m\n\n");
    
    // пользователь самостоятельно определяет количество мужчин и женщин, которые хотят попасть в ванную комнату
    printf("Введите количество мужчин: ");
    scanf("%d", &num_males);               // ввод пользователем числа мужчин с клавиатуры
    pthread_t males[num_males];            // создаём массив потоков мужчин с количеством элементов, заданных пользователем

    printf("Введите количество женщин: ");
    scanf("%d", &num_females);             // ввод пользователем числа женщин с клавиатуры
    pthread_t females[num_females];        // создаём массив потоков женщин с количеством элементов, заданных пользователем

    // инициализация данных, используемых каждым потоком
    int in = 0;                            // индекс для записи элемента в буфер при входе человека 
    int out = 0;                           // индекс для записи элемента в буфер при выходе человека  
    int count = 0;                         // количество людей в ванной комнате
    int male_inside = 0;                   // количество мужчин в ванной комнате
    int female_inside = 0;                 // количество женщин в ванной комнате
    int male_total = num_males;            // сколько всего мужчин должно войти в ванную комнату
    int female_total = num_females;        // сколько всего женщин должно воти в ванную комнату
    int turn = 0;                          // переменная, определяющая приоритет при освобождении ванной комнаты (0 для мужчин, 1 для женщин)

    // помещаем все эти переменные в единый массив для передачи каждому потоку
    int data[8] = {in, out, count, male_inside, female_inside, male_total, female_total, turn};

    // инициализация семафоров
    sem_init(&empty, 0, BUFFER_SIZE);      // изначально все места в ванной комнате пустые
    sem_init(&full, 0, 0);                 // изначально ни одного занятого места в ванной комнате нет
    sem_init(&mutex, 0, 1);                // в ванной комнате никого нет - разрешается в неё доступ
    sem_init(&threadSemaphore, 0, 0);      // пока не созданы все потоки (отдельные люди), никто в ванную комнату зайти не может

    // работа с потоками
    printf("\n\033[38;5;215m\t\t\t\t\t     Р А Б О Т А   С   П О Т О К А М И\033[0m\n\n");
    
    // cоздание потоков для мужчин
    for (int i = 0; i < num_males; i++)
    {
        // &males[i]: Это адрес переменной типа pthread_t, куда будет сохранен идентификатор созданного потока
        // NULL: Это атрибуты по умолчанию для создания потока
        // male: Это указатель на функцию, которая будет выполнена в новом потоке
        // data: Это данные, которые будут переданы в функцию male в качестве аргумента
        pthread_create(&males[i], NULL, male, data);
    }// for i

    // создание потоков для женщин
    for (int i = 0; i < num_females; i++)
    {
        // &females[i]: Это адрес переменной типа pthread_t, куда будет сохранен идентификатор созданного потока
        // NULL: Это атрибуты по умолчанию для создания потока
        // female: Это указатель на функцию, которая будет выполнена в новом потоке
        // data: Это данные, которые будут переданы в функцию female в качестве аргумента
        pthread_create(&females[i], NULL, female, data);
    }// for i

    // разрешение запуска потоков после создания всех необходимых потоков
    sem_post(&threadSemaphore);

    // ожидание завершения потоков для мужчин
    for (int i = 0; i < num_males; i++)
    {
        // males[i] представляет идентификатор потока, который ожидается для завершения
        // NULL указывает на то, что не требуется возвращать значение, возвращаемое потоком
        pthread_join(males[i], NULL);
    }// for i

    // ожидание завершения потоков для женщин
    for (int i = 0; i < num_females; i++)
    {
        // females[i] представляет идентификатор потока, который ожидается для завершения
        // NULL указывает на то, что не требуется возвращать значение, возвращаемое потоком
        pthread_join(females[i], NULL);
    }// for i

    // уничтожение семафоров
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);

    printf("\n\033[38;5;226mМ О Д Е Л И Р О В А Н И Е   В Ы П О Л Н Е Н О   У С П Е Ш Н О!   В С Е   Ч И С Т Ы Е\033[0m\n");
    print_end();                           // печатаем завершающую надпись
    clearInputBuffer();                    // чистим буфер ввода перед ожиданием нажатия клавиши
    getchar();                             // ожидаем нажатия клавиши пользователем перед завершением программы
    return 0;                              // программа завершена: возвращаем обещанное значение
}