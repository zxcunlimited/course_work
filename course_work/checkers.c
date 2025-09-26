#include <GL/glut.h> //��� �������� ����������
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define SIZE 8 //�����

//������������
typedef enum {
    EMPTY = 0,
    WHITE,
    BLACK,
    WHITE_KING,
    BLACK_KING
} Piece;

Piece board[SIZE][SIZE]; // ������������� �����
int selectedX = -1, selectedY = -1; // ��� ������ ������ (-1 ��� �� ������� ������)
bool gameStarted = false; //���� ��� ������ ����
int gameMode = 0; // 1 - ������� ������ ����, 2 - ������� ������ ��������
Piece turn = BLACK; //������������� �������� ���� (BLACK ��� �����������, �.�. �� ����� ���������)

//������� ������������� �����
void initBoard() {
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if ((x + y) % 2 == 1 && y < 3) board[y][x] = WHITE; // ����������� ����� �����
            else if ((x + y) % 2 == 1 && y > 4) board[y][x] = BLACK; // ����������
            else board[y][x] = EMPTY; //������ ������
        }
    }
    selectedX = selectedY = -1; //����� ������ ����� ����� ���������� ����
    turn = WHITE; //������ ��� �������� ����� �����
}

//�������� �������� �� ������ ���������
//piece - ����������� �����, ������� ����� ������ �����, current - ��������� ����� ������
bool isEnemy(int piece, int current) {
    if (piece == EMPTY) return false;
    if ((current == BLACK || current == BLACK_KING) && (piece == WHITE || piece == WHITE_KING)) return true;
    if ((current == WHITE || current == WHITE_KING) && (piece == BLACK || piece == BLACK_KING)) return true;
    return false; // ���� ������ ���� ��� ������ ������ ��� ��� ��������� ������
}

//�������� ������ �������� �� ��� ������
bool isKing(int piece) {
    return piece == WHITE_KING || piece == BLACK_KING;
}

//����������� � ����� ��� ���������� ����
void promoteIfNeeded(int x, int y) {
    if (board[y][x] == WHITE && y == SIZE - 1) board[y][x] = WHITE_KING;
    if (board[y][x] == BLACK && y == 0) board[y][x] = BLACK_KING;
}

// ������ ����� (�����)
// cx, cy - ����� ����� �� ����������� X � Y, r - ������ �����
void drawCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN); // ������������� ����� ������� � ������ ��� �������������, � ����� ����� ����� �������������
    glVertex2f(cx, cy); // ������ ����������� ����� �����
    for (int i = 0; i <= 100; i++) { //101 ����� ����� �������� ���������� ��� ��������
        float angle = 2.0f * 3.1415926f * i / 100; // ������� ���� � ��������, ����� ��� ������� �� ��� ���-�� � ����� �� 2 ��
        glVertex2f(cx + cosf(angle) * r, cy + sinf(angle) * r); //������ ���������� ��� ������������
    }
    glEnd(); //���������� ����������� ���������
}

//������� ������� �����
void drawBoard() {
    float squareSize = 1.0f / (SIZE + 2); // ������ ������ � ������ �������� (10% ����)
    float offset = squareSize; // ������ �� ����

    // ��������� �����
    glColor3f(0.3f, 0.2f, 0.1f); //���������� ���� 
    glBegin(GL_QUADS); // ����� ��������� ������������
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glVertex2f(0, 1);
    glEnd(); // ��������� ���������

    // ��������� ��������� (A-H)
    glColor3f(1, 1, 1); // ����� ����
    for (int x = 0; x < SIZE; x++) {
        glRasterPos2f(offset + x * squareSize + squareSize * 0.4f, offset - squareSize * 0.7f); //������������� ������� ���������� ������
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'A' + x);// ������ ������

        glRasterPos2f(offset + x * squareSize + squareSize * 0.4f, offset + SIZE * squareSize + squareSize * 0.3f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'A' + x);
    }

    // ��������� ������ (1-8)
    for (int y = 0; y < SIZE; y++) {
        glRasterPos2f(offset - squareSize * 0.7f, offset + y * squareSize + squareSize * 0.4f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '1' + (SIZE - 1 - y));

        glRasterPos2f(offset + SIZE * squareSize + squareSize * 0.3f, offset + y * squareSize + squareSize * 0.4f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '1' + (SIZE - 1 - y));
    }

    // ��������� ����� ������� ����� (�� ������)
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if ((x + y) % 2 == 0) glColor3f(255 / 255.0f, 254 / 255.0f, 122 / 255.0f); //������� ����
            else glColor3f(0.4f, 0.2f, 0.1f); //�����-����������

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
                glColor3f(0, 1, 0); //�������
                drawCircle(offset + (x + 0.5f) * squareSize, offset + (y + 0.5f) * squareSize, squareSize * 0.45f);
            }
        }
    }
}

//������� ���������� ������
void drawStartScreen() {
    // ������ ������
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(0.2f, 0.5f); glVertex2f(0.8f, 0.5f);
    glVertex2f(0.8f, 0.6f); glVertex2f(0.2f, 0.6f);
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(0.2f, 0.3f); glVertex2f(0.8f, 0.3f);
    glVertex2f(0.8f, 0.4f); glVertex2f(0.2f, 0.4f);
    glEnd();

    // ����� ����� �� �������
    glColor3f(1, 1, 1);
    glRasterPos2f(0.3f, 0.54f);
    const char* text1 = " PLAYER VS BOT";
    while (*text1) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text1++);

    glRasterPos2f(0.3f, 0.34f);
    const char* text2 = " PLAYER VS PLAYER";
    while (*text2) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text2++);

    // ������� ������� "CHECKERS"
    glColor3f(1, 1, 1); // ���� �������
    glRasterPos2f(0.32f, 0.75f);
    const char* title = "          CHECKERS";
    while (*title) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *title++);
}

//������� ������� ������ ������ ���� ����� ���������� ������ ���, ���������� GLUT
void display() {
    glClear(GL_COLOR_BUFFER_BIT); // ������� ������
    glLoadIdentity(); // ����� ���� �������������

    if (!gameStarted) drawStartScreen();//��������� �����
    else drawBoard();//����

    glutSwapBuffers(); // ���������� ������� ���� �� ������
}

//������� ��� �������� ����� �� ������ ������� ������ ��������� ������
bool canCaptureFrom(int x, int y) {
    int piece = board[y][x]; //��������� ������
    if (piece == EMPTY) return false;

    // ��� ������� �����
    if (!isKing(piece)) {
        int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };//������ ����������� �������� �����
        for (int d = 0; d < 4; d++) { //������� ���� 4 �����������
            int dx = dirs[d][0], dy = dirs[d][1]; // �������� x � y �� ������ ��������

            // ��������� ���������� �������� � ������� ������
            int mx = x + dx;
            int my = y + dy;
            int tx = x + 2 * dx;
            int ty = y + 2 * dy;

            //�������� �������� �� ����� ��������� � ���� �� �� ��� �����
            if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE &&
                isEnemy(board[my][mx], piece) && board[ty][tx] == EMPTY) {
                return true;
            }
        }
    }
    else { // ��� �����
        int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} }; //������ ����������� �������� �����
        for (int d = 0; d < 4; d++) { //������� ���� 4 �����������
            int dx = dirs[d][0], dy = dirs[d][1]; // �������� x � y �� ������ ��������
            int nx = x + dx, ny = y + dy; // ���������� �������� ������
            bool foundEnemy = false;//����, ������� ���������� ����� �� �� �����

            // ���� ��������� ����� �� ���������
            while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                if (board[ny][nx] != EMPTY) { //���� ������ �� ������
                    if (isEnemy(board[ny][nx], piece) && !foundEnemy) { //���������� ��� ��� ��������� ����� � ��� �� ��� �� ��������� � ���� �����������
                        foundEnemy = true;
                        // ��������� ������ �� ��������� ������
                        int tx = nx + dx, ty = ny + dy;
                        if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY) {
                            // ���������, ��� ����� ������ � ��������� ������ ��� ������ �����
                            bool clearPath = true; //���� �� ������� ������ ����� ������ � ��������� ������
                            int cx = x + dx, cy = y + dy; //������� ������
                            while (cx != nx || cy != ny) { //�� ��� ��� ���� �� ����� �������� �������
                                if (board[cy][cx] != EMPTY) {
                                    clearPath = false;
                                    break;
                                }
                                cx += dx; //���� � ��������� ������
                                cy += dy; //����������
                            }
                            if (clearPath) return true; //���� ��� �����
                        }
                        break;
                    }
                    else {
                        break; // ����� ���� ����� ��� ������ ���������, �.�. �� ����� ������������� ����� ���� ������
                    }
                }
                nx += dx; //���������� �������� �� ���������
                ny += dy;
            }
        }
    }
    return false; //���� ������ ����������
}

//����� �� ����� ������� ��� �� ����� ������ � ������
bool canKingMove(int fromX, int fromY, int toX, int toY, bool* isCapture) { //� ��������� ���������� �������� ������� ����� ����� �������� ������� ������� ����� ��������� �� ���������� ��� �� ������ ������
    int dx = toX - fromX; //���������� ����� ���������  
    int dy = toY - fromY;

    // ����� ������ ��������� �� ���������
    if (abs(dx) != abs(dy)) return false; //���� ��� ����������, �.�. ���������� �� ��������� �� ���������

    //���������� ����������� ���� � ����������
    int stepX = dx > 0 ? 1 : -1;
    int stepY = dy > 0 ? 1 : -1;
    int distance = abs(dx); //���������� �����
    int enemyCount = 0; //���-�� ������ � ����
    int enemyX = -1, enemyY = -1; //���������� ��������� �����

    // ��������� ���� �����
    for (int i = 1; i < distance; i++) {
        int x = fromX + i * stepX; //��������� ������� ������ �� ����   
        int y = fromY + i * stepY;

        if (board[y][x] != EMPTY) {
            if (isEnemy(board[y][x], board[fromY][fromX])) {
                enemyCount++; //����������� ������� ������
                enemyX = x; //����������� ���������� �����
                enemyY = y;
            }
            else {
                return false; // ���� ����� �� ���� 
            }
        }
    }

    if (enemyCount == 0) {
        *isCapture = false; //�������� ��� ��� �������
        return true; // ��� �������
    }
    else if (enemyCount == 1) {
        // ���������, ��� �� ��������� ������ �����
        int behindX = enemyX + stepX;
        int behindY = enemyY + stepY;

        if (behindX == toX && behindY == toY) { //���� ����� ����� ��������� ������� ������, �� ��� ���������� ������
            *isCapture = true; //��������� ��� ���� ������
            return true; // ��� �������
        }

        // ����� ����� ������������ �� ����� ������ �� ��������� ������
        while (behindX >= 0 && behindX < SIZE && behindY >= 0 && behindY < SIZE) { //���� � �������� ����
            if (board[behindY][behindX] != EMPTY) return false; //���� �� ���� ����� ����������� ������ ������, �� ��� ����������
            if (behindX == toX && behindY == toY) { //���� ������� ������ ��������� � �������
                *isCapture = true; //��������� ��� ���� ������
                return true; // ��� ��������
            }
            behindX += stepX; //��������� ������ �� ���������
            behindY += stepY;
        }
    }

    return false; //��� ����������
}

//������� ��� ���������� ���� ����
void makeBotMove() {
    typedef struct {
        int fromX, fromY, toX, toY; //���������� ������ � ���� ��������� ������
        bool isCapture; //�������� �� ��� �������
    } Move;

    Move moves[100]; // ��� �������� ���� ��������� �����
    int moveCount = 0; //������� ��������� �����

    // ��� 1: ������� ��� ��������� ������
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            int piece = board[y][x]; //��������� ������
            if (piece == BLACK || piece == BLACK_KING) {
                if (isKing(piece)) {
                    // ��� �����
                    int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} }; //��� 4 ��������� �����������
                    for (int d = 0; d < 4; d++) {
                        int dx = dirs[d][0], dy = dirs[d][1]; //����� ����������� ��� x � y
                        int nx = x + dx, ny = y + dy; //���������� �������� ������
                        bool foundEnemy = false; //����, ������� ���������� ����� �� �� �����

                        while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                            if (board[ny][nx] != EMPTY) {
                                if (isEnemy(board[ny][nx], piece) && !foundEnemy) { //���������� ��� ��� ��������� ����� � ��� �� ��� �� ��������� � ���� �����������
                                    foundEnemy = true; //����� �����
                                    // ��������� ������ �� ��������� ������
                                    int tx = nx + dx, ty = ny + dy; //���� �� ����� �� ��������� ������
                                    if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY) {
                                        // ���������, ��� ����� ������ � ��������� ������ ��� ������ �����
                                        bool clearPath = true; //���� ������� ��������� �� ������� ����� ����� ������ � ��������� �������
                                        int cx = x + dx, cy = y + dy; //����� ��� ������� ����������
                                        while (cx != nx || cy != ny) { // �� ��� ���, ���� �� ����� ������� �����������
                                            if (board[cy][cx] != EMPTY) {
                                                clearPath = false; //���� ������
                                                break;
                                            }
                                            cx += dx; //��������� ������ �� ���������
                                            cy += dy;
                                        }
                                        if (clearPath) { //���� ��� �����
                                            // ��������� ��� ��������� ������ �� ��������� ������
                                            while (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE && board[ty][tx] == EMPTY) {
                                                moves[moveCount++] = (Move){ x, y, tx, ty, true };
                                                tx += dx; //������� ���������� �� ������
                                                ty += dy;
                                            }
                                        }
                                    }
                                    break;
                                }
                                else {
                                    break; // ����� ���� ����� ��� ������ ���������
                                }
                            }
                            nx += dx; //������� ���������� �������� ������
                            ny += dy;
                        }
                    }
                }
                else {
                    // ��� ������� �����
                    int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} }; //��� 4 ��������� �����������
                    for (int d = 0; d < 4; d++) {
                        int dx = dirs[d][0], dy = dirs[d][1]; //����� ����������� ��� x � y
                        if (dy > 0) continue; // ������� ������ ������
                        int mx = x + dx, my = y + dy; //���������� �������� ������
                        int tx = x + 2 * dx, ty = y + 2 * dy; //���������� ������� ������
                        if (tx >= 0 && tx < SIZE && ty >= 0 && ty < SIZE &&
                            isEnemy(board[my][mx], piece) && board[ty][tx] == EMPTY) {
                            moves[moveCount++] = (Move){ x, y, tx, ty, true };
                        }
                    }
                }
            }
        }
    }

    // ��� 2: ���� ��� ������ � ������� ������� ����
    if (moveCount == 0) {
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {
                int piece = board[y][x];
                if (piece == BLACK || piece == BLACK_KING) {
                    if (isKing(piece)) {
                        // ��� �����
                        int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} }; //��� 4 ��������� �����������
                        for (int d = 0; d < 4; d++) {
                            int dx = dirs[d][0], dy = dirs[d][1]; //���������� ��� x � y
                            int nx = x + dx, ny = y + dy; //���������� ��� �������� ������

                            while (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[ny][nx] == EMPTY) {
                                moves[moveCount++] = (Move){ x, y, nx, ny, false };
                                nx += dx; //������ ���������� �������� ������
                                ny += dy;
                            }
                        }
                    }
                    else {
                        // ��� ������� �����
                        int dirs[4][2] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} }; //��� 4 ��������� �����������
                        for (int d = 0; d < 4; d++) {
                            int dx = dirs[d][0], dy = dirs[d][1];
                            if (dy > 0) continue; // ������� ������ ������
                            int nx = x + dx, ny = y + dy; //���������� ��� �������� ������
                            if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[ny][nx] == EMPTY) {
                                moves[moveCount++] = (Move){ x, y, nx, ny, false };
                            }
                        }
                    }
                }
            }
        }
    }

    if (moveCount == 0) return; // ��� �����

    // ��� 3: ������� ��������� ��� (����������� ������)
    Move m;
    bool hasCapture = false; //���� �� ������� ������
    for (int i = 0; i < moveCount; i++) { //���� �� ����������  ��������� �����
        if (moves[i].isCapture) { //�������� �� ��� �������
            hasCapture = true; //����� ������
            break;
        }
    }

    if (hasCapture) {
        int captureMoves = 0; //������� ������
        for (int i = 0; i < moveCount; i++) {
            if (moves[i].isCapture) captureMoves++;
        }
        m = moves[rand() % captureMoves]; //�������� ��������� ������ �� ���������� ������
    }
    else {
        m = moves[rand() % moveCount]; //�������� ����� ��������� ��� �� ���������� �����
    }

    int piece = board[m.fromY][m.fromX]; //�������� ������ ������� ����� ������

    // ��������� ��������� ���
    if (m.isCapture) {
        if (isKing(piece)) {
            // ��� ����� ������� ��������� ����� �� ����
            int dx = m.toX > m.fromX ? 1 : -1;
            int dy = m.toY > m.fromY ? 1 : -1;
            int x = m.fromX + dx; //������� ����� �� ���������
            int y = m.fromY + dy;

            while (x != m.toX || y != m.toY) { //���� �� ������� �����������
                if (isEnemy(board[y][x], piece)) {
                    board[y][x] = EMPTY;
                    break;
                }
                x += dx; //������� �����
                y += dy;
            }
        }
        else {
            // ��� ������� �����
            int mx = (m.fromX + m.toX) / 2; //����� �������� ����� ������� � ������, ��� ��� ��� ��� ��������� �������� �����
            int my = (m.fromY + m.toY) / 2;
            board[my][mx] = EMPTY;
        }
    }

    board[m.toY][m.toX] = piece; //��������� ���������� ��������� ������
    board[m.fromY][m.fromX] = EMPTY; //����� ��� ���� �����, ������ ���� ������
    promoteIfNeeded(m.toX, m.toY); //���� ����� �������� ����, �� ��� ������������ � �����

    // ��� 4: ���� ��� ���� ������ � ����������, ���� ��������
    if (m.isCapture && canCaptureFrom(m.toX, m.toY)) { //���� ��� �������� ������� � �������� ����� ������ ������� ������ ���������
        selectedX = m.toX; //��������� �������� ������� ���������
        selectedY = m.toY;
        makeBotMove(); // ���������� ���������� ������
    }
    else {
        turn = WHITE; //����� ����� �������
        selectedX = selectedY = -1; //���������� ����� ��
    }

    glutPostRedisplay(); //����������� ������
}

//������� ������� �������� �� ������� ��������� ��� ������
void tryMove(int toX, int toY) { //�� ���� ���������� ������� ������
    int dx = toX - selectedX; //������� ����� ������� � ������� �������
    int dy = toY - selectedY;
    int piece = board[selectedY][selectedX]; //��������� ������, ������� ����� ���������

    if (board[toY][toX] != EMPTY) return;

    if (!isKing(piece)) {
        // ������� �����
        bool isBlack = (piece == BLACK || piece == BLACK_KING); //��� ���� ������
        bool validStep = abs(dx) == 1 && ((isBlack && dy == -1) || (!isBlack && dy == 1)); //�������� ������� �������� ���� �� ��������� �� ���� ������
        bool validJump = abs(dx) == 2 && abs(dy) == 2; //�������� �� ������ ����� ���� ������

        if (validStep) {
            board[toY][toX] = piece; //������ ������ �� ������� ���������� 
            board[selectedY][selectedX] = EMPTY; //������ ������ ���������� �� ������
            promoteIfNeeded(toX, toY); //��� ���������� ���� ���������� � �����
            selectedX = selectedY = -1; //���������� ���������� �����
            turn = (turn == BLACK) ? WHITE : BLACK;
            glutPostRedisplay(); //�������������� �����

            if (gameMode == 1 && turn == BLACK) makeBotMove(); //���� ��� ����� ���� � ����� ������, �� ���������� ������� ���� 
            return;
        }

        if (validJump) {
            int mx = selectedX + dx / 2; //�������� ��������, �.�. ��������� ������
            int my = selectedY + dy / 2;
            if (!isEnemy(board[my][mx], piece)) return; //���� �� ��� ���������, �� ���� �� ������

            board[my][mx] = EMPTY;
            board[toY][toX] = piece;
            board[selectedY][selectedX] = EMPTY;
            promoteIfNeeded(toX, toY);

            selectedX = toX; //���������� ����� ������� �����, ��� ���������
            selectedY = toY;

            if (canCaptureFrom(toX, toY)) { //�������� ����� �� ������� ������
                glutPostRedisplay(); //��������� �����
            }
            else {
                selectedX = selectedY = -1; //����� ������ �����
                turn = (turn == BLACK) ? WHITE : BLACK;
                if (gameMode == 1 && turn == BLACK) makeBotMove();
                glutPostRedisplay();
            }
        }
    }
    else {
        // �����
        bool isCapture = false; //���� �������� �� ���� �������
        if (canKingMove(selectedX, selectedY, toX, toY, &isCapture)) { //�������� �������� �� ��� ��� �����
            if (isCapture) {
                // ������� � ������� ��������� �����
                int stepX = dx > 0 ? 1 : -1; //���������� ����������� �������� �����
                int stepY = dy > 0 ? 1 : -1;
                int x = selectedX + stepX; //�������� �����
                int y = selectedY + stepY;

                while (x != toX || y != toY) {
                    if (isEnemy(board[y][x], piece)) {
                        board[y][x] = EMPTY;
                        break;
                    }
                    x += stepX; //������� ������ �����
                    y += stepY;
                }
            }

            board[toY][toX] = piece;
            board[selectedY][selectedX] = EMPTY;
            promoteIfNeeded(toX, toY);

            if (isCapture && canCaptureFrom(toX, toY)) { //�������� �� ��� ������� � �������� ����� �� ������ ������� ������
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

//������� ������� ������������ ������� �����
void mouse(int button, int state, int x, int y) { //� ���������� ����� ������� ������, ��������� ������ � ���������� ����� � ��������
    if (state != GLUT_DOWN) return; //��������� ��� ��� ������� ������: ���� ����� ������, �� ����� ���
    //��������� ���������� ������ ���� �� 0 �� 1
    float fx = (float)x / glutGet(GLUT_WINDOW_WIDTH); //������� ����������� ������ � ��������������� 
    float fy = 1.0f - (float)y / glutGet(GLUT_WINDOW_HEIGHT); //����������� Y, �.�. � ������ (0,0) ��� ����� ������ ����

    //��������� ����� �� ���� � ���� �� ������ ������ ����
    if (!gameStarted) {
        if (fx > 0.2f && fx < 0.8f && fy > 0.5f && fy < 0.6f) {
            gameMode = 1;
            gameStarted = true; //���� ���������� 
            initBoard(); //������ ����
            glutPostRedisplay(); //��������� �����
        }
        else if (fx > 0.2f && fx < 0.8f && fy > 0.3f && fy < 0.4f) {
            gameMode = 2;
            gameStarted = true;
            initBoard();
            glutPostRedisplay();
        }
        return;
    }

    float squareSize = 1.0f / (SIZE + 2); // ���������� ������ ������ ������ ���� (10% �� ����)
    float offset = squareSize; // ������ �� ���� ����

    // ��������� ������� �������� ����
    float boardLeft = offset; // 0.1f
    float boardRight = offset + SIZE * squareSize;// 0.9f
    float boardBottom = offset;// 0.1f
    float boardTop = offset + SIZE * squareSize;// 0.9f

    if (fx < boardLeft || fx > boardRight || fy < boardBottom || fy > boardTop) return; // ���� ���� ��� ������, �� �� ������������

    int bx = (fx - offset) / squareSize; // ��������� � ����� ������ ����� ����� 
    int by = (fy - offset) / squareSize;

    if ((gameMode == 1 && turn != WHITE) || bx < 0 || bx >= SIZE || by < 0 || by >= SIZE) return;

    int clicked = board[by][bx]; //�������� ��� ������ �� ������
    //��������� ������� �� ����� �� ���� ������
    if ((turn == BLACK && (clicked == BLACK || clicked == BLACK_KING)) ||
        (turn == WHITE && (clicked == WHITE || clicked == WHITE_KING))) {
        selectedX = bx; //���������� ������ ��� ���������
        selectedY = by;
    }
    else if (selectedX != -1) { //���� ��� ���� ��������� ������, �� ������� ������� ���
        tryMove(bx, by);
    }
    glutPostRedisplay(); //��������� ����
}

//������� ������� ���������� ��� ��������� ������� ����
void reshape(int w, int h) {
    glViewport(0, 0, w, h); //������������� ������� ������ ����, �.�. ��� �������� ������� ���� �������� �������
    glMatrixMode(GL_PROJECTION); //������������� �� ������ � �������� ��������, ���������� ��� ����� ������������ ���������� �� ������
    glLoadIdentity(); //���������� �������������
    gluOrtho2D(0, 1, 0, 1); //������������� ��������� ��������������� ��������  
    glMatrixMode(GL_MODELVIEW); //������������� ������� �����
    glLoadIdentity(); //���������� �������������
}

int main(int argc, char** argv) { //��������� ����� ��� ��� ��� ������������� 
    srand(time(NULL));
    glutInit(&argc, argv); //������������ ������������� glut    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); //��������� ������ �����������, ���� ����� ������������ �� ������, ������ ������������ ��� ���������� �����, ����� �������� ��������, ���� ������ ��� ������ rgb
    glutInitWindowSize(800, 800); //���������������� ����� ����
    glutCreateWindow("Checkers game"); //�������� ����
    glutDisplayFunc(display); //������������ ������� �������, ����, ����� ������ � �.�.
    glutMouseFunc(mouse);//������������ ������� ��� ������
    glutReshapeFunc(reshape); //������������ ������� ��������� ������� ����
    glClearColor(0.2f, 0.2f, 0.2f, 1); //�����-����� ��� ��� ����
    glutMainLoop(); //��� ����������� ��������� �������
    return 0;
}