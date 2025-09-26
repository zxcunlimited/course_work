#include <GL/glut.h> //для создания приложения
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define SIZE 8 //длина

//перечисление
typedef enum {
    EMPTY = 0,
    WHITE,
    BLACK,
    WHITE_KING,
    BLACK_KING
} Piece;

Piece board[SIZE][SIZE]; // инициализация доски
int selectedX = -1, selectedY = -1; // для выбора фигуры (-1 это не выбрана фигура)
bool gameStarted = false; //флаг для старта игры
int gameMode = 0; // 1 - человек против бота, 2 - человек против человека
Piece turn = BLACK; //инициализация текущего хода (BLACK это произвольно, т.е. не имеет значаения)

//функция инициализации доски
void initBoard() {
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if ((x + y) % 2 == 1 && y < 3) board[y][x] = WHITE; // расставляем белые шашки
            else if ((x + y) % 2 == 1 && y > 4) board[y][x] = BLACK; // аналогично
            else board[y][x] = EMPTY; //пустая клетка
        }
    }
    selectedX = selectedY = -1; //сброс выбора шашки после завершения хода
    turn = WHITE; //первый ход начинают белые шашки
}

//проверка является ли фигура вражеской
//piece - проверяемая шашка, которую хочет сьесть игрок, current - выбранная шашка игрока
bool isEnemy(int piece, int current) {
    if (piece == EMPTY) return false;
    if ((current == BLACK || current == BLACK_KING) && (piece == WHITE || piece == WHITE_KING)) return true;
    if ((current == WHITE || current == WHITE_KING) && (piece == BLACK || piece == BLACK_KING)) return true;
    return false; // если фигура своя или пустая клетка или нет вражеской фигуры
}

//проверка фигуры является ли она дамкой
bool isKing(int piece) {
    return piece == WHITE_KING || piece == BLACK_KING;
}

//превращение в дамку при достижении края
void promoteIfNeeded(int x, int y) {
    if (board[y][x] == WHITE && y == SIZE - 1) board[y][x] = WHITE_KING;
    if (board[y][x] == BLACK && y == 0) board[y][x] = BLACK_KING;
}

// рисуем шашки (круги)
// cx, cy - центр круга по координатам X и Y, r - радиус круга
void drawCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN); // использование одной вершины в центре для треугольников, в общем режим веера треугольников
    glVertex2f(cx, cy); // задаем центральную точку круга
    for (int i = 0; i <= 100; i++) { //101 точка чтобы замыкать окружность без разрывов
        float angle = 2.0f * 3.1415926f * i / 100; // текущий угол в радианах, текущ шаг делится на общ кол-во и умнож на 2 Пи
        glVertex2f(cx + cosf(angle) * r, cy + sinf(angle) * r); //ставим координаты для треугольника
    }
    glEnd(); //завершение определения примитива
}

