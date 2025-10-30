#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <locale.h>
#include <Windows.h>
#include <time.h>

#define SIZE 8 // Это макрос, который задаёт размер игрового поля.

typedef enum
{ // перечисление которое определяет какие фигуры могут быть на доске
    EMPTY = 0,
    WHITE,
    BLACK,
    WHITE_KING,
    BLACK_KING
} Piece;

Piece board[SIZE][SIZE];            // создаем поле
int selectedX = -1, selectedY = -1; // координаты выбранной шашки (-1 это ничего не выбрано)
bool gameStarted = false;           // флаг указывающий начата ли игра
int gameMode = 0;                   // 1 - человек против бота, 2 - человек против человека
Piece turn;                         // определяет чей ход сейчас, черных или белых

// инициализируем доску шашками
void initBoard()
{
    for (int y = 0; y < SIZE; y++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            if ((x + y) % 2 == 1 && y < 3)
                board[y][x] = WHITE;
            else if ((x + y) % 2 == 1 && y > 4)
                board[y][x] = BLACK;
            else
                board[y][x] = EMPTY;
        }
    }
    selectedX = selectedY = -1;
    turn = WHITE;
}

// проверка является ли шашка вражеской
bool isEnemy(int piece, int current)
{
    if (piece == EMPTY)
        return false;
    if ((current == BLACK || current == BLACK_KING) && (piece == WHITE || piece == WHITE_KING))
        return true;
    if ((current == WHITE || current == WHITE_KING) && (piece == BLACK || piece == BLACK_KING))
        return true;
    return false;
}

// проверяем является ли шашка дамкой
bool isKing(int piece)
{
    return piece == WHITE_KING || piece == BLACK_KING;
}

// повышение шашки до дамки
void promoteIfNeeded(int x, int y)
{
    if (board[y][x] == WHITE && y == SIZE - 1)
        board[y][x] = WHITE_KING;
    if (board[y][x] == BLACK && y == 0)
        board[y][x] = BLACK_KING;
}

// отрисовка шашек
void drawCircle(float cx, float cy, float r)
{                             /*координаты центра и радиус круга*/
    glBegin(GL_TRIANGLE_FAN); // для веера треугольников
    glVertex2f(cx, cy);       // определеяет первую вершину, т.е. центр круга
    for (int i = 0; i <= 100; i++)
    {                                                           /*рисуем 100 треугольников вокруг центра*/
        float angle = 2.0f * 3.1415926f * i / 100;              // вычисляем текущий угол в радианах для каждой точки на окружности
        glVertex2f(cx + cosf(angle) * r, cy + sinf(angle) * r); // добавляем вершину на окружности (точку на краю круга)
    }
    glEnd(); // завершаем
}

// отрисовка игрового поля
void drawBoard()
{
    float squareSize = 1.0f / (SIZE + 2); // размер одной клетки (0.1 от всего поля)
    float offset = squareSize;            // смещение от левого/нижнего края до начала самой доски

    // Draw border
    glColor3f(0.3f, 0.2f, 0.1f); // устанавливаем текущий цвет для рисования (темно-коричневый)
    glBegin(GL_QUADS);           // устанавливаем что это квадрат
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glVertex2f(0, 1); // закрашиваем весь экран
    glEnd();          // завершаем

    // Draw coordinate letters (A-H)
    glColor3f(1, 1, 1); // белый цвет для текста координат
    for (int x = 0; x < SIZE; x++)
    {
        glRasterPos2f(offset + x * squareSize + squareSize * 0.4f, offset - squareSize * 0.7f); // устанвливаем позицию для отрисовки символов снизу
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'A' + x);                                 // устанавливаем шрифт и рисуем символы по алфавиту

        glRasterPos2f(offset + x * squareSize + squareSize * 0.4f, offset + SIZE * squareSize + squareSize * 0.3f); // сверху
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'A' + x);
    }

    // Draw coordinate numbers (1-8)
    for (int y = 0; y < SIZE; y++)
    {
        glRasterPos2f(offset - squareSize * 0.7f, offset + y * squareSize + squareSize * 0.4f); // устнавливаем координаты для чисел (слева)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '1' + (SIZE - 1 - y));                    // нумерация от 8 до 1, тк начинается снизу вверх

        glRasterPos2f(offset + SIZE * squareSize + squareSize * 0.3f, offset + y * squareSize + squareSize * 0.4f); // справа
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '1' + (SIZE - 1 - y));
    }

    // Draw checkerboard
    for (int y = 0; y < SIZE; y++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            if ((x + y) % 2 == 0)
                glColor3f(255 / 255.0f, 254 / 255.0f, 122 / 255.0f); // светлые клетки
            else
                glColor3f(0.4f, 0.2f, 0.1f); // коричневые клетки

            glBegin(GL_QUADS); // рисуем квадратные клетки, устанавливая для них границы
            glVertex2f(offset + x * squareSize, offset + y * squareSize);
            glVertex2f(offset + (x + 1) * squareSize, offset + y * squareSize);
            glVertex2f(offset + (x + 1) * squareSize, offset + (y + 1) * squareSize);
            glVertex2f(offset + x * squareSize, offset + (y + 1) * squareSize);
            glEnd(); // завершаем

            int piece = board[y][x]; // читаем содержимеое из клетки, если она не пустая, то рисуем шашку
            if (piece != EMPTY)
            {
                if (piece == WHITE || piece == WHITE_KING)
                    glColor3f(1, 1, 1);
                else
                    glColor3f(0, 0, 0);
                drawCircle(offset + (x + 0.5f) * squareSize, offset + (y + 0.5f) * squareSize, squareSize * 0.4f); // рисуем шашку

                if (isKing(piece))
                {
                    glColor3f(1, 0, 0); // красный цвет
                    drawCircle(offset + (x + 0.5f) * squareSize, offset + (y + 0.5f) * squareSize, squareSize * 0.2f);
                }
            }

            if (x == selectedX && y == selectedY)
            { // если это выбранная шашка, то закрашиваем ее в зеленый цвет
                glColor3f(0, 1, 0);
                drawCircle(offset + (x + 0.5f) * squareSize, offset + (y + 0.5f) * squareSize, squareSize * 0.45f);
            }
        }
    }
}

