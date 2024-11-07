// Jumping Tian
//
// This program was written by Group 5
// Christopher Marchel Mark Gijoh (01082240011)
// Farrel Fyelo Hollans Tamaela   (01082240024)
// Gian Jeconiah Sitompul         (01082240025)
// Kenny Lay                      (01082240010)
// on 1) October 25, 2024 (Starter code)
//    2) October 31, 2024 (Phase 1.3)
//    3) November 7, 2024 (Phase 2.3)
//
// TODO: Description of program

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  CONSTANTS  /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Provided constants
#define SIZE 9
#define TRUE 1
#define FALSE 0
#define XSTART 8
#define YSTART 4

// You may choose to add additional #defines here.
#define TILE board[row][col]

// Provided Enums
enum tile_type
{
    LILLYPAD,
    BANK,
    WATER,
    TURTLE,
    LOG
};

// Additional Enums
enum direction
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    STAY
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////  STRUCTS  //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Provided structs
struct board_tile
{
    enum tile_type type; // The type of piece it is (water, bank, etc.)
    int occupied;        // TRUE or FALSE based on if Frogger is there.
};

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  FUNCTION PROTOTYPES  ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Your function prototypes here
void init_board(struct board_tile board[SIZE][SIZE]);
void add_log(struct board_tile board[SIZE][SIZE], int, int, int);
void clear_row(struct board_tile board[SIZE][SIZE], int);
void remove_log(struct board_tile board[SIZE][SIZE], int, int);
void move_frogger(struct board_tile board[SIZE][SIZE], int *x, int *y, enum direction);

// Prints out the current state of the board.
void print_board(struct board_tile board[SIZE][SIZE]);
char type_to_char(enum tile_type type);

////////////////////////////////////////////////////////////////////////////////
//////////////////////////  FUNCTION IMPLEMENTATIONS  //////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(void)
{

    printf("Welcome to Frogger Game!\n");
    struct board_tile game_board[SIZE][SIZE];

    // (Stage 1.1) Initialise the gameboard.
    init_board(game_board);

    // Read user input and place turtles.
    printf("How many turtles? ");

    // (Stage 1.2): Scan in the turtles, and place them on the map.
    int num_turtles;
    scanf("%d", &num_turtles);
    int turtle_x, turtle_y;
    if (num_turtles > 0)
        printf("Enter pairs:\n");

    for (int i = 0; i < num_turtles; i++)
    {
        scanf("%d %d", &turtle_x, &turtle_y);
        if ((turtle_x > 0 && turtle_x < SIZE - 1) && (turtle_y >= 0 || turtle_y <= SIZE - 1) && (!game_board[turtle_x][turtle_y].occupied))
            game_board[turtle_x][turtle_y].type = TURTLE;
    }

    // Start the game and print out the gameboard.
    printf("Game Started\n");
    print_board(game_board);

    // (Stage 1.3): Create a command loop, to read and execute commands!
    printf("Enter command: ");
    char command;
    int x, y;
    int y_start, y_end;
    int x_frog = XSTART, y_frog = YSTART;

    while (scanf(" %c", &command) != EOF)
    {
        if (command == 'l')
        {
            scanf(" %d %d %d", &x, &y_start, &y_end);
            if ((x > 0) && (x < SIZE - 1))
                add_log(game_board, x, y_start, y_end);
        }
        else if (command == 'c')
        {
            scanf(" %d", &x);
            if ((x > 0) && (x < SIZE - 1))
                clear_row(game_board, x);
        }
        else if (command == 'r')
        {
            scanf(" %d %d", &x, &y);
            if ((x > 0) && (x < SIZE - 1))
                remove_log(game_board, x, y);
        }
        else
        {
            direction move_direction = STAY;
            switch (command)
            {
            case 'w':
                move_direction = FORWARD;
                break;
            case 's':
                move_direction = BACKWARD;
                break;
            case 'a':
                move_direction = LEFT;
                break;
            case 'd':
                move_direction = RIGHT;
                break;

            default:
                break;
            }
            move_frogger(game_board, &x_frog, &y_frog, move_direction);
        }
        print_board(game_board);
        printf("Enter command: ");
    }

    printf("Thank you for playing Frogger Game!\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// ADDITIONAL FUNCTIONS /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// TODO: Add more functions here!

void init_board(struct board_tile board[SIZE][SIZE])
{
    for (int row = 0; row < SIZE; row++)
    {
        for (int col = 0; col < SIZE; col++)
        {
            TILE.occupied = FALSE;
            if (row == 0)
            {
                if (col % 2 == 0)
                    TILE.type = LILLYPAD;
                else
                    TILE.type = WATER;
            }
            else if (row == SIZE - 1)
            {
                TILE.type = BANK;
                if (col == SIZE / 2)
                    TILE.occupied = TRUE;
            }
            else
                TILE.type = WATER;
        }
    }
}

void add_log(struct board_tile board[SIZE][SIZE], int x, int y_start, int y_end)
{
    if (y_start < 0)
        y_start = 0;
    if (y_end > SIZE - 1)
        y_end = SIZE - 1;
    for (int i = 0; i < SIZE; i++)
        if (board[x][i].type == TURTLE)
            return;
    for (int i = y_start; i <= y_end; i++)
        board[x][i].type = LOG;
}

void clear_row(struct board_tile board[SIZE][SIZE], int x)
{
    for (int i = 0; i < SIZE; i++)
        if (board[x][i].occupied)
            return;
    for (int i = 0; i < SIZE; i++)
        board[x][i].type = WATER;
}

void remove_log(struct board_tile board[SIZE][SIZE], int x, int y)
{
    for (int i = 0; i < SIZE; i++)
        if (board[x][i].occupied)
            return;

    if (board[x][y].type != LOG)
        return;
    else
        board[x][y].type = WATER;

    int i = y + 1, j = y - 1;
    while (board[x][i].type == LOG)
    {
        board[x][i].type = WATER;
        i++;
    }
    while (board[x][j].type == LOG)
    {
        board[x][j].type = WATER;
        i--;
    }
}

void move_frogger(struct board_tile board[SIZE][SIZE], int *x, int *y, enum direction move_direction)
{
    if (move_direction == STAY)
        return;

    int new_x = *x, new_y = *y;
    switch (move_direction)
    {
    case FORWARD:
        new_x--;
        break;
    case BACKWARD:
        new_x++;
        break;
    case LEFT:
        new_y--;
        break;
    case RIGHT:
        new_y++;
        break;
    default:
        break;
    }

    if (new_x < 0 || new_x >= SIZE || new_y < 0 || new_y >= SIZE)
        return;

    board[*x][*y].occupied = FALSE;
    board[new_x][new_y].occupied = TRUE;
    *x = new_x;
    *y = new_y;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// PROVIDED FUNCTIONS //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void print_board(struct board_tile board[SIZE][SIZE])
{
    for (int row = 0; row < SIZE; row++)
    {
        for (int col = 0; col < SIZE; col++)
        {
            char type_char = '\0';
            if (TILE.occupied)
            {
                type_char = 'F';
            }
            else
            {
                type_char = type_to_char(TILE.type);
            }
            printf("%c ", type_char);
        }
        printf("\n");
    }
}

char type_to_char(enum tile_type type)
{
    char type_char = ' ';
    if (type == LILLYPAD)
    {
        type_char = 'o';
    }
    else if (type == BANK)
    {
        type_char = 'x';
    }
    else if (type == WATER)
    {
        type_char = '~';
    }
    else if (type == TURTLE)
    {
        type_char = 'T';
    }
    else if (type == LOG)
    {
        type_char = 'L';
    }
    return type_char;
}