//рисовка игровой доски
void drawBoard() {
    float squareSize = 1.0f / (SIZE + 2); // размер клетки с учетом отступов (10% окна)
    float offset = squareSize; // отступ от края

    // рисование рамки
    glColor3f(0.3f, 0.2f, 0.1f); //коричневый цвет 
    glBegin(GL_QUADS); // режим рисования треугольника
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glVertex2f(0, 1);
    glEnd(); // завершаем рисование

    // отрисовка координат (A-H)
    glColor3f(1, 1, 1); // белый цвет
    for (int x = 0; x < SIZE; x++) {
        glRasterPos2f(offset + x * squareSize + squareSize * 0.4f, offset - squareSize * 0.7f); //устанавливаем позицию растрового текста
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'A' + x);// рисует символ

        glRasterPos2f(offset + x * squareSize + squareSize * 0.4f, offset + SIZE * squareSize + squareSize * 0.3f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'A' + x);
    }

    // отрисовка чиисел (1-8)
    for (int y = 0; y < SIZE; y++) {
        glRasterPos2f(offset - squareSize * 0.7f, offset + y * squareSize + squareSize * 0.4f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '1' + (SIZE - 1 - y));

        glRasterPos2f(offset + SIZE * squareSize + squareSize * 0.3f, offset + y * squareSize + squareSize * 0.4f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '1' + (SIZE - 1 - y));
    }

    // отрисовка самой игровой доски (ее клеток)
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if ((x + y) % 2 == 0) glColor3f(255 / 255.0f, 254 / 255.0f, 122 / 255.0f); //бежевый цвет
            else glColor3f(0.4f, 0.2f, 0.1f); //темно-коричневый

            glBegin(GL_QUADS);
            glVertex2f(offset + x * squareSize, offset + y * squareSize);
            glVertex2f(offset + (x + 1) * squareSize, offset + y * squareSize);
            glVertex2f(offset + (x + 1) * squareSize, offset + (y + 1) * squareSize);
            glVertex2f(offset + x * squareSize, offset + (y + 1) * squareSize);
            glEnd();

            int piece = board[y][x];
            if (piece != EMPTY) {
                if (piece == WHITE || piece == WHITE_KING) glColor3f(1, 1, 1);
                else glColor3f(0, 0, 0);
                drawCircle(offset + (x + 0.5f) * squareSize, offset + (y + 0.5f) * squareSize, squareSize * 0.4f);

                if (isKing(piece)) {
                    glColor3f(1, 0, 0);
                    drawCircle(offset + (x + 0.5f) * squareSize, offset + (y + 0.5f) * squareSize, squareSize * 0.2f);
                }
            }

            if (x == selectedX && y == selectedY) {
                glColor3f(0, 1, 0); //зеленый
                drawCircle(offset + (x + 0.5f) * squareSize, offset + (y + 0.5f) * squareSize, squareSize * 0.45f);
            }
        }
    }
}

//рисовка стартового экрана
void drawStartScreen() {
    // Рисуем кнопки
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(0.2f, 0.5f); glVertex2f(0.8f, 0.5f);
    glVertex2f(0.8f, 0.6f); glVertex2f(0.2f, 0.6f);
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(0.2f, 0.3f); glVertex2f(0.8f, 0.3f);
    glVertex2f(0.8f, 0.4f); glVertex2f(0.2f, 0.4f);
    glEnd();

    // Белый текст на кнопках
    glColor3f(1, 1, 1);
    glRasterPos2f(0.3f, 0.54f);
    const char* text1 = " PLAYER VS BOT";
    while (*text1) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text1++);

    glRasterPos2f(0.3f, 0.34f);
    const char* text2 = " PLAYER VS PLAYER";
    while (*text2) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text2++);

    // Большая надпись "CHECKERS"
    glColor3f(1, 1, 1); // Цвет надписи
    glRasterPos2f(0.32f, 0.75f);
    const char* title = "          CHECKERS";
    while (*title) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *title++);
}

//функция которая рисует каждый кадр когда вызывается каждый раз, вызывается GLUT
void display() {
    glClear(GL_COLOR_BUFFER_BIT); // очистка экрана
    glLoadIdentity(); // сброс всех трансформаций

    if (!gameStarted) drawStartScreen();//стартовый экран
    else drawBoard();//поле

    glutSwapBuffers(); // отображает готовый кадр на экране
}

