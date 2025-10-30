#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <locale.h>
#include <Windows.h>
#include <time.h>

#define SIZE 8 // ��� ������, ������� ����� ������ �������� ����.

typedef enum
{ // ������������ ������� ���������� ����� ������ ����� ���� �� �����
    EMPTY = 0,
    WHITE,
    BLACK,
    WHITE_KING,
    BLACK_KING
} Piece;

Piece board[SIZE][SIZE];            // ������� ����
int selectedX = -1, selectedY = -1; // ���������� ��������� ����� (-1 ��� ������ �� �������)
bool gameStarted = false;           // ���� ����������� ������ �� ����
int gameMode = 0;                   // 1 - ������� ������ ����, 2 - ������� ������ ��������
Piece turn;                         // ���������� ��� ��� ������, ������ ��� �����

// �������������� ����� �������
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

// �������� �������� �� ����� ���������
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

// ��������� �������� �� ����� ������
bool isKing(int piece)
{
    return piece == WHITE_KING || piece == BLACK_KING;
}

// ��������� ����� �� �����
void promoteIfNeeded(int x, int y)
{
    if (board[y][x] == WHITE && y == SIZE - 1)
        board[y][x] = WHITE_KING;
    if (board[y][x] == BLACK && y == 0)
        board[y][x] = BLACK_KING;
}

// ��������� �����
void drawCircle(float cx, float cy, float r)
{                             /*���������� ������ � ������ �����*/
    glBegin(GL_TRIANGLE_FAN); // ��� ����� �������������
    glVertex2f(cx, cy);       // ����������� ������ �������, �.�. ����� �����
    for (int i = 0; i <= 100; i++)
    {                                                           /*������ 100 ������������� ������ ������*/
        float angle = 2.0f * 3.1415926f * i / 100;              // ��������� ������� ���� � �������� ��� ������ ����� �� ����������
        glVertex2f(cx + cosf(angle) * r, cy + sinf(angle) * r); // ��������� ������� �� ���������� (����� �� ���� �����)
    }
    glEnd(); // ���������
}

// ��������� �������� ����
void drawBoard()
{
    float squareSize = 1.0f / (SIZE + 2); // ������ ����� ������ (0.1 �� ����� ����)
    float offset = squareSize;            // �������� �� ������/������� ���� �� ������ ����� �����

    // Draw border
    glColor3f(0.3f, 0.2f, 0.1f); // ������������� ������� ���� ��� ��������� (�����-����������)
    glBegin(GL_QUADS);           // ������������� ��� ��� �������
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glVertex2f(0, 1); // ����������� ���� �����
    glEnd();          // ���������

    // Draw coordinate letters (A-H)
    glColor3f(1, 1, 1); // ����� ���� ��� ������ ���������
    for (int x = 0; x < SIZE; x++)
    {
        glRasterPos2f(offset + x * squareSize + squareSize * 0.4f, offset - squareSize * 0.7f); // ������������ ������� ��� ��������� �������� �����
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'A' + x);                                 // ������������� ����� � ������ ������� �� ��������

        glRasterPos2f(offset + x * squareSize + squareSize * 0.4f, offset + SIZE * squareSize + squareSize * 0.3f); // ������
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'A' + x);
    }

    // Draw coordinate numbers (1-8)
    for (int y = 0; y < SIZE; y++)
    {
        glRasterPos2f(offset - squareSize * 0.7f, offset + y * squareSize + squareSize * 0.4f); // ������������ ���������� ��� ����� (�����)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '1' + (SIZE - 1 - y));                    // ��������� �� 8 �� 1, �� ���������� ����� �����

        glRasterPos2f(offset + SIZE * squareSize + squareSize * 0.3f, offset + y * squareSize + squareSize * 0.4f); // ������
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '1' + (SIZE - 1 - y));
    }

    // Draw checkerboard
    for (int y = 0; y < SIZE; y++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            if ((x + y) % 2 == 0)
                glColor3f(255 / 255.0f, 254 / 255.0f, 122 / 255.0f); // ������� ������
            else
                glColor3f(0.4f, 0.2f, 0.1f); // ���������� ������

            glBegin(GL_QUADS); // ������ ���������� ������, ������������ ��� ��� �������
            glVertex2f(offset + x * squareSize, offset + y * squareSize);
            glVertex2f(offset + (x + 1) * squareSize, offset + y * squareSize);
            glVertex2f(offset + (x + 1) * squareSize, offset + (y + 1) * squareSize);
            glVertex2f(offset + x * squareSize, offset + (y + 1) * squareSize);
            glEnd(); // ���������

            int piece = board[y][x]; // ������ ����������� �� ������, ���� ��� �� ������, �� ������ �����
            if (piece != EMPTY)
            {
                if (piece == WHITE || piece == WHITE_KING)
                    glColor3f(1, 1, 1);
                else
                    glColor3f(0, 0, 0);
                drawCircle(offset + (x + 0.5f) * squareSize, offset + (y + 0.5f) * squareSize, squareSize * 0.4f); // ������ �����

                if (isKing(piece))
                {
                    glColor3f(1, 0, 0); // ������� ����
                    drawCircle(offset + (x + 0.5f) * squareSize, offset + (y + 0.5f) * squareSize, squareSize * 0.2f);
                }
            }

            if (x == selectedX && y == selectedY)
            { // ���� ��� ��������� �����, �� ����������� �� � ������� ����
                glColor3f(0, 1, 0);
                drawCircle(offset + (x + 0.5f) * squareSize, offset + (y + 0.5f) * squareSize, squareSize * 0.45f);
            }
        }
    }
}

