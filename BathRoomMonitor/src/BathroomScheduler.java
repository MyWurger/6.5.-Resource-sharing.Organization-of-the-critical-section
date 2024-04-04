import java.util.Scanner;                           // импортируем класс Scanner для считывания ввода от пользователя


public class BathroomScheduler

{
    private static final int BUFFER_SIZE = 3;       // глобальная константа, которая определяет размер буфера, который будет использоваться для отслеживания доступа к ванной комнате
    private int[] buffer = new int[BUFFER_SIZE];    // объявление и выделение памяти "в одном флаконе" под массив для хранения информации о том, кто находится в ванной комнате (0 - никого, 1 - мужчина, 2 - женщина)
    private int in = 0;                             // индекс для добавления (in) записи о входе человека в ванную комнату  
    private int out = 0;                            // индекс для добавления (out) записи о выходе человека из ванной комнаты
    private int count = 0;                          // счётчик для отслеживания количества человек в ванной комнате
    private int male_inside = 0;                    // текущее количество мужчин в ванной комнате
    private int female_inside = 0;                  // текущее количество женщин в ванной комнате
    private int male_total;                         // общее количество мужчин, которые должны посетить ванную комнату
    private int female_total;                       // общее количество женщин,которые должны посетить ванную комнату
    private boolean turn = true;                    // флаг, определяющий, чья очередь сейчас (true - мужчин, false - женщин)

    public BathroomScheduler(int maleTotal, int femaleTotal) 
    {
        this.male_total = maleTotal;
        this.female_total = femaleTotal;
    }

    public synchronized void enterMale() throws InterruptedException {
        while (female_inside > 0 || (turn == false && male_inside == 0) || count == BUFFER_SIZE) {
            wait();
        }
        male_inside++;
        male_total--;
        count++;
        if (male_inside == 1 && female_total > 0) {
            turn = false;
        }
        buffer[in] = 1;
        in = (in + 1) % BUFFER_SIZE;
        System.out.println("Мужчина вошел в ванную. Мужчин в ванной: " + count + ", Женщин в ванной: " + female_inside);
        notifyAll();
    }

    public synchronized void exitMale() {
        male_inside--;
        count--;
        buffer[out] = 0;
        out = (out + 1) % BUFFER_SIZE;
        System.out.println("Мужчина вышел из ванной. Мужчин в ванной: " + count + ", Женщин в ванной: " + female_inside);
        if (male_inside == 0 && female_total > 0) {
            System.out.println("Женщин в очереди: " + female_total);
            System.out.println("\n");
            turn = false;
        }
        notifyAll();
    }

    public synchronized void enterFemale() throws InterruptedException {
        while (male_inside > 0 || (turn == true && female_inside == 0) || count == BUFFER_SIZE) {
            wait();
        }
        female_inside++;
        female_total--;
        count++;
        if (female_inside == 1 && male_total > 0) {
            turn = true;
        }
        buffer[in] = 2;
        in = (in + 1) % BUFFER_SIZE;
        System.out.println("Женщина вошла в ванную. Мужчин в ванной: " + male_inside + ", Женщин в ванной: " + count);
        notifyAll();
    }

    public synchronized void exitFemale() {
        female_inside--;
        count--;
        buffer[out] = 0;
        out = (out + 1) % BUFFER_SIZE;
        System.out.println("Женщина вышла из ванной. Мужчин в ванной: " + male_inside + ", Женщин в ванной: " + count);
        if (female_inside == 0 && male_total > 0) {
            System.out.println("Мужчин в очереди: " + male_total);
            System.out.println("\n");
            turn = true;
        }
        notifyAll();
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.print("Введите количество мужчин: ");
        int numMales = scanner.nextInt();

        System.out.print("Введите количество женщин: ");
        int numFemales = scanner.nextInt();

        BathroomScheduler scheduler = new BathroomScheduler(numMales, numFemales);

        for (int i = 0; i < numMales; i++) {
            new Thread(() -> {
                try {
                    scheduler.enterMale();
                    Thread.sleep(300); // сокращаем время ванной до 500 миллисекунд
                    scheduler.exitMale();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }).start();
        }

        for (int i = 0; i < numFemales; i++) {
            new Thread(() -> {
                try {
                    scheduler.enterFemale();
                    Thread.sleep(300); // simulate using the bathroom
                    scheduler.exitFemale();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }).start();
        }

        scanner.close();
    }
}