#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <conio.h>
#include <errno.h>    

void hideCursor(){
    printf("\e[?25l");
}

void showCursor(){
    printf("\e[?25h");
}

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

#define ROWS 10 
#define COLS 30 
#define WALL '#'
#define MAX_SNAKE_LENGTH (ROWS * COLS)

typedef struct{
    int X;
    int Y;
} Point;

typedef struct{
    Point body[MAX_SNAKE_LENGTH];
    int length;
} Snake;

#define clrscr() printf("\e[1;1H\e[2J")
void gotoxy(int x,int y){
    printf("%c[%d;%df",0x1B,y,x);
}

void drawBoundaries(){
    for(int row = 0; row < ROWS + 2; row++){
        for(int col = 0; col < COLS + 2; col++){
            if(row == 0 || row == ROWS + 1 || col == 0 || col == COLS + 1){
                printf("%c", WALL);         
            }
            else{
                printf(" ");
            }
        }
        printf("\n");
    }
}

void drawScore(int score){
    gotoxy(1, ROWS + 3);
    printf("Poang: %d   ", score);
}

void spawnFood(Snake *snake, Point *food){
    int valid = 0;
    while(!valid){
        // Inre spelplanen ligger på koordinaterna 2 till COLS+1 (och 2 till ROWS+1)
        food->X = (rand() % COLS) + 2;
        food->Y = (rand() % ROWS) + 2;
        
        valid = 1;
        // Kontrollera att maten inte hamnar på ormen
        for(int i = 0; i < snake->length; i++){
            if(snake->body[i].X == food->X && snake->body[i].Y == food->Y){
                valid = 0;
                break;
            }
        }
    }
    gotoxy(food->X, food->Y);
    printf("*");
}

typedef enum {
    Dir_Left,    
    Dir_Right,
    Dir_Up,
    Dir_Down,
} Dir;

int kbhit2() {
    return _kbhit();
}

int main(){
    srand(time(NULL));
    hideCursor();
    
    int playAgain = 1;

    // Yttre loop som hanterar omstart av spelet
    while(playAgain){
        Snake snake;
        snake.length = 3; // Återställ startlängd
        for(int i = 0; i < snake.length; i++){
            snake.body[i].X = 10 - i;
            snake.body[i].Y = 6;
        }

        Point food;
        int score = 0;
        Dir dir = Dir_Right;
        int gameOver = 0;

        clrscr();
        drawBoundaries();
        spawnFood(&snake, &food);
        drawScore(score);

        // Rita ut startormen
        for(int i = 0; i < snake.length; i++){
            gotoxy(snake.body[i].X, snake.body[i].Y);
            printf("%c", (i == 0) ? 'O' : 'o');
        }

        // Inre spelloop för den pågående rundan
        while(!gameOver){
            if(kbhit2()){
                char ch = getch();
                if(ch == 'u' && dir != Dir_Down) dir = Dir_Up;
                else if(ch == 'd' && dir != Dir_Up) dir = Dir_Down;
                else if(ch == 'l' && dir != Dir_Right) dir = Dir_Left;
                else if(ch == 'r' && dir != Dir_Left) dir = Dir_Right;
                else if(ch == 'q') {
                    gameOver = 1;
                    playAgain = 0;
                    break;
                }
            }

            // Rensa svansen
            gotoxy(snake.body[snake.length - 1].X, snake.body[snake.length - 1].Y);
            printf(" ");

            // Flytta kroppen framåt
            for(int i = snake.length - 1; i > 0; i--){
                snake.body[i] = snake.body[i - 1];
            }

            // Flytta huvudet
            if(dir == Dir_Up) snake.body[0].Y--;
            if(dir == Dir_Down) snake.body[0].Y++;
            if(dir == Dir_Left) snake.body[0].X--;
            if(dir == Dir_Right) snake.body[0].X++;

            // 1. Kollisionsdetektering med väggar
            if(snake.body[0].X < 2 || snake.body[0].X > COLS + 1 || snake.body[0].Y < 2 || snake.body[0].Y > ROWS + 1){
                gotoxy(1, ROWS + 5);
                printf("GAME OVER! Du krockade med vaggen.\n");
                gameOver = 1;
                break;
            }

            // 2. Kollisionsdetektering med sig själv
            for(int i = 1; i < snake.length; i++){
                if(snake.body[0].X == snake.body[i].X && snake.body[0].Y == snake.body[i].Y){
                    gotoxy(1, ROWS + 5);
                    printf("GAME OVER! Du krockade med dig sjalv.\n");
                    gameOver = 1;
                    break;
                }
            }
            if(gameOver) break;

            // Kontrollera om ormen äter maten
            if(snake.body[0].X == food.X && snake.body[0].Y == food.Y){
                score += 10;
                if(snake.length < MAX_SNAKE_LENGTH){
                    snake.length++;
                    // Initiera den nya svansdelen så den inte ritas ut på (0,0) första bildrutan
                    snake.body[snake.length - 1] = snake.body[snake.length - 2];
                }
                spawnFood(&snake, &food);
                drawScore(score);
            }

            // Rita ut ormen (Huvud 'O', kropp 'o')
            for(int i = 0; i < snake.length; i++){
                gotoxy(snake.body[i].X, snake.body[i].Y);
                printf("%c", (i == 0) ? 'O' : 'o');
            }

            msleep(400);
        }

        // Fråga om spelaren vill köra igen (om man inte avslutade med 'q')
        if(playAgain){
            gotoxy(1, ROWS + 6);
            printf("Spela igen? (j/n): ");
            
            while(1){
                char choice = getch();
                if(choice == 'j' || choice == 'J' || choice == 'y' || choice == 'Y'){
                    playAgain = 1;
                    break;
                }
                else if(choice == 'n' || choice == 'N' || choice == 'q' || choice == 'Q'){
                    playAgain = 0;
                    break;
                }
            }
        }
    }

    showCursor();
    gotoxy(1, ROWS + 8);
    printf("Tack for att du spelade!\n");
    return 0;
}