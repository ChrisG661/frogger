#include <stdio.h>

#define SIZE 9
#define TRUE 1
#define FALSE 0

enum tile_type { LILLYPAD, BANK, WATER, TURTLE, LOG };

struct board_tile {
    enum tile_type type;    // attribute 1
    int occupied;           // attribute 2
};

void print_board(struct board_tile board[SIZE][SIZE]);
void init_board(struct board_tile board[SIZE][SIZE]);
char type_to_char(enum tile_type);

int main(void) 
{
    printf("Welcome to Frogger Game!\n");
    struct board_tile game_board[SIZE][SIZE];


    init_board(game_board);

    int num_turtle;
    printf("How many turtles? ");
    scanf("%d", &num_turtle);
    int turtle_x, turtle_y;
    if (num_turtle > 0) printf("Enter pairs:\n");

    for (int i = 0; i < num_turtle; i++) 
    {
        scanf("%d %d", &turtle_x, &turtle_y);
        if ((turtle_x > 0) && (turtle_x < 8) && 
        (turtle_y > 0) && (turtle_y < 8) && 
        (!game_board[turtle_x][turtle_y].occupied)) 
        game_board[turtle_x][turtle_y].type = TURTLE;
    }

print_board(game_board);

char command;
printf("Enter Command: ");
while (scanf("%c\n", &command) != EOF) 
{
    if (command != NULL) {
        print_board(game_board);
        printf("Enter Command: ");
    }
}

return 0;
}
                
void init_board(struct board_tile board[SIZE][SIZE]) 
{
    for (int row = 0; row < SIZE; row++) 
    {
        for (int col = 0; col < SIZE; col++) 
        {
            board[row][col].occupied = FALSE;
            if (row == 0) {
                if (col % 2 == 0)
                    board[row][col].type = LILLYPAD;
                else
                    board[row][col].type = WATER;
            } else if (row == 8) {
                board[row][col].type = BANK;
                if (col == 4)
                    board[row][col].occupied = TRUE;
            } else {
                board[row][col].type = WATER;
            }
        }
    }
}

void print_board(struct board_tile board[SIZE][SIZE]) 
{
    char type_char = '\0';
    for (int row = 0; row < SIZE; row++) 
    {
        for (int col = 0; col < SIZE; col++) 
        {
            if (board[row][col].occupied)
                type_char = 'F';
            else
                type_char = type_to_char(board[row][col].type);
            printf("%c ", type_char);
        }
        printf("\n");
    }
}

char type_to_char(enum tile_type type)
{
    char type_char = ' ';
    if(type == LILLYPAD){
        type_char = 'o';
    } else if(type == BANK){
        type_char = 'x';
    } else if(type == WATER){
        type_char = '~';
    } else if(type == TURTLE){
        type_char = 'T';
    } else if(type == LOG){
        type_char = 'L';
    }
    return type_char;
}
