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
#include <iostream>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  CONSTANTS  /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Provided constants
#define SIZE 9
#define TRUE 1
#define FALSE 0
#define XSTART SIZE - 1
#define YSTART SIZE / 2
#define LIVES 3

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

// Additional structs

struct bug
{
    bool present;             // TRUE or FALSE based on if a bug is there.
    enum direction direction; // The direction the bug is moving.
};

// Provided structs
struct board_tile
{
    enum tile_type type; // The type of piece it is (water, bank, etc.)
    int occupied;        // TRUE or FALSE based on if Frogger is there.
    struct bug bug;      // The bug that is on the tile.
};

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  FUNCTION PROTOTYPES  ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Your function prototypes here
void init_board(struct board_tile board[SIZE][SIZE]);
void add_turtle(struct board_tile board[SIZE][SIZE], int, int);
void add_log(struct board_tile board[SIZE][SIZE], int, int, int);
void clear_row(struct board_tile board[SIZE][SIZE], int);
void remove_log(struct board_tile board[SIZE][SIZE], int, int);
void move_frogger(struct board_tile board[SIZE][SIZE], int *, int *, enum direction);
void add_bug(struct board_tile board[SIZE][SIZE], int, int);
void remove_bug(struct board_tile board[SIZE][SIZE], int, int);
void move_bugs(struct board_tile board[SIZE][SIZE]);

// Prints out the current state of the board.
void print_board(struct board_tile board[SIZE][SIZE]);
char type_to_char(enum tile_type type);

////////////////////////////////////////////////////////////////////////////////
//////////////////////////  FUNCTION IMPLEMENTATIONS  //////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(void)
{
    cout << "Welcome to Frogger Game!" << '\n';
    struct board_tile game_board[SIZE][SIZE];

    // (Stage 1.1) Initialise the gameboard.
    init_board(game_board);

    // Read user input and place turtles.
    cout << "How many turtles? ";

    // (Stage 1.2): Scan in the turtles, and place them on the map.
    int num_turtles;
    cin >> num_turtles;
    int turtle_x, turtle_y;
    if (num_turtles > 0)
        cout << "Enter pairs:" << '\n';

    for (int i = 0; i < num_turtles; i++)
    {
        cin >> turtle_x >> turtle_y;
        add_turtle(game_board, turtle_x, turtle_y);
    }

    // Start the game and print out the gameboard.
    cout << "Game Started" << '\n';
    print_board(game_board);

    // (Stage 1.3): Create a command loop, to read and execute commands!
    cout << "Enter command: ";
    char command;
    int x, y;
    int y_start, y_end;
    int x_frog = XSTART, y_frog = YSTART;
    int lives = LIVES;

    while (cin >> command)
    {
        if (command == 'l')
        {
            cin >> x >> y_start >> y_end;
            add_log(game_board, x, y_start, y_end);
        }
        else if (command == 'c')
        {
            cin >> x;
            clear_row(game_board, x);
        }
        else if (command == 'r')
        {
            cin >> x >> y;
            remove_log(game_board, x, y);
        }
        else if (command == 'b')
        {
            cin >> x >> y;
            add_bug(game_board, x, y);
        }
        else if (command == 'q')
        {
            cout << "Quitting game..." << '\n';
            break;
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
            move_bugs(game_board);

            if (game_board[x_frog][y_frog].type == LILLYPAD)
            {
                print_board(game_board);
                cout << "\nWahoo!! You Won!\n";
                break;
            }
            else if ((game_board[x_frog][y_frog].type == WATER) ||
                     (game_board[x_frog][y_frog].bug.present))
            {
                lives--;
                print_board(game_board);
                if (!lives)
                {
                    cout << "\n !! GAME OVER !!\n\n";
                    break;
                }
                else
                {
                    game_board[x_frog][y_frog].occupied = FALSE;
                    cout << "\n# LIVES LEFT: " << lives << " #\n\n";
                    x_frog = XSTART, y_frog = YSTART;
                    game_board[x_frog][y_frog].occupied = TRUE;
                }
            }
            else
            {
                print_board(game_board);
                cout << "Enter command: ";
                continue;
            }
        }
        print_board(game_board);
        cout << "Enter command: ";
    }

    cout << "Thank you for playing Frogger Game!" << '\n';
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
            board[row][col].occupied = FALSE;
            board[row][col].bug.present = FALSE;
            board[row][col].bug.direction = RIGHT;
            if (row == 0)
            {
                if (col % 2 == 0)
                    board[row][col].type = LILLYPAD;
                else
                    board[row][col].type = WATER;
            }
            else if (row == SIZE - 1)
            {
                board[row][col].type = BANK;
                if (col == SIZE / 2)
                    board[row][col].occupied = TRUE;
            }
            else
                board[row][col].type = WATER;
        }
    }
}

void add_turtle(struct board_tile board[SIZE][SIZE], int x, int y)
{
    if (board[x][y].occupied || x < 0 || x >= SIZE || y < 0 || y >= SIZE)
        return;
    if (board[x][y].type == WATER)
        board[x][y].type = TURTLE;
}

void add_log(struct board_tile board[SIZE][SIZE], int x, int y_start, int y_end)
{
    if (x < 0 || x >= SIZE)
        return;
    for (int i = 0; i < SIZE; i++)
        if (board[x][i].type == TURTLE)
            return;

    if (y_start < 0)
        y_start = 0;
    if (y_end > SIZE - 1)
        y_end = SIZE - 1;
    for (int i = y_start; i <= y_end; i++)
        board[x][i].type = LOG;
}

void clear_row(struct board_tile board[SIZE][SIZE], int x)
{
    if (x < 0 || x >= SIZE)
        return;
    for (int i = 0; i < SIZE; i++)
        if (board[x][i].occupied)
            return;
    for (int i = 0; i < SIZE; i++)
    {
        board[x][i].type = WATER;
        remove_bug(board, x, i);
    }
}

void remove_log(struct board_tile board[SIZE][SIZE], int x, int y)
{
    if (x < 0 || x >= SIZE)
        return;
    if (board[x][y].type != LOG)
        return;

    for (int i = 0; i < SIZE; i++)
        if (board[x][i].occupied)
            return;

        else
        {
            board[x][y].type = WATER;
            remove_bug(board, x, y);
        }

    int i = y + 1, j = y - 1;
    while (board[x][i].type == LOG)
    {
        board[x][i].type = WATER;
        remove_bug(board, x, i);
        i++;
    }
    while (board[x][j].type == LOG)
    {
        board[x][j].type = WATER;
        remove_bug(board, x, j);
        j--;
    }
}

void move_frogger(struct board_tile board[SIZE][SIZE], int *x, int *y, direction move_direction)
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

void add_bug(struct board_tile board[SIZE][SIZE], int x, int y)
{
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE)
        return;

    if (board[x][y].type == LOG || board[x][y].type == TURTLE)
    {
        board[x][y].bug.present = TRUE;
        board[x][y].bug.direction = RIGHT;
    }
}

void remove_bug(struct board_tile board[SIZE][SIZE], int x, int y)
{
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE)
        return;

    if (board[x][y].bug.present)
    {
        board[x][y].bug.present = FALSE;
        board[x][y].bug.direction = RIGHT;
    }
}

void move_bugs(struct board_tile board[SIZE][SIZE])
{
    for (int row = 0; row < SIZE; row++)
    {
        for (int col = 0; col < SIZE; col++)
        {
            if (!board[row][col].bug.present)
                continue;

            direction dir = board[row][col].bug.direction;
            int next_col = dir == RIGHT ? col + 1 : col - 1;

            if (next_col < 0 || next_col >= SIZE)
            {
                dir = dir == RIGHT ? LEFT : RIGHT;
                next_col = dir == RIGHT ? col + 1 : col - 1;
                if (board[row][next_col].bug.present)
                    continue;
            }

            tile_type current_type = board[row][col].type;
            tile_type next_type = board[row][next_col].type;

            if (!(next_type == LOG || next_type == TURTLE) || board[row][next_col].bug.present)
            {
                dir = dir == RIGHT ? LEFT : RIGHT;
                next_col = dir == RIGHT ? col + 1 : col - 1;
                next_type = board[row][next_col].type;
                if (!(next_type == LOG || next_type == TURTLE) || board[row][next_col].bug.present)
                    continue;
            }

            if (current_type == LOG || current_type == TURTLE)
            {
                board[row][col].bug.present = FALSE;
                board[row][next_col].bug.present = TRUE;
                board[row][next_col].bug.direction = dir;
                if (dir == RIGHT)
                    col++;
            }
        }
    }
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
            if (board[row][col].occupied)
            {
                type_char = 'F';
            }
            else if (board[row][col].bug.present)
            {
                type_char = 'B';
            }
            else
            {
                type_char = type_to_char(board[row][col].type);
            }
            cout << type_char << " ";
        }
        cout << '\n';
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