//функция для проверки может ли фигура сделать взятие вражеской фигуры
bool canCaptureFrom(int x, int y) {
    int piece = board[y][x]; //выбранная фигура
    if (piece == EMPTY) return false;

    // Для обычных шашек
    if (!isKing(piece)) {
        int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };//массив направления движения дамки
        for (int d = 0; d < 4; d++) { //перебор всех 4 направлений
            int dx = dirs[d][0], dy = dirs[d][1]; // выбираем x и y на каждой итерации

            // вычисляем координаты соседних и дальних клеток
            int mx = x + dx;
            int my = y + dy;
            int tx = x + 2 * dx;
            int ty = y + 2 * dy;

            //проверка является ли шашка вражеской и есть ли за ней место
            if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE &&
                isEnemy(board[my][mx], piece) && board[ty][tx] == EMPTY) {
                return true;
            }
        }
    }
    else { // Для дамок
        int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} }; //массив направления движения дамки
        for (int d = 0; d < 4; d++) { //перебор всех 4 направлений
            int dx = dirs[d][0], dy = dirs[d][1]; // выбираем x и y на каждой итерации
            int nx = x + dx, ny = y + dy; // координаты соседних клеток
            bool foundEnemy = false;//флаг, который показывает нашли ли мы врага

            // Ищем вражескую шашку по диагонали
            while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                if (board[ny][nx] != EMPTY) { //если клетка не пустая
                    if (isEnemy(board[ny][nx], piece) && !foundEnemy) { //убеждаемся что это вражеская шашка и что мы еще не встречали в этом направлении
                        foundEnemy = true;
                        // Проверяем клетку за вражеской шашкой
                        int tx = nx + dx, ty = ny + dy;
                        if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY) {
                            // Проверяем, что между дамкой и вражеской шашкой нет других фигур
                            bool clearPath = true; //флаг на наличие фигуры между дамкой и вражеской шашкой
                            int cx = x + dx, cy = y + dy; //дальние клетки
                            while (cx != nx || cy != ny) { //до тех пор пока не равны соседним клеткам
                                if (board[cy][cx] != EMPTY) {
                                    clearPath = false;
                                    break;
                                }
                                cx += dx; //идем к следующей клетке
                                cy += dy; //аналогично
                            }
                            if (clearPath) return true; //если нет фигур
                        }
                        break;
                    }
                    else {
                        break; // Нашли свою шашку или вторую вражескую, т.е. не можем перепрыгивать через свои фигуры
                    }
                }
                nx += dx; //продолжаем движение по диагонали
                ny += dy;
            }
        }
    }
    return false; //если взятие невозможно
}

//может ли дамка сделать ход из одной клетки в другую
bool canKingMove(int fromX, int fromY, int toX, int toY, bool* isCapture) { //в параметры координаты исходной позиции дамки потом координы целевой позиции потом указатель на переменную был ли захват фигуры
    int dx = toX - fromX; //расстояние между позициями  
    int dy = toY - fromY;

    // Дамка должна двигаться по диагонали
    if (abs(dx) != abs(dy)) return false; //если ход недопустим, т.к. координаты не совпадают по диагонали

    //определяем направление шага и расстояние
    int stepX = dx > 0 ? 1 : -1;
    int stepY = dy > 0 ? 1 : -1;
    int distance = abs(dx); //количество шагов
    int enemyCount = 0; //кол-во врагов в пути
    int enemyX = -1, enemyY = -1; //координаты наденного врага

    // Проверяем путь дамки
    for (int i = 1; i < distance; i++) {
        int x = fromX + i * stepX; //вычисляем текущую клетку на пути   
        int y = fromY + i * stepY;

        if (board[y][x] != EMPTY) {
            if (isEnemy(board[y][x], board[fromY][fromX])) {
                enemyCount++; //увеличиваем счетчик врагов
                enemyX = x; //присваиваем координаты врага
                enemyY = y;
            }
            else {
                return false; // Своя шашка на пути 
            }
        }
    }

    if (enemyCount == 0) {
        *isCapture = false; //указваем что нет захвата
        return true; // ход воможен
    }
    else if (enemyCount == 1) {
        // Проверяем, что за вражеской шашкой пусто
        int behindX = enemyX + stepX;
        int behindY = enemyY + stepY;

        if (behindX == toX && behindY == toY) { //если после врага находится целевая клетка, то это допустимый захват
            *isCapture = true; //указываем что есть захват
            return true; // ход воможен
        }

        // Дамка может остановиться на любой клетке за вражеской шашкой
        while (behindX >= 0 && behindX < SIZE && behindY >= 0 && behindY < SIZE) { //цикл в пределах поля
            if (board[behindY][behindX] != EMPTY) return false; //если на пути дамки встречается другая фигура, то ход невозможен
            if (behindX == toX && behindY == toY) { //если текущая клетка совпадает с целевой
                *isCapture = true; //указываем что есть захват
                return true; // ход возможен
            }
            behindX += stepX; //двигаемся дальше по диагонали
            behindY += stepY;
        }
    }

    return false; //ход невозможен
}

//функция для выполнения хода бота
void makeBotMove() {
    typedef struct {
        int fromX, fromY, toX, toY; //координаты откуда и куда двигается фигура
        bool isCapture; //является ли ход взятием
    } Move;

    Move moves[100]; // для хранения всех возможных ходов
    int moveCount = 0; //счетчик найденных ходов

    // Шаг 1: собрать ВСЕ возможные взятия
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            int piece = board[y][x]; //выбранная фигура
            if (piece == BLACK || piece == BLACK_KING) {
                if (isKing(piece)) {
                    // Для дамки
                    int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} }; //все 4 возможных направления
                    for (int d = 0; d < 4; d++) {
                        int dx = dirs[d][0], dy = dirs[d][1]; //берем направления для x и y
                        int nx = x + dx, ny = y + dy; //координаты соседних клеток
                        bool foundEnemy = false; //флаг, который показывает нашли ли мы врага

                        while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                            if (board[ny][nx] != EMPTY) {
                                if (isEnemy(board[ny][nx], piece) && !foundEnemy) { //убеждаемся что это вражеская шашка и что мы еще не встречали в этом направлении
                                    foundEnemy = true; //нашли врага
                                    // Проверяем клетку за вражеской шашкой
                                    int tx = nx + dx, ty = ny + dy; //есть ли место за вражеской шашкой
                                    if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY) {
                                        // Проверяем, что между дамкой и вражеской шашкой нет других фигур
                                        bool clearPath = true; //флаг который указывает на наличие фигур между дамкой и вражеской фигурой
                                        int cx = x + dx, cy = y + dy; //берем еще дальние координаты
                                        while (cx != nx || cy != ny) { // до тех пор, пока не равны текущим координатам
                                            if (board[cy][cx] != EMPTY) {
                                                clearPath = false; //есть фигура
                                                break;
                                            }
                                            cx += dx; //двигаемся дальше по диагонали
                                            cy += dy;
                                        }
                                        if (clearPath) { //если нет фигур
                                            // Добавляем все возможные клетки за вражеской шашкой
                                            while (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY) {
                                                moves[moveCount++] = (Move){ x, y, tx, ty, true };
                                                tx += dx; //двигаем координаты за врагом
                                                ty += dy;
                                            }
                                        }
                                    }
                                    break;
                                }
                                else {
                                    break; // нашли свою шашку или вторую вражескую
                                }
                            }
                            nx += dx; //двигаем координаты соседних клеток
                            ny += dy;
                        }
                    }
                }
                else {
                    // Для обычной шашки
                    int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} }; //все 4 возможных направления
                    for (int d = 0; d < 4; d++) {
                        int dx = dirs[d][0], dy = dirs[d][1]; //берем направления для x и y
                        if (dy > 0) continue; // обычные только вперед
                        int mx = x + dx, my = y + dy; //координаты соседних клеток
                        int tx = x + 2 * dx, ty = y + 2 * dy; //координаты дальних клеток
                        if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE &&
                            isEnemy(board[my][mx], piece) && board[ty][tx] == EMPTY) {
                            moves[moveCount++] = (Move){ x, y, tx, ty, true };
                        }
                    }
                }
            }
        }
    }

    // Шаг 2: если нет взятий — собрать обычные ходы
    if (moveCount == 0) {
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {
                int piece = board[y][x];
                if (piece == BLACK || piece == BLACK_KING) {
                    if (isKing(piece)) {
                        // Для дамки
                        int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} }; //все 4 возможных направления
                        for (int d = 0; d < 4; d++) {
                            int dx = dirs[d][0], dy = dirs[d][1]; //координаты для x и y
                            int nx = x + dx, ny = y + dy; //координаты для соседних клеток

                            while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[ny][nx] == EMPTY) {
                                moves[moveCount++] = (Move){ x, y, nx, ny, false };
                                nx += dx; //движем координаты соседних клеток
                                ny += dy;
                            }
                        }
                    }
                    else {
                        // Для обычной шашки
                        int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} }; //все 4 возможных направления
                        for (int d = 0; d < 4; d++) {
                            int dx = dirs[d][0], dy = dirs[d][1];
                            if (dy > 0) continue; // обычные только вперед
                            int nx = x + dx, ny = y + dy; //координаты для соседних клеток
                            if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[ny][nx] == EMPTY) {
                                moves[moveCount++] = (Move){ x, y, nx, ny, false };
                            }
                        }
                    }
                }
            }
        }
    }

    if (moveCount == 0) return; // нет ходов

    // Шаг 3: выбрать случайный ход (предпочитая взятия)
    Move m;
    bool hasCapture = false; //флаг на наличие взятия
    for (int i = 0; i < moveCount; i++) { //цикл до количества  найденных ходов
        if (moves[i].isCapture) { //является ли ход взятием
            hasCapture = true; //нашли взятие
            break;
        }
    }

    if (hasCapture) {
        int captureMoves = 0; //счетчик взятий
        for (int i = 0; i < moveCount; i++) {
            if (moves[i].isCapture) captureMoves++;
        }
        m = moves[rand() % captureMoves]; //выбираем случайное взятие до количества взятий
    }
    else {
        m = moves[rand() % moveCount]; //выбираем любой доступный ход до количества ходов
    }

    int piece = board[m.fromY][m.fromX]; //сохранем фигуру которой будем ходить

    // выполнить начальный ход
    if (m.isCapture) {
        if (isKing(piece)) {
            // Для дамки находим вражескую шашку на пути
            int dx = m.toX > m.fromX ? 1 : -1;
            int dy = m.toY > m.fromY ? 1 : -1;
            int x = m.fromX + dx; //двигаем дамку по координам
            int y = m.fromY + dy;

            while (x != m.toX || y != m.toY) { //цикл до целевых координатов
                if (isEnemy(board[y][x], piece)) {
                    board[y][x] = EMPTY;
                    break;
                }
                x += dx; //двигаем дамку
                y += dy;
            }
        }
        else {
            // Для обычной шашки
            int mx = (m.fromX + m.toX) / 2; //берем середину между началом и концом, как раз там где находится вржеская шашка
            int my = (m.fromY + m.toY) / 2;
            board[my][mx] = EMPTY;
        }
    }

    board[m.toY][m.toX] = piece; //обновляем координаты выбранной фигуры
    board[m.fromY][m.fromX] = EMPTY; //место где была шашка, должно быть пустым
    promoteIfNeeded(m.toX, m.toY); //если шашка достигла края, то она превращается в дамку

    // Шаг 4: если это было взятие — продолжить, если возможно
    if (m.isCapture && canCaptureFrom(m.toX, m.toY)) { //если ход является взятием и проверка может фигура сделать взятие вражеской
        selectedX = m.toX; //сохраняем значения целевых координат
        selectedY = m.toY;
        makeBotMove(); // Рекурсивно продолжаем взятие
    }
    else {
        turn = WHITE; //иначе ходит человек
        selectedX = selectedY = -1; //сбрасываем выбор фи
    }

    glutPostRedisplay(); //перерисовка экрана
}

//функция которая отвечает за попытку выполнить ход игрока
void tryMove(int toX, int toY) { //на вход координаты целевой клетки
    int dx = toX - selectedX; //разница между целевой и текущей клеткой
    int dy = toY - selectedY;
    int piece = board[selectedY][selectedX]; //сохраняем фигуру, которой хотим двигаться

    if (board[toY][toX] != EMPTY) return;

    if (!isKing(piece)) {
        // Обычная шашка
        bool isBlack = (piece == BLACK || piece == BLACK_KING); //тру если черная
        bool validStep = abs(dx) == 1 && ((isBlack && dy == -1) || (!isBlack && dy == 1)); //проверка наличия простого хода по диагонали на одну клетку
        bool validJump = abs(dx) == 2 && abs(dy) == 2; //проверка на прыжок через одну клетку

        if (validStep) {
            board[toY][toX] = piece; //ставим фигуру на целевые координаты 
            board[selectedY][selectedX] = EMPTY; //чистим старые координаты от фигуры
            promoteIfNeeded(toX, toY); //при достижении края превращаем в дамку
            selectedX = selectedY = -1; //сбрасываем выделенную шашку
            turn = (turn == BLACK) ? WHITE : BLACK;
            glutPostRedisplay(); //перерисовываем экран

            if (gameMode == 1 && turn == BLACK) makeBotMove(); //если это режим бота и ходят черные, то используем функцию бота 
            return;
        }

        if (validJump) {
            int mx = selectedX + dx / 2; //выбираем середину, т.е. вражескую фигуру
            int my = selectedY + dy / 2;
            if (!isEnemy(board[my][mx], piece)) return; //если не явл вражеской, то ниче не делаем

            board[my][mx] = EMPTY;
            board[toY][toX] = piece;
            board[selectedY][selectedX] = EMPTY;
            promoteIfNeeded(toX, toY);

            selectedX = toX; //заопминаем новую позицию шашки, как выбранную
            selectedY = toY;

            if (canCaptureFrom(toX, toY)) { //проверка может ли сделать взятие
                glutPostRedisplay(); //обновляем экран
            }
            else {
                selectedX = selectedY = -1; //сброс выбора шашки
                turn = (turn == BLACK) ? WHITE : BLACK;
                if (gameMode == 1 && turn == BLACK) makeBotMove();
                glutPostRedisplay();
            }
        }
    }
    else {
        // Дамка
        bool isCapture = false; //флаг является ли флаг взятием
        if (canKingMove(selectedX, selectedY, toX, toY, &isCapture)) { //проверка возможен ли ход для дамки
            if (isCapture) {
                // Находим и удаляем вражескую шашку
                int stepX = dx > 0 ? 1 : -1; //определяем направление движения шашки
                int stepY = dy > 0 ? 1 : -1;
                int x = selectedX + stepX; //движение дамки
                int y = selectedY + stepY;

                while (x != toX || y != toY) {
                    if (isEnemy(board[y][x], piece)) {
                        board[y][x] = EMPTY;
                        break;
                    }
                    x += stepX; //двигаем дальше дамку
                    y += stepY;
                }
            }

            board[toY][toX] = piece;
            board[selectedY][selectedX] = EMPTY;
            promoteIfNeeded(toX, toY);

            if (isCapture && canCaptureFrom(toX, toY)) { //является ли ход взятием и проверка может ли фигура сделать взятие
                selectedX = toX;
                selectedY = toY;
                glutPostRedisplay();
            }
            else {
                selectedX = selectedY = -1;
                turn = (turn == BLACK) ? WHITE : BLACK;
                if (gameMode == 1 && turn == BLACK) makeBotMove();
                glutPostRedisplay();
            }
        }
    }
}

//функция которая обрабатывает нажатия мышки
void mouse(int button, int state, int x, int y) { //в параметрах какой кнопкой нажато, состояние кнопки и координаты клика в пикселях
    if (state != GLUT_DOWN) return; //проверяем что это нажатие кнопки: даун когда нажато, ап когда нет
    //вычисляем координаты внутри окна от 0 до 1
    float fx = (float)x / glutGet(GLUT_WINDOW_WIDTH); //перевод координатов экрана в нормализованные 
    float fy = 1.0f - (float)y / glutGet(GLUT_WINDOW_HEIGHT); //инвертируем Y, т.к. в опенгл (0,0) это левый нижний угол

    //проверяем попал ли клик в одну из кнопок старта игры
    if (!gameStarted) {
        if (fx > 0.2f && fx < 0.8f && fy > 0.5f && fy < 0.6f) {
            gameMode = 1;
            gameStarted = true; //игра начинается 
            initBoard(); //запуск поля
            glutPostRedisplay(); //обновляем экран
        }
        else if (fx > 0.2f && fx < 0.8f && fy > 0.3f && fy < 0.4f) {
            gameMode = 2;
            gameStarted = true;
            initBoard();
            glutPostRedisplay();
        }
        return;
    }

    float squareSize = 1.0f / (SIZE + 2); // определяет размер клетки внутри поля (10% от окна)
    float offset = squareSize; // отступ от края окна

    // вычисляем границы игрового поля
    float boardLeft = offset; // 0.1f
    float boardRight = offset + SIZE * squareSize;// 0.9f
    float boardBottom = offset;// 0.1f
    float boardTop = offset + SIZE * squareSize;// 0.9f

    if (fx < boardLeft || fx > boardRight || fy < boardBottom || fy > boardTop) return; // если клик вне границ, то он игнорируется

    int bx = (fx - offset) / squareSize; // вычисляем в какую клетку попал игрок 
    int by = (fy - offset) / squareSize;

    if ((gameMode == 1 && turn != WHITE) || bx < 0 || bx >= SIZE || by < 0 || by >= SIZE) return;

    int clicked = board[by][bx]; //получаем тип фигуры на клетке
    //проверяем кликнул ли игрок на свою фигуру
    if ((turn == BLACK && (clicked == BLACK || clicked == BLACK_KING)) ||
        (turn == WHITE && (clicked == WHITE || clicked == WHITE_KING))) {
        selectedX = bx; //запоминаем клетку как выбранную
        selectedY = by;
    }
    else if (selectedX != -1) { //если уже есть выбранная фигура, то пробуем сделать ход
        tryMove(bx, by);
    }
    glutPostRedisplay(); //обновляем окно
}

//функция которая вызывается при изменении размера окна
void reshape(int w, int h) {
    glViewport(0, 0, w, h); //устанавливает область вывода окна, т.е. при именении размера окна меняется область
    glMatrixMode(GL_PROJECTION); //переключаемся на работу с матрицей проекции, определяет как будут отображаться координаты на экране
    glLoadIdentity(); //сбрасываем трансформации
    gluOrtho2D(0, 1, 0, 1); //устанавливаем двумерную ортографическую проекцию  
    glMatrixMode(GL_MODELVIEW); //устанавливаем матрицу снова
    glLoadIdentity(); //сбрасываем трансформации
}

int main(int argc, char** argv) { //параметры нужны как раз для инициализации 
    srand(time(NULL));
    glutInit(&argc, argv); //обязательная инициализация glut    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); //установка режима отображения, один буфер показывается на экране, другой используется для следующего кадра, чтобы избежать мерцания, след макрос для цветов rgb
    glutInitWindowSize(800, 800); //устнававливается рамер окна
    glutCreateWindow("Checkers game"); //название окна
    glutDisplayFunc(display); //регистрируем функцию рисовки, меню, доску фигуры и т.д.
    glutMouseFunc(mouse);//регистрируем функцию для кликов
    glutReshapeFunc(reshape); //регистрируем функцию изменения размера окна
    glClearColor(0.2f, 0.2f, 0.2f, 1); //темно-серый фон для меню
    glutMainLoop(); //для бесконечной обработки событий
    return 0;
}