# Разделение ресурсов. Организация критической секции
---
## Задание: 
В работе необходимо написать 2 программы для реализации критической секции в соответствии с вариантом задания. 
- **Программа 1.** Критическую секцию первой программы реализовать на семафорах;
- **Программа 2.** Критическую секцию второй программы реализовать с применением мониторов. 
Реализовать вывод на  экран информации о попытках зайти в критическую секцию, успешном заходе и выходе из критической секции.

## Вариант:
Варианты в словесном описании программы, для которой необходимо реализовать много поточное приложение. Каждый поток реализует одну сущность из
предложенного варианта. Все задания можно свести к классическим задачам синхронизации.

В общежитии института имеется совместная ванная комната. Если в
ванной комнате есть женщина, то другая женщина может туда войти, а мужчина не
может, и наоборот. На ванной есть индикатор, показывающий, в каком из трех
состояний находится ванная: 
1) никого нет;
2) в ванной женщины;
3) в ванной мужчины.

Напишите систему, контролирующую доступ в ванную.