// меню, стартовый экран
void drawStartScreen()
{
    // Рисуем кнопки
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS); // рисуем четырехугольник, черную кнопку
    glVertex2f(0.2f, 0.5f);
    glVertex2f(0.8f, 0.5f);
    glVertex2f(0.8f, 0.6f);
    glVertex2f(0.2f, 0.6f);
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(0.2f, 0.3f);
    glVertex2f(0.8f, 0.3f);
    glVertex2f(0.8f, 0.4f);
    glVertex2f(0.2f, 0.4f);
    glEnd();

    // Белый текст на кнопках
    glColor3f(1, 1, 1);
    glRasterPos2f(0.3f, 0.54f);
    const char *text1 = " PLAYER VS BOT";
    while (*text1)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text1++);

    glRasterPos2f(0.3f, 0.34f);
    const char *text2 = " PLAYER VS PLAYER";
    while (*text2)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text2++);

    // Большая надпись "CHECKERS"
    glColor3f(1, 1, 1); // Цвет надписи
    glRasterPos2f(0.32f, 0.75f);
    const char *title = "          CHECKERS";
    while (*title)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *title++);
}

// для перерисовки окна
void display()
{
    glClear(GL_COLOR_BUFFER_BIT); // очищается экран
    glLoadIdentity();             // сбрасывает матрицы преобразования

    if (!gameStarted)
        drawStartScreen(); // если игра не начата, то рисуем меню
    else
        drawBoard(); // иначе доску

    glutSwapBuffers(); // используется двойная буферизация для избежания мерцания
}

// проверка может ли фигура выполнить взятие
bool canCaptureFrom(int x, int y)
{
    int piece = board[y][x]; // считываем содержимое клетки
    if (piece == EMPTY)
        return false; // если пустая

    // Для обычных шашек
    if (!isKing(piece))
    {
        int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
        for (int d = 0; d < 4; d++)
        {
            int dx = dirs[d][0], dy = dirs[d][1]; // перебираем каждое из четыех направлений

            int mx = x + dx; // промежуточные клетки
            int my = y + dy;
            int tx = x + 2 * dx; // целевые клетки
            int ty = y + 2 * dy;

            // проверяем условия для валидного взятия
            if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE &&
                mx >= 0 && mx < SIZE && my >= 0 && my < SIZE &&
                isEnemy(board[my][mx], piece) && board[ty][tx] == EMPTY)
            {
                return true;
            }
        }
    }
    else
    { // Для дамок
        int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
        for (int d = 0; d < 4; d++)
        {
            int dx = dirs[d][0], dy = dirs[d][1];
            int nx = x + dx, ny = y + dy; // координаты первой клетки по диагонали от дамки
            bool foundEnemy = false;

            // Ищем вражескую шашку по диагонали
            while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
            {
                if (board[ny][nx] != EMPTY)
                {
                    if (isEnemy(board[ny][nx], piece) && !foundEnemy)
                    { // если нашли вражескую шашку
                        foundEnemy = true;
                        // Проверяем клетку за вражеской шашкой
                        int tx = nx + dx, ty = ny + dy; // соседняя клетка за шашкой
                        if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY)
                        {
                            // Проверяем, что между дамкой и вражеской шашкой нет других фигур
                            bool clearPath = true;
                            int cx = x + dx, cy = y + dy;
                            while (cx != nx || cy != ny)
                            {
                                if (board[cy][cx] != EMPTY)
                                {
                                    clearPath = false;
                                    break;
                                }
                                cx += dx;
                                cy += dy;
                            }
                            if (clearPath)
                                return true;
                        }
                        break;
                    }
                    else
                    {
                        break; // Нашли свою шашку или вторую вражескую
                    }
                }
                nx += dx;
                ny += dy;
            }
        }
    }
    return false;
}

// возращает true если дамка может выполнить ход
bool canKingMove(int fromX, int fromY, int toX, int toY, bool *isCapture)
{                         /*isCapture это флаг на взятие*/
    int dx = toX - fromX; // вычисляем разницу в координатах
    int dy = toY - fromY;

    // Дамка должна двигаться по диагонали
    if (abs(dx) != abs(dy))
        return false;

    int stepX = dx > 0 ? 1 : -1; // направление шага про осям
    int stepY = dy > 0 ? 1 : -1;
    int distance = abs(dx);       // дистанция
    int enemyCount = 0;           // сколько вражеских фигур встретилось на пути
    int enemyX = -1, enemyY = -1; // координаты последней вражеской фигуры

    // Проверяем путь дамки
    for (int i = 1; i < distance; i++)
    {
        int x = fromX + i * stepX; // координаты каждой промежуточной клетки
        int y = fromY + i * stepY;

        if (board[y][x] != EMPTY)
        {
            if (isEnemy(board[y][x], board[fromY][fromX]))
            { // если вражеская фигура
                enemyCount++;
                enemyX = x;
                enemyY = y;
            }
            else
            {
                return false; // Своя шашка на пути
            }
        }
    }

    if (enemyCount == 0)
    {
        *isCapture = false;
        return true; // Просто ход без взятия
    }
    else if (enemyCount == 1)
    {
        // Проверяем, что за вражеской шашкой пусто
        int behindX = enemyX + stepX;
        int behindY = enemyY + stepY;

        if (behindX == toX && behindY == toY)
        {
            *isCapture = true;
            return true;
        }

        // Дамка может остановиться на любой клетке за вражеской шашкой
        while (behindX >= 0 && behindX < SIZE && behindY >= 0 && behindY < SIZE)
        {
            if (board[behindY][behindX] != EMPTY)
                return false;
            if (behindX == toX && behindY == toY)
            {
                *isCapture = true;
                return true;
            }
            behindX += stepX;
            behindY += stepY;
        }
    }

    return false;
}

/* ---------- НОВЫЕ ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ОЦЕНКИ ХОДОВ БОТА ---------- */

/* Сохраняет текущую доску в temp и восстанавливает из temp */
void backupBoard(Piece temp[SIZE][SIZE])
{
    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
            temp[y][x] = board[y][x];
}
void restoreBoard(Piece temp[SIZE][SIZE])
{
    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
            board[y][x] = temp[y][x];
}

/* Генерирует все возможные взятия для цвета 'color' и возвращает через массив moves
   Возвращаем: число найденных взятий */
// структура для ходов
typedef struct
{
    int fromX, fromY, toX, toY;
    bool isKing;
} FastMove;

/* Получить возможные взятия для конкретной позиции (используется для проверки уязвимости) */
int generateCaptureMovesForColor(int color, FastMove outMoves[], int maxMoves)
{
    int count = 0; // счетчик найденных взятий
    for (int y = 0; y < SIZE; y++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            int piece = board[y][x];
            if (piece == EMPTY)
                continue;
            bool pieceIsColor = ((color == BLACK) && (piece == BLACK || piece == BLACK_KING)) ||
                                ((color == WHITE) && (piece == WHITE || piece == WHITE_KING)); // проверяем, принадлежит ли фигура в текущей клетке указанному color
            if (!pieceIsColor)
                continue;

            if (isKing(piece))
            {
                int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                for (int d = 0; d < 4; d++)
                {
                    int dx = dirs[d][0], dy = dirs[d][1]; // подбираем направления
                    int nx = x + dx, ny = y + dy;         // координаты текущей клетки рядом дамкой
                    bool foundEnemy = false;              // флаг на наличие врага
                    while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
                    { /*цикл пока не выйдем за пределы поля*/
                        if (board[ny][nx] != EMPTY)
                        {
                            if (isEnemy(board[ny][nx], piece) && !foundEnemy)
                            {
                                foundEnemy = true;
                                int tx = nx + dx, ty = ny + dy; // ставим координаты целевой клетки, за вражеской фигурой
                                if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY)
                                {                          /*если в пределах поля и клетка пустая*/
                                    bool clearPath = true; // флаг на наличие фигур между найденной вражеской фигурой и дамкой
                                    int cx = x + dx, cy = y + dy;
                                    while (cx != nx || cy != ny)
                                    {
                                        if (board[cy][cx] != EMPTY)
                                        {
                                            clearPath = false;
                                            break;
                                        }
                                        cx += dx;
                                        cy += dy;
                                    }
                                    if (clearPath)
                                    { /*если путь до вражеской фигуры чист, то дамка может приземлиться на любую пустую клетку, лежащую дальше за врагом (не только на первую).*/
                                        while (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY)
                                        {
                                            /*outMoves — массив FastMove, в который записывают найденные ходы с взятием.
                                            maxMoves — максимальное количество элементов, которые безопасно записать в outMoves.*/
                                            if (count < maxMoves)
                                                outMoves[count] = (FastMove){x, y, tx, ty, true};
                                            count++;
                                            tx += dx;
                                            ty += dy;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        nx += dx;
                        ny += dy;
                    }
                }
            }
            else
            {
                int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                for (int d = 0; d < 4; d++)
                {
                    int dx = dirs[d][0], dy = dirs[d][1];
                    // обычные шашки: direction restriction handled via isEnemy checks / board coords
                    int mx = x + dx, my = y + dy;
                    int tx = x + 2 * dx, ty = y + 2 * dy;
                    if (mx >= 0 && mx < SIZE && my >= 0 && my < SIZE &&
                        tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE &&
                        isEnemy(board[my][mx], piece) && board[ty][tx] == EMPTY)
                    {
                        if (count < maxMoves)
                            outMoves[count] = (FastMove){x, y, tx, ty, false}; // если взятие допустимо — записываем ход в outMoves
                        count++;
                    }
                }
            }
        }
    }
    return count; // возвращаем общее число найденных вариантов взятия count
}

/* Проверяет — есть ли у противника взятие, которое снимет фигуру, стоящую в (targetX,targetY)
   Возвращает true, если такая угроза есть. Эта функция симулирует на текущей доске (предполагается, что ход уже сделан). */
bool opponentCanCaptureSquare(int targetX, int targetY, int opponentColor)
{
    FastMove moves[200];
    int cnt = generateCaptureMovesForColor(opponentColor, moves, 200);
    // Для каждого взятия нужно понять, какая координата съеденной фигуры.
    for (int i = 0; i < cnt && i < 200; i++)
    {
        int fx = moves[i].fromX, fy = moves[i].fromY, tx = moves[i].toX, ty = moves[i].toY;
        if (!moves[i].isKing)
        {
            // обычный прыжок — съеденная фигура в середине
            int mx = (fx + tx) / 2;
            int my = (fy + ty) / 2;
            if (mx == targetX && my == targetY)
                return true;
        }
        else
        {
            // дамка — нужно найти вражескую фигуру на диагонали между fx,fy и tx,ty
            int dx = (tx > fx) ? 1 : -1; // выбираем направление
            int dy = (ty > fy) ? 1 : -1;
            int cx = fx + dx, cy = fy + dy; // сохраняем начальные координаты
            while (cx != tx || cy != ty)
            {
                if (cx == targetX && cy == targetY)
                    return true; // дамка может съесть эту клетку
                if (board[cy][cx] != EMPTY && !(cx == targetX && cy == targetY))
                {
                    // это какая-то фигура (но если это не target — это препятствие для данного варианта)
                    break;
                }
                cx += dx;
                cy += dy;
            }
        }
    }
    return false;
}

/* Рекурсивно считаем максимальное количество съедаемых фигур (цепочек) для бота, но с ограниченной глубиной.
   Для простоты: функция симулирует только цепочки, начинающиеся с one capture move (from->to),
   и возвращает максимум до depthLimit. */
int simulateCaptureChainCount(int fromX, int fromY, int toX, int toY, Piece piece, int depthLimit)
{
    Piece temp[SIZE][SIZE]; // сохраняем и копируем доску во временную переменную
    backupBoard(temp);

    // Выполнить одно взятие (извлечь съеденную)
    if (isKing(piece))
    {
        int dx = (toX > fromX) ? 1 : -1;
        int dy = (toY > fromY) ? 1 : -1;
        int x = fromX + dx, y = fromY + dy;
        while (x != toX || y != toY)
        {
            if (isEnemy(board[y][x], piece))
            {
                board[y][x] = EMPTY;
                break;
            }
            x += dx;
            y += dy;
        }
    }
    else
    {
        int mx = (fromX + toX) / 2;
        int my = (fromY + toY) / 2;
        board[my][mx] = EMPTY;
    }
    board[toY][toX] = piece;
    board[fromY][fromX] = EMPTY;
    promoteIfNeeded(toX, toY); // если достигла конца поля, то повышается

    int best = 1; // уже сделал 1 взятие
    if (depthLimit > 1 && canCaptureFrom(toX, toY))
    {
        // Найдем все возможные последующие взятия с позиции toX,toY
        // Перечисляем возможные цели — мы можем повторно использовать часть логики генерации взятий
        // Для простоты соберём ходы и рекурсивно считать максимум
        // Собираем потенциальные capture moves from toX,toY
        Piece curPiece = board[toY][toX];
        if (curPiece != EMPTY)
        {
            // Для дамки
            if (isKing(curPiece))
            {
                int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                for (int d = 0; d < 4; d++)
                {
                    int dx = dirs[d][0], dy = dirs[d][1];
                    int nx = toX + dx, ny = toY + dy;
                    bool foundEnemy = false;
                    while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
                    {
                        if (board[ny][nx] != EMPTY)
                        {
                            if (isEnemy(board[ny][nx], curPiece) && !foundEnemy)
                            {
                                foundEnemy = true;
                                int tx = nx + dx, ty = ny + dy;
                                if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY)
                                {
                                    int curtx = tx, curty = ty;
                                    while (curtx >= 0 && curtx < SIZE && curty >= 0 && curty < SIZE && board[curty][curtx] == EMPTY)
                                    {
                                        int chain = 1 + simulateCaptureChainCount(toX, toY, curtx, curty, curPiece, depthLimit - 1); // рекурсивно увеличиваем цепочку взятий
                                        if (chain > best)
                                            best = chain; // присваивается к переменной со взятиями
                                        curtx += dx;
                                        curty += dy;
                                    }
                                }
                            }
                            break;
                        }
                        nx += dx;
                        ny += dy;
                    }
                }
            }
            else
            {
                int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                for (int d = 0; d < 4; d++)
                {
                    int dx = dirs[d][0], dy = dirs[d][1];
                    int mx = toX + dx, my = toY + dy;
                    int tx = toX + 2 * dx, ty = toY + 2 * dy;
                    if (mx >= 0 && mx < SIZE && my >= 0 && my < SIZE &&
                        tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE &&
                        isEnemy(board[my][mx], curPiece) && board[ty][tx] == EMPTY)
                    {
                        int chain = 1 + simulateCaptureChainCount(toX, toY, tx, ty, curPiece, depthLimit - 1); // рекурсивно увеличиваем цепочку взятий
                        if (chain > best)
                            best = chain;
                    }
                }
            }
        }
    }

    restoreBoard(temp); // восстанавливаем доску, чтобы она не была изменена
    return best;        // возвращает максимальное количество взятых фигур в цепочке
}

/* Оценка хода: +за длину цепочки, +за продвижение к дамке, -за уязвимость */
int evaluateMoveScore(int fromX, int fromY, int toX, int toY, bool isCapture, Piece piece)
{
    int score = 0; // для оценки + и -

    // Базовые приоритеты
    if (isCapture)
    {
        // оцениваем длину цепочки (ограниченная глубина)
        int chain = simulateCaptureChainCount(fromX, fromY, toX, toY, piece, 4); // глубина 4
        score += chain * 200;                                                    // вес для взятий достаточно большой
    }

    // Поощряем продвижение к дамке (для черных: y уменьшение до 0)
    if (!isKing(piece))
    {
        if (piece == BLACK)
        {
            int distBefore = fromY;
            int distAfter = toY;
            if (distAfter < distBefore)
                score += (distBefore - distAfter) * 5;
            // бонус если дошел до последней линии
            if (toY == 0)
                score += 80;
        }
        else if (piece == WHITE)
        {
            int distBefore = (SIZE - 1 - fromY);
            int distAfter = (SIZE - 1 - toY);
            if (distAfter < distBefore)
                score += (distBefore - distAfter) * 5;
            if (toY == SIZE - 1)
                score += 80;
        }
    }
    else
    {
        // для дамки небольшой бонус за активность (чем дальше в центр — тем лучше)
        int centerDist = abs(toX - SIZE / 2) + abs(toY - SIZE / 2);
        score += (20 - centerDist);
    }

    // Штраф за уязвимость: если после выполнения хода противник может съесть нашу фигуру — штраф
    Piece temp[SIZE][SIZE];
    backupBoard(temp);
    // Выполнить ход на доске
    if (isCapture)
    {
        if (isKing(piece))
        {
            int dx = (toX > fromX) ? 1 : -1;
            int dy = (toY > fromY) ? 1 : -1;
            int x = fromX + dx, y = fromY + dy;
            while (x != toX || y != toY)
            {
                if (isEnemy(board[y][x], piece))
                {
                    board[y][x] = EMPTY;
                    break;
                }
                x += dx;
                y += dy;
            }
        }
        else
        {
            int mx = (fromX + toX) / 2, my = (fromY + toY) / 2;
            board[my][mx] = EMPTY;
        }
    }
    board[toY][toX] = piece;
    board[fromY][fromX] = EMPTY;
    promoteIfNeeded(toX, toY);

    // Проверим, может ли соперник снять фигуру на toX,toY
    int opponentColor = (piece == BLACK || piece == BLACK_KING) ? WHITE : BLACK;
    if (opponentCanCaptureSquare(toX, toY, opponentColor))
    {
        score -= 180; // большой штраф за риск стать съеденным
    }
    else
    {
        score += 10; // небольшой бонус за безопасность
    }

    // восстановим доску
    restoreBoard(temp);

    return score;
}

/* ---------- Конец вспомогательных функций ---------- */

// возвращает true если есть у фигуры хотя бы один допустимый ход
bool hasAnyMove(Piece color)
{
    for (int y = 0; y < SIZE; y++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            int piece = board[y][x];
            if (piece == EMPTY)
                continue;

            // Проверяем, принадлежит ли фигура нужному цвету
            bool isMyPiece = ((color == WHITE) && (piece == WHITE || piece == WHITE_KING)) ||
                             ((color == BLACK) && (piece == BLACK || piece == BLACK_KING));
            if (!isMyPiece)
                continue;

            // Проверка возможных взятий
            if (canCaptureFrom(x, y))
                return true;

            // Проверка обычных ходов
            if (isKing(piece))
            {
                // Дамка может двигаться по диагоналям до препятствия
                int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                for (int d = 0; d < 4; d++)
                {
                    int dx = dirs[d][0], dy = dirs[d][1];
                    int nx = x + dx, ny = y + dy;
                    while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
                    {
                        if (board[ny][nx] == EMPTY)
                            return true;
                        else
                            break;
                        nx += dx;
                        ny += dy;
                    }
                }
            }
            else
            {
                // Обычная шашка
                int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                for (int d = 0; d < 4; d++)
                {
                    int dx = dirs[d][0], dy = dirs[d][1];
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[ny][nx] == EMPTY)
                    {
                        // Для черных — движение вверх, для белых — вниз
                        if ((piece == BLACK && dy == -1) || (piece == WHITE && dy == 1))
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

void makeBotMove()
{
    /* новое: бенчмарк по времени */
    // путь до файла C:\Users\danya\source\repos\zxcunlimited\course_work\course_work-1\checkers_better\data.csv
    FILE* f = fopen("data.csv", "a"); // открываем файл в режиме дозаписи
    if (!f) {
        perror("Не удалось открыть файл");
        return;
    }

    printf("------Статистика скорости------\n");

    // структура для 64-битных целых чисел
    LARGE_INTEGER start_time, end_time, frequency;

    // получаем частоту счетчика производительности
    QueryPerformanceFrequency(&frequency);

    // начало отсчета времени
    QueryPerformanceCounter(&start_time);

    double elapsed_ms_1, elapsed_ms_2, elapsed_ms_3;
    if (!gameStarted)
        return; // если игра не запущена

    typedef struct
    {
        int fromX, fromY, toX, toY;
        bool isCapture;
        Piece piece;
    } Move;

    Move moves[400];   // массив для накопления всех найденных ходов
    int moveCount = 0; // счетчик найденных ходов

    // --- Проверка: есть ли вообще ходы ---
    bool whiteCanMoveBefore = hasAnyMove(WHITE);
    bool blackCanMoveBefore = hasAnyMove(BLACK);
    if (!whiteCanMoveBefore || !blackCanMoveBefore)
    {
        printf("\n=== ИГРА ОКОНЧЕНА ===\n");
        if (!whiteCanMoveBefore && !blackCanMoveBefore)
            printf("Ничья! Ни у кого нет ходов.\n");
        else if (!whiteCanMoveBefore)
            printf("Чёрные победили!\n");
        else if (!blackCanMoveBefore)
            printf("Белые победили!\n");
        gameStarted = false;
        glutPostRedisplay(); // обновляем кадр
        return;
    }

    // --- Шаг 1: собрать все возможные взятия ---
    for (int y = 0; y < SIZE; y++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            int piece = board[y][x];
            if (piece == BLACK || piece == BLACK_KING)
            {
                if (isKing(piece))
                {
                    // Дамка может бить в любом направлении
                    int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                    for (int d = 0; d < 4; d++)
                    {
                        int dx = dirs[d][0], dy = dirs[d][1];
                        int nx = x + dx, ny = y + dy;
                        bool foundEnemy = false;
                        while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
                        {
                            if (board[ny][nx] != EMPTY)
                            {
                                if (isEnemy(board[ny][nx], piece) && !foundEnemy)
                                {
                                    foundEnemy = true;
                                    int tx = nx + dx, ty = ny + dy;
                                    if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY)
                                    {
                                        bool clearPath = true; // проверка на наличие фигур между вражеской фигурой и дамкой
                                        int cx = x + dx, cy = y + dy;
                                        while (cx != nx || cy != ny)
                                        {
                                            if (board[cy][cx] != EMPTY)
                                            {
                                                clearPath = false;
                                                break;
                                            }
                                            cx += dx;
                                            cy += dy;
                                        }
                                        if (clearPath)
                                        {
                                            while (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY)
                                            {
                                                moves[moveCount++] = (Move){x, y, tx, ty, true, piece};
                                                tx += dx;
                                                ty += dy;
                                            }
                                        }
                                    }
                                    break;
                                }
                                else
                                    break;
                            }
                            nx += dx;
                            ny += dy;
                        }
                    }
                }
                else
                {
                    // Обычная шашка: теперь может бить во все 4 диагонали
                    int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                    for (int d = 0; d < 4; d++)
                    {
                        int dx = dirs[d][0], dy = dirs[d][1];
                        int mx = x + dx, my = y + dy;
                        int tx = x + 2 * dx, ty = y + 2 * dy;
                        if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE &&
                            mx >= 0 && mx < SIZE && my >= 0 && my < SIZE &&
                            isEnemy(board[my][mx], piece) && board[ty][tx] == EMPTY)
                        {
                            moves[moveCount++] = (Move){x, y, tx, ty, true, piece};
                        }
                    }
                }
            }
        }
    }

    // конец отсчета времени
    QueryPerformanceCounter(&end_time);

    // вычисление прошедшего времени в миллисекундах
    elapsed_ms_1 = (double)(end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;

    printf("Время выполнения анализа возможности взятия: %.5f миллисекунд\n", elapsed_ms_1);

    // начало отсчета времени
    QueryPerformanceCounter(&start_time);

    // --- Шаг 2: если нет взятий — собрать обычные ходы ---
    if (moveCount == 0)
    {
        for (int y = 0; y < SIZE; y++)
        {
            for (int x = 0; x < SIZE; x++)
            {
                int piece = board[y][x];
                if (piece == BLACK || piece == BLACK_KING)
                {
                    if (isKing(piece))
                    {
                        int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                        for (int d = 0; d < 4; d++)
                        {
                            int dx = dirs[d][0], dy = dirs[d][1];
                            int nx = x + dx, ny = y + dy;
                            while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[ny][nx] == EMPTY)
                            {
                                moves[moveCount++] = (Move){x, y, nx, ny, false, piece};
                                nx += dx;
                                ny += dy;
                            }
                        }
                    }
                    else
                    {
                        // Обычная шашка может двигаться только вверх, но бить теперь может назад
                        int dirs[2][2] = {{-1, -1}, {1, -1}};
                        for (int d = 0; d < 2; d++)
                        {
                            int dx = dirs[d][0], dy = dirs[d][1];
                            int nx = x + dx, ny = y + dy;
                            if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[ny][nx] == EMPTY)
                                moves[moveCount++] = (Move){x, y, nx, ny, false, piece};
                        }
                    }
                }
            }
        }
    }

    // конец отсчета времени
    QueryPerformanceCounter(&end_time);

    // вычисление прошедшего времени в миллисекундах
    elapsed_ms_2 = (double)(end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;

    printf("Время выполнения анализа обычного хода: %.5f миллисекунд\n", elapsed_ms_2);

    // начало отсчета времени
    QueryPerformanceCounter(&start_time);

    if (moveCount == 0)
    {
        printf("\n=== ИГРА ОКОНЧЕНА ===\n");
        printf("Белые победили! (у чёрных нет возможных ходов)\n");
        gameStarted = false;
        glutPostRedisplay();
        return;
    }

    // --- Шаг 3: оценить ходы ---
    int bestIdx = 0;             // индекс лучшего хода
    int bestScore = -1000000000; // оценка лучшего хода
    for (int i = 0; i < moveCount; i++)
    {
        int score = evaluateMoveScore(
            moves[i].fromX, moves[i].fromY,
            moves[i].toX, moves[i].toY,
            moves[i].isCapture, moves[i].piece);
        if (score > bestScore)
        {
            bestScore = score;
            bestIdx = i;
        }
    }

    // конец отсчета времени
    QueryPerformanceCounter(&end_time);

    // вычисление прошедшего времени в миллисекундах
    elapsed_ms_3 = (double)(end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;

    printf("Время оценки хода: %.5f миллисекунд\n", elapsed_ms_3);

    fprintf(f, "%.5lf;%.5lf;%.5lf\n", elapsed_ms_1, elapsed_ms_2, elapsed_ms_3);
    fclose(f);

    Move m = moves[bestIdx];             // берем лучший ход
    int piece = board[m.fromY][m.fromX]; // сохраняем для того чтобы определить шашка или дамка

    // --- Выполнить ход ---
    if (m.isCapture)
    {
        if (isKing(piece))
        {
            int dx = (m.toX > m.fromX) ? 1 : -1;
            int dy = (m.toY > m.fromY) ? 1 : -1;
            int x = m.fromX + dx;
            int y = m.fromY + dy;
            while (x != m.toX || y != m.toY)
            {
                if (isEnemy(board[y][x], piece))
                {
                    board[y][x] = EMPTY;
                    break;
                }
                x += dx;
                y += dy;
            }
        }
        else
        {
            int mx = (m.fromX + m.toX) / 2;
            int my = (m.fromY + m.toY) / 2;
            board[my][mx] = EMPTY;
        }
    }

    board[m.toY][m.toX] = piece;
    board[m.fromY][m.fromX] = EMPTY;
    promoteIfNeeded(m.toX, m.toY);

    // --- Многоходовое взятие во всех направлениях ---
    if (m.isCapture && canCaptureFrom(m.toX, m.toY))
    {
        selectedX = m.toX;
        selectedY = m.toY;
        makeBotMove(); // рекурсивное продолжение
        return;
    }

    // --- Завершение хода ---
    turn = WHITE;
    selectedX = selectedY = -1;

    // --- Проверка конца игры ---
    bool whiteCanMove = hasAnyMove(WHITE);
    bool blackCanMove = hasAnyMove(BLACK);
    if (!whiteCanMove || !blackCanMove)
    {
        printf("\n=== ИГРА ОКОНЧЕНА ===\n");
        if (!whiteCanMove && !blackCanMove)
            printf("Ничья! Ни у кого нет ходов.\n");
        else if (!whiteCanMove)
            printf("Чёрные победили!\n");
        else if (!blackCanMove)
            printf("Белые победили!\n");
        gameStarted = false;
    }
                                             
    printf("-------------------------------\n");

    glutPostRedisplay();
}

void checkGameEnd()
{
    bool whiteCanMove = hasAnyMove(WHITE);
    bool blackCanMove = hasAnyMove(BLACK);

    if (!whiteCanMove || !blackCanMove)
    {
        printf("\n=== ИГРА ОКОНЧЕНА ===\n");
        if (!whiteCanMove && !blackCanMove)
            printf("Ничья! Ни у кого нет ходов.\n");
        else if (!whiteCanMove)
            printf("Чёрные победили!\n");
        else if (!blackCanMove)
            printf("Белые победили!\n");

        gameStarted = false;
    }
}

// попытка сделать ход
void tryMove(int toX, int toY)
{

    int dx = toX - selectedX; // расстояние от выбранной шашки до целевой клетки
    int dy = toY - selectedY;
    int piece = board[selectedY][selectedX];

    if (board[toY][toX] != EMPTY)
        return;

    if (!isKing(piece))
    {
        // Обычная шашка
        bool isBlack = (piece == BLACK || piece == BLACK_KING);
        bool validStep = abs(dx) == 1 && ((isBlack && dy == -1) || (!isBlack && dy == 1)); // проверка на обычный (незахватывающий) одноклеточный ход
        bool validJump = abs(dx) == 2 && abs(dy) == 2;                                     // прыжок на 2 клетки (взятие шашки)

        if (validStep)
        {
            board[toY][toX] = piece;
            board[selectedY][selectedX] = EMPTY;
            promoteIfNeeded(toX, toY);
            selectedX = selectedY = -1;
            turn = (turn == BLACK) ? WHITE : BLACK;
            glutPostRedisplay();

            // Если сейчас ходит бот, не проверяем конец игры — проверит бот
            if (gameMode == 1 && turn == BLACK)
            {
                makeBotMove();
            }
            else
            {
                checkGameEnd();
            }
            return;
        }

        if (validJump)
        {
            int mx = selectedX + dx / 2;
            int my = selectedY + dy / 2;
            if (!isEnemy(board[my][mx], piece))
                return;

            board[my][mx] = EMPTY;
            board[toY][toX] = piece;
            board[selectedY][selectedX] = EMPTY;
            promoteIfNeeded(toX, toY);

            selectedX = toX;
            selectedY = toY;

            if (canCaptureFrom(toX, toY))
            {
                glutPostRedisplay();
                // Оставляем возможность продолжить ход
                return;
            }
            else
            {
                selectedX = selectedY = -1;
                turn = (turn == BLACK) ? WHITE : BLACK;
                glutPostRedisplay();

                if (gameMode == 1 && turn == BLACK)
                {
                    makeBotMove();
                }
                else
                {
                    checkGameEnd();
                }
            }
        }

    }

    else
    {
        // Дамка
        bool isCapture = false;
        if (canKingMove(selectedX, selectedY, toX, toY, &isCapture))
        {
            if (isCapture)
            {
                // Удаляем вражескую шашку на пути
                int stepX = dx > 0 ? 1 : -1;
                int stepY = dy > 0 ? 1 : -1;
                int x = selectedX + stepX;
                int y = selectedY + stepY;

                while (x != toX || y != toY)
                {
                    if (isEnemy(board[y][x], piece))
                    {
                        board[y][x] = EMPTY;
                        break;
                    }
                    x += stepX;
                    y += stepY;
                }
            }

            board[toY][toX] = piece;
            board[selectedY][selectedX] = EMPTY;

            if (isCapture && canCaptureFrom(toX, toY))
            {
                selectedX = toX;
                selectedY = toY;
                glutPostRedisplay();
                return;
            }
            else
            {
                selectedX = selectedY = -1;
                turn = (turn == BLACK) ? WHITE : BLACK;
                glutPostRedisplay();

                if (gameMode == 1 && turn == BLACK)
                {
                    makeBotMove();
                }
                else
                {
                    checkGameEnd();
                }
            }
        }

    }

}

// функция для мышки
void mouse(int button, int state, int x, int y)
{
    if (state != GLUT_DOWN)
        return;
    /*Преобразуем пиксельные координаты (x,y) в нормализованные координаты окна*/
    float fx = (float)x / glutGet(GLUT_WINDOW_WIDTH);
    float fy = 1.0f - (float)y / glutGet(GLUT_WINDOW_HEIGHT);

    if (!gameStarted)
    {
        if (fx > 0.2f && fx < 0.8f && fy > 0.5f && fy < 0.6f)
        {
            gameMode = 1; // против бота
            gameStarted = true;
            initBoard();
            glutPostRedisplay();
        }
        else if (fx > 0.2f && fx < 0.8f && fy > 0.3f && fy < 0.4f)
        {
            gameMode = 2;
            gameStarted = true;
            initBoard();
            glutPostRedisplay();
        }
        return;
    }

    float squareSize = 1.0f / (SIZE + 2); // величина клетки внутри поля
    float offset = squareSize;            // отступ слева снизу

    // Вычисляем границы области, в которой нарисована доска
    float boardLeft = offset;
    float boardRight = offset + SIZE * squareSize;
    float boardBottom = offset;
    float boardTop = offset + SIZE * squareSize;

    if (fx < boardLeft || fx > boardRight || fy < boardBottom || fy > boardTop)
        return;

    int bx = (fx - offset) / squareSize; // Переводим нормированные координаты fx, fy в индексы клетки
    int by = (fy - offset) / squareSize;

    if ((gameMode == 1 && turn != WHITE) || bx < 0 || bx >= SIZE || by < 0 || by >= SIZE)
        return;

    int clicked = board[by][bx];
    if ((turn == BLACK && (clicked == BLACK || clicked == BLACK_KING)) ||
        (turn == WHITE && (clicked == WHITE || clicked == WHITE_KING)))
    {
        selectedX = bx;
        selectedY = by;
    }
    else if (selectedX != -1)
    {
        tryMove(bx, by);
    }
    glutPostRedisplay();
}

// обработчик изменения размера окна
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char **argv)
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL, "");
    srand(time(NULL));                           // Инициализация генератора случайных чисел текущим временем
    glutInit(&argc, argv);                       // Инициализация библиотеки GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // Задаёт режим отображения, двойная буф, ргб
    glutInitWindowSize(800, 800);                // размер окна
    glutCreateWindow("Checkers game");           // название окна
    glutDisplayFunc(display);                    // Регистрируем функцию обратного вызова (callback) для отрисовки.
    glutMouseFunc(mouse);                        // Регистрируем функцию mouse() — обработчик событий мыши
    glutReshapeFunc(reshape);
    glClearColor(0.2f, 0.2f, 0.2f, 1); // серый фон после очистки
    glutMainLoop();                    // Запускаем основной цикл GLUT
    return 0;
}