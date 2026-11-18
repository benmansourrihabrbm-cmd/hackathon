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

// --- POÄNGTEXT UNDER SPELET ---
// Skriver ut aktuell poäng på raden nedanför spelplanen
void drawScore(int score){
    gotoxy(1, ROWS + 3);
    printf("Poang: %d   ", score);
}

// --- SLUMPA UT MAT ---
// Obs: Får inte vara på ormen eller utanför spelplanen!
void spawnFood(Snake *snake, Point *food){
    int valid = 0;
    while(!valid){
        // Slumpar fram koordinater innanför väggarna (från 2 till COLS+1 / ROWS+1)
        food->X = (rand() % COLS) + 2;
        food->Y = (rand() % ROWS) + 2;
        
        valid = 1;
        // Kontrollerar att maten inte hamnar på någon av ormens kroppsdelar
        for(int i = 0; i < snake->length; i++){
            if(snake->body[i].X == food->X && snake->body[i].Y == food->Y){
                valid = 0; // Om den hamnar på ormen, slingan kör om och slumpar nytt
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

    while(playAgain){
        Snake snake;
        snake.length = 3;
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

        // --- ÄNDRA TILL STORT O, SÅ KAN KROPPEN SENARE VARA LILLA o ---
        // Huvudet (index 0) ritas ut med stort 'O', resten av kroppen med lilla 'o'
        for(int i = 0; i < snake.length; i++){
            gotoxy(snake.body[i].X, snake.body[i].Y);
            printf("%c", (i == 0) ? 'O' : 'o');
        }

        while(!gameOver){
            if(kbhit2()){
                int ch = getch();
                if(ch == 0 || ch == 224){
                    int arrow = getch();
                    if(arrow == 72 && dir != Dir_Down) dir = Dir_Up;
                    else if(arrow == 80 && dir != Dir_Up) dir = Dir_Down;
                    else if(arrow == 75 && dir != Dir_Right) dir = Dir_Left;
                    else if(arrow == 77 && dir != Dir_Left) dir = Dir_Right;
                } 
                else {
                    if((ch == 'u' || ch == 'U') && dir != Dir_Down) dir = Dir_Up;
                    else if((ch == 'd' || ch == 'D') && dir != Dir_Up) dir = Dir_Down;
                    else if((ch == 'l' || ch == 'L') && dir != Dir_Right) dir = Dir_Left;
                    else if((ch == 'r' || ch == 'R') && dir != Dir_Left) dir = Dir_Right;
                    else if(ch == 'q' || ch == 'Q') {
                        gameOver = 1;
                        playAgain = 0;
                        break;
                    }
                }
            }

            // Rensa svansen på skärmen
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

            // --- COLLISION DETECTION ---
            // 1. Kontrollera om huvudet krockar med väggarna
            if(snake.body[0].X < 1 || snake.body[0].X > COLS  || snake.body[0].Y < 1 || snake.body[0].Y > ROWS + 1){
                gotoxy(1, ROWS + 5);
                printf("GAME OVER! Du krockade med vaggen.\n");
                gameOver = 1;
                break;
            }

            // 2. Kontrollera om huvudet krockar med sin egen kropp (självkollision)
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
                
                // --- GER POÄNG ---
                score += 100; 
                
                // --- MASKEN VÄXER ---
                if(snake.length < MAX_SNAKE_LENGTH){
                    snake.length++;
                    snake.body[snake.length - 1] = snake.body[snake.length - 2];
                }
                
                // --- SLUMPA UT NY MAT ---
                spawnFood(&snake, &food); 
                drawScore(score);         
            }

            // Rita ut ormen igen på dess nya positioner
            for(int i = 0; i < snake.length; i++){
                gotoxy(snake.body[i].X, snake.body[i].Y);
                printf("%c", (i == 0) ? 'O' : 'o');
            }

            msleep(400); // Spelhastighet (paus i millisekunder)
        }

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