// ����, ��������� �����
void drawStartScreen()
{
    // ������ ������
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS); // ������ ���������������, ������ ������
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

    // ����� ����� �� �������
    glColor3f(1, 1, 1);
    glRasterPos2f(0.3f, 0.54f);
    const char *text1 = " PLAYER VS BOT";
    while (*text1)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text1++);

    glRasterPos2f(0.3f, 0.34f);
    const char *text2 = " PLAYER VS PLAYER";
    while (*text2)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text2++);

    // ������� ������� "CHECKERS"
    glColor3f(1, 1, 1); // ���� �������
    glRasterPos2f(0.32f, 0.75f);
    const char *title = "          CHECKERS";
    while (*title)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *title++);
}

// ��� ����������� ����
void display()
{
    glClear(GL_COLOR_BUFFER_BIT); // ��������� �����
    glLoadIdentity();             // ���������� ������� ��������������

    if (!gameStarted)
        drawStartScreen(); // ���� ���� �� ������, �� ������ ����
    else
        drawBoard(); // ����� �����

    glutSwapBuffers(); // ������������ ������� ����������� ��� ��������� ��������
}

// �������� ����� �� ������ ��������� ������
bool canCaptureFrom(int x, int y)
{
    int piece = board[y][x]; // ��������� ���������� ������
    if (piece == EMPTY)
        return false; // ���� ������

    // ��� ������� �����
    if (!isKing(piece))
    {
        int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
        for (int d = 0; d < 4; d++)
        {
            int dx = dirs[d][0], dy = dirs[d][1]; // ���������� ������ �� ������ �����������

            int mx = x + dx; // ������������� ������
            int my = y + dy;
            int tx = x + 2 * dx; // ������� ������
            int ty = y + 2 * dy;

            // ��������� ������� ��� ��������� ������
            if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE &&
                mx >= 0 && mx < SIZE && my >= 0 && my < SIZE &&
                isEnemy(board[my][mx], piece) && board[ty][tx] == EMPTY)
            {
                return true;
            }
        }
    }
    else
    { // ��� �����
        int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
        for (int d = 0; d < 4; d++)
        {
            int dx = dirs[d][0], dy = dirs[d][1];
            int nx = x + dx, ny = y + dy; // ���������� ������ ������ �� ��������� �� �����
            bool foundEnemy = false;

            // ���� ��������� ����� �� ���������
            while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
            {
                if (board[ny][nx] != EMPTY)
                {
                    if (isEnemy(board[ny][nx], piece) && !foundEnemy)
                    { // ���� ����� ��������� �����
                        foundEnemy = true;
                        // ��������� ������ �� ��������� ������
                        int tx = nx + dx, ty = ny + dy; // �������� ������ �� ������
                        if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY)
                        {
                            // ���������, ��� ����� ������ � ��������� ������ ��� ������ �����
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
                        break; // ����� ���� ����� ��� ������ ���������
                    }
                }
                nx += dx;
                ny += dy;
            }
        }
    }
    return false;
}

// ��������� true ���� ����� ����� ��������� ���
bool canKingMove(int fromX, int fromY, int toX, int toY, bool *isCapture)
{                         /*isCapture ��� ���� �� ������*/
    int dx = toX - fromX; // ��������� ������� � �����������
    int dy = toY - fromY;

    // ����� ������ ��������� �� ���������
    if (abs(dx) != abs(dy))
        return false;

    int stepX = dx > 0 ? 1 : -1; // ����������� ���� ��� ����
    int stepY = dy > 0 ? 1 : -1;
    int distance = abs(dx);       // ���������
    int enemyCount = 0;           // ������� ��������� ����� ����������� �� ����
    int enemyX = -1, enemyY = -1; // ���������� ��������� ��������� ������

    // ��������� ���� �����
    for (int i = 1; i < distance; i++)
    {
        int x = fromX + i * stepX; // ���������� ������ ������������� ������
        int y = fromY + i * stepY;

        if (board[y][x] != EMPTY)
        {
            if (isEnemy(board[y][x], board[fromY][fromX]))
            { // ���� ��������� ������
                enemyCount++;
                enemyX = x;
                enemyY = y;
            }
            else
            {
                return false; // ���� ����� �� ����
            }
        }
    }

    if (enemyCount == 0)
    {
        *isCapture = false;
        return true; // ������ ��� ��� ������
    }
    else if (enemyCount == 1)
    {
        // ���������, ��� �� ��������� ������ �����
        int behindX = enemyX + stepX;
        int behindY = enemyY + stepY;

        if (behindX == toX && behindY == toY)
        {
            *isCapture = true;
            return true;
        }

        // ����� ����� ������������ �� ����� ������ �� ��������� ������
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

/* ---------- ����� ��������������� ������� ��� ������ ����� ���� ---------- */

/* ��������� ������� ����� � temp � ��������������� �� temp */
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

/* ���������� ��� ��������� ������ ��� ����� 'color' � ���������� ����� ������ moves
   ����������: ����� ��������� ������ */
// ��������� ��� �����
typedef struct
{
    int fromX, fromY, toX, toY;
    bool isKing;
} FastMove;

/* �������� ��������� ������ ��� ���������� ������� (������������ ��� �������� ����������) */
int generateCaptureMovesForColor(int color, FastMove outMoves[], int maxMoves)
{
    int count = 0; // ������� ��������� ������
    for (int y = 0; y < SIZE; y++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            int piece = board[y][x];
            if (piece == EMPTY)
                continue;
            bool pieceIsColor = ((color == BLACK) && (piece == BLACK || piece == BLACK_KING)) ||
                                ((color == WHITE) && (piece == WHITE || piece == WHITE_KING)); // ���������, ����������� �� ������ � ������� ������ ���������� color
            if (!pieceIsColor)
                continue;

            if (isKing(piece))
            {
                int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                for (int d = 0; d < 4; d++)
                {
                    int dx = dirs[d][0], dy = dirs[d][1]; // ��������� �����������
                    int nx = x + dx, ny = y + dy;         // ���������� ������� ������ ����� ������
                    bool foundEnemy = false;              // ���� �� ������� �����
                    while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
                    { /*���� ���� �� ������ �� ������� ����*/
                        if (board[ny][nx] != EMPTY)
                        {
                            if (isEnemy(board[ny][nx], piece) && !foundEnemy)
                            {
                                foundEnemy = true;
                                int tx = nx + dx, ty = ny + dy; // ������ ���������� ������� ������, �� ��������� �������
                                if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY)
                                {                          /*���� � �������� ���� � ������ ������*/
                                    bool clearPath = true; // ���� �� ������� ����� ����� ��������� ��������� ������� � ������
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
                                    { /*���� ���� �� ��������� ������ ����, �� ����� ����� ������������ �� ����� ������ ������, ������� ������ �� ������ (�� ������ �� ������).*/
                                        while (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY)
                                        {
                                            /*outMoves � ������ FastMove, � ������� ���������� ��������� ���� � �������.
                                            maxMoves � ������������ ���������� ���������, ������� ��������� �������� � outMoves.*/
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
                    // ������� �����: direction restriction handled via isEnemy checks / board coords
                    int mx = x + dx, my = y + dy;
                    int tx = x + 2 * dx, ty = y + 2 * dy;
                    if (mx >= 0 && mx < SIZE && my >= 0 && my < SIZE &&
                        tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE &&
                        isEnemy(board[my][mx], piece) && board[ty][tx] == EMPTY)
                    {
                        if (count < maxMoves)
                            outMoves[count] = (FastMove){x, y, tx, ty, false}; // ���� ������ ��������� � ���������� ��� � outMoves
                        count++;
                    }
                }
            }
        }
    }
    return count; // ���������� ����� ����� ��������� ��������� ������ count
}

/* ��������� � ���� �� � ���������� ������, ������� ������ ������, ������� � (targetX,targetY)
   ���������� true, ���� ����� ������ ����. ��� ������� ���������� �� ������� ����� (��������������, ��� ��� ��� ������). */
bool opponentCanCaptureSquare(int targetX, int targetY, int opponentColor)
{
    FastMove moves[200];
    int cnt = generateCaptureMovesForColor(opponentColor, moves, 200);
    // ��� ������� ������ ����� ������, ����� ���������� ��������� ������.
    for (int i = 0; i < cnt && i < 200; i++)
    {
        int fx = moves[i].fromX, fy = moves[i].fromY, tx = moves[i].toX, ty = moves[i].toY;
        if (!moves[i].isKing)
        {
            // ������� ������ � ��������� ������ � ��������
            int mx = (fx + tx) / 2;
            int my = (fy + ty) / 2;
            if (mx == targetX && my == targetY)
                return true;
        }
        else
        {
            // ����� � ����� ����� ��������� ������ �� ��������� ����� fx,fy � tx,ty
            int dx = (tx > fx) ? 1 : -1; // �������� �����������
            int dy = (ty > fy) ? 1 : -1;
            int cx = fx + dx, cy = fy + dy; // ��������� ��������� ����������
            while (cx != tx || cy != ty)
            {
                if (cx == targetX && cy == targetY)
                    return true; // ����� ����� ������ ��� ������
                if (board[cy][cx] != EMPTY && !(cx == targetX && cy == targetY))
                {
                    // ��� �����-�� ������ (�� ���� ��� �� target � ��� ����������� ��� ������� ��������)
                    break;
                }
                cx += dx;
                cy += dy;
            }
        }
    }
    return false;
}

/* ���������� ������� ������������ ���������� ��������� ����� (�������) ��� ����, �� � ������������ ��������.
   ��� ��������: ������� ���������� ������ �������, ������������ � one capture move (from->to),
   � ���������� �������� �� depthLimit. */
int simulateCaptureChainCount(int fromX, int fromY, int toX, int toY, Piece piece, int depthLimit)
{
    Piece temp[SIZE][SIZE]; // ��������� � �������� ����� �� ��������� ����������
    backupBoard(temp);

    // ��������� ���� ������ (������� ���������)
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
    promoteIfNeeded(toX, toY); // ���� �������� ����� ����, �� ����������

    int best = 1; // ��� ������ 1 ������
    if (depthLimit > 1 && canCaptureFrom(toX, toY))
    {
        // ������ ��� ��������� ����������� ������ � ������� toX,toY
        // ����������� ��������� ���� � �� ����� �������� ������������ ����� ������ ��������� ������
        // ��� �������� ������ ���� � ���������� ������� ��������
        // �������� ������������� capture moves from toX,toY
        Piece curPiece = board[toY][toX];
        if (curPiece != EMPTY)
        {
            // ��� �����
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
                                        int chain = 1 + simulateCaptureChainCount(toX, toY, curtx, curty, curPiece, depthLimit - 1); // ���������� ����������� ������� ������
                                        if (chain > best)
                                            best = chain; // ������������� � ���������� �� ��������
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
                        int chain = 1 + simulateCaptureChainCount(toX, toY, tx, ty, curPiece, depthLimit - 1); // ���������� ����������� ������� ������
                        if (chain > best)
                            best = chain;
                    }
                }
            }
        }
    }

    restoreBoard(temp); // ��������������� �����, ����� ��� �� ���� ��������
    return best;        // ���������� ������������ ���������� ������ ����� � �������
}

/* ������ ����: +�� ����� �������, +�� ����������� � �����, -�� ���������� */
int evaluateMoveScore(int fromX, int fromY, int toX, int toY, bool isCapture, Piece piece)
{
    int score = 0; // ��� ������ + � -

    // ������� ����������
    if (isCapture)
    {
        // ��������� ����� ������� (������������ �������)
        int chain = simulateCaptureChainCount(fromX, fromY, toX, toY, piece, 4); // ������� 4
        score += chain * 200;                                                    // ��� ��� ������ ���������� �������
    }

    // �������� ����������� � ����� (��� ������: y ���������� �� 0)
    if (!isKing(piece))
    {
        if (piece == BLACK)
        {
            int distBefore = fromY;
            int distAfter = toY;
            if (distAfter < distBefore)
                score += (distBefore - distAfter) * 5;
            // ����� ���� ����� �� ��������� �����
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
        // ��� ����� ��������� ����� �� ���������� (��� ������ � ����� � ��� �����)
        int centerDist = abs(toX - SIZE / 2) + abs(toY - SIZE / 2);
        score += (20 - centerDist);
    }

    // ����� �� ����������: ���� ����� ���������� ���� ��������� ����� ������ ���� ������ � �����
    Piece temp[SIZE][SIZE];
    backupBoard(temp);
    // ��������� ��� �� �����
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

    // ��������, ����� �� �������� ����� ������ �� toX,toY
    int opponentColor = (piece == BLACK || piece == BLACK_KING) ? WHITE : BLACK;
    if (opponentCanCaptureSquare(toX, toY, opponentColor))
    {
        score -= 180; // ������� ����� �� ���� ����� ���������
    }
    else
    {
        score += 10; // ��������� ����� �� ������������
    }

    // ����������� �����
    restoreBoard(temp);

    return score;
}

/* ---------- ����� ��������������� ������� ---------- */

// ���������� true ���� ���� � ������ ���� �� ���� ���������� ���
bool hasAnyMove(Piece color)
{
    for (int y = 0; y < SIZE; y++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            int piece = board[y][x];
            if (piece == EMPTY)
                continue;

            // ���������, ����������� �� ������ ������� �����
            bool isMyPiece = ((color == WHITE) && (piece == WHITE || piece == WHITE_KING)) ||
                             ((color == BLACK) && (piece == BLACK || piece == BLACK_KING));
            if (!isMyPiece)
                continue;

            // �������� ��������� ������
            if (canCaptureFrom(x, y))
                return true;

            // �������� ������� �����
            if (isKing(piece))
            {
                // ����� ����� ��������� �� ���������� �� �����������
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
                // ������� �����
                int dirs[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                for (int d = 0; d < 4; d++)
                {
                    int dx = dirs[d][0], dy = dirs[d][1];
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[ny][nx] == EMPTY)
                    {
                        // ��� ������ � �������� �����, ��� ����� � ����
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
    /* �����: �������� �� ������� */
    // ���� �� ����� C:\Users\danya\source\repos\zxcunlimited\course_work\course_work-1\checkers_better\data.csv
    FILE* f = fopen("data.csv", "a"); // ��������� ���� � ������ ��������
    if (!f) {
        perror("�� ������� ������� ����");
        return;
    }

    printf("------���������� ��������------\n");

    // ��������� ��� 64-������ ����� �����
    LARGE_INTEGER start_time, end_time, frequency;

    // �������� ������� �������� ������������������
    QueryPerformanceFrequency(&frequency);

    // ������ ������� �������
    QueryPerformanceCounter(&start_time);

    double elapsed_ms_1, elapsed_ms_2, elapsed_ms_3;
    if (!gameStarted)
        return; // ���� ���� �� ��������

    typedef struct
    {
        int fromX, fromY, toX, toY;
        bool isCapture;
        Piece piece;
    } Move;

    Move moves[400];   // ������ ��� ���������� ���� ��������� �����
    int moveCount = 0; // ������� ��������� �����

    // --- ��������: ���� �� ������ ���� ---
    bool whiteCanMoveBefore = hasAnyMove(WHITE);
    bool blackCanMoveBefore = hasAnyMove(BLACK);
    if (!whiteCanMoveBefore || !blackCanMoveBefore)
    {
        printf("\n=== ���� �������� ===\n");
        if (!whiteCanMoveBefore && !blackCanMoveBefore)
            printf("�����! �� � ���� ��� �����.\n");
        else if (!whiteCanMoveBefore)
            printf("׸���� ��������!\n");
        else if (!blackCanMoveBefore)
            printf("����� ��������!\n");
        gameStarted = false;
        glutPostRedisplay(); // ��������� ����
        return;
    }

    // --- ��� 1: ������� ��� ��������� ������ ---
    for (int y = 0; y < SIZE; y++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            int piece = board[y][x];
            if (piece == BLACK || piece == BLACK_KING)
            {
                if (isKing(piece))
                {
                    // ����� ����� ���� � ����� �����������
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
                                        bool clearPath = true; // �������� �� ������� ����� ����� ��������� ������� � ������
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
                    // ������� �����: ������ ����� ���� �� ��� 4 ���������
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

    // ����� ������� �������
    QueryPerformanceCounter(&end_time);

    // ���������� ���������� ������� � �������������
    elapsed_ms_1 = (double)(end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;

    printf("����� ���������� ������� ����������� ������: %.5f �����������\n", elapsed_ms_1);

    // ������ ������� �������
    QueryPerformanceCounter(&start_time);

    // --- ��� 2: ���� ��� ������ � ������� ������� ���� ---
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
                        // ������� ����� ����� ��������� ������ �����, �� ���� ������ ����� �����
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

    // ����� ������� �������
    QueryPerformanceCounter(&end_time);

    // ���������� ���������� ������� � �������������
    elapsed_ms_2 = (double)(end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;

    printf("����� ���������� ������� �������� ����: %.5f �����������\n", elapsed_ms_2);

    // ������ ������� �������
    QueryPerformanceCounter(&start_time);

    if (moveCount == 0)
    {
        printf("\n=== ���� �������� ===\n");
        printf("����� ��������! (� ������ ��� ��������� �����)\n");
        gameStarted = false;
        glutPostRedisplay();
        return;
    }

    // --- ��� 3: ������� ���� ---
    int bestIdx = 0;             // ������ ������� ����
    int bestScore = -1000000000; // ������ ������� ����
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

    // ����� ������� �������
    QueryPerformanceCounter(&end_time);

    // ���������� ���������� ������� � �������������
    elapsed_ms_3 = (double)(end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;

    printf("����� ������ ����: %.5f �����������\n", elapsed_ms_3);

    fprintf(f, "%.5lf;%.5lf;%.5lf\n", elapsed_ms_1, elapsed_ms_2, elapsed_ms_3);
    fclose(f);

    Move m = moves[bestIdx];             // ����� ������ ���
    int piece = board[m.fromY][m.fromX]; // ��������� ��� ���� ����� ���������� ����� ��� �����

    // --- ��������� ��� ---
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

    // --- ������������ ������ �� ���� ������������ ---
    if (m.isCapture && canCaptureFrom(m.toX, m.toY))
    {
        selectedX = m.toX;
        selectedY = m.toY;
        makeBotMove(); // ����������� �����������
        return;
    }

    // --- ���������� ���� ---
    turn = WHITE;
    selectedX = selectedY = -1;

    // --- �������� ����� ���� ---
    bool whiteCanMove = hasAnyMove(WHITE);
    bool blackCanMove = hasAnyMove(BLACK);
    if (!whiteCanMove || !blackCanMove)
    {
        printf("\n=== ���� �������� ===\n");
        if (!whiteCanMove && !blackCanMove)
            printf("�����! �� � ���� ��� �����.\n");
        else if (!whiteCanMove)
            printf("׸���� ��������!\n");
        else if (!blackCanMove)
            printf("����� ��������!\n");
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
        printf("\n=== ���� �������� ===\n");
        if (!whiteCanMove && !blackCanMove)
            printf("�����! �� � ���� ��� �����.\n");
        else if (!whiteCanMove)
            printf("׸���� ��������!\n");
        else if (!blackCanMove)
            printf("����� ��������!\n");

        gameStarted = false;
    }
}

// ������� ������� ���
void tryMove(int toX, int toY)
{

    int dx = toX - selectedX; // ���������� �� ��������� ����� �� ������� ������
    int dy = toY - selectedY;
    int piece = board[selectedY][selectedX];

    if (board[toY][toX] != EMPTY)
        return;

    if (!isKing(piece))
    {
        // ������� �����
        bool isBlack = (piece == BLACK || piece == BLACK_KING);
        bool validStep = abs(dx) == 1 && ((isBlack && dy == -1) || (!isBlack && dy == 1)); // �������� �� ������� (���������������) ������������� ���
        bool validJump = abs(dx) == 2 && abs(dy) == 2;                                     // ������ �� 2 ������ (������ �����)

        if (validStep)
        {
            board[toY][toX] = piece;
            board[selectedY][selectedX] = EMPTY;
            promoteIfNeeded(toX, toY);
            selectedX = selectedY = -1;
            turn = (turn == BLACK) ? WHITE : BLACK;
            glutPostRedisplay();

            // ���� ������ ����� ���, �� ��������� ����� ���� � �������� ���
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
                // ��������� ����������� ���������� ���
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
        // �����
        bool isCapture = false;
        if (canKingMove(selectedX, selectedY, toX, toY, &isCapture))
        {
            if (isCapture)
            {
                // ������� ��������� ����� �� ����
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

// ������� ��� �����
void mouse(int button, int state, int x, int y)
{
    if (state != GLUT_DOWN)
        return;
    /*����������� ���������� ���������� (x,y) � ��������������� ���������� ����*/
    float fx = (float)x / glutGet(GLUT_WINDOW_WIDTH);
    float fy = 1.0f - (float)y / glutGet(GLUT_WINDOW_HEIGHT);

    if (!gameStarted)
    {
        if (fx > 0.2f && fx < 0.8f && fy > 0.5f && fy < 0.6f)
        {
            gameMode = 1; // ������ ����
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

    float squareSize = 1.0f / (SIZE + 2); // �������� ������ ������ ����
    float offset = squareSize;            // ������ ����� �����

    // ��������� ������� �������, � ������� ���������� �����
    float boardLeft = offset;
    float boardRight = offset + SIZE * squareSize;
    float boardBottom = offset;
    float boardTop = offset + SIZE * squareSize;

    if (fx < boardLeft || fx > boardRight || fy < boardBottom || fy > boardTop)
        return;

    int bx = (fx - offset) / squareSize; // ��������� ������������� ���������� fx, fy � ������� ������
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

// ���������� ��������� ������� ����
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
    srand(time(NULL));                           // ������������� ���������� ��������� ����� ������� ��������
    glutInit(&argc, argv);                       // ������������� ���������� GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // ����� ����� �����������, ������� ���, ���
    glutInitWindowSize(800, 800);                // ������ ����
    glutCreateWindow("Checkers game");           // �������� ����
    glutDisplayFunc(display);                    // ������������ ������� ��������� ������ (callback) ��� ���������.
    glutMouseFunc(mouse);                        // ������������ ������� mouse() � ���������� ������� ����
    glutReshapeFunc(reshape);
    glClearColor(0.2f, 0.2f, 0.2f, 1); // ����� ��� ����� �������
    glutMainLoop();                    // ��������� �������� ���� GLUT
    return 0;
}