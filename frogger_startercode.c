// Program name
//
// This program was written by [your name] (NIM i.e. )
// on [date]
//
// TODO: Description of program

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  CONSTANTS  /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Provided constants
#define SIZE        9
#define TRUE        1
#define FALSE       0

// TODO: you may choose to add additional #defines here.

// Provided Enums
enum tile_type {LILLYPAD, BANK, WATER, TURTLE, LOG};

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////  STRUCTS  //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Provided structs
struct board_tile {
    enum tile_type type; // The type of piece it is (water, bank, etc.)
    int occupied;        // TRUE or FALSE based on if Frogger is there.
};

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  FUNCTION PROTOTYPES  ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// TODO: Your function prototypes here

// Prints out the current state of the board.
void print_board(struct board_tile board[SIZE][SIZE]);
void init_board(struct board_tile board[SIZE][SIZE]);
char type_to_char(enum tile_type type);

////////////////////////////////////////////////////////////////////////////////
//////////////////////////  FUNCTION IMPLEMENTATIONS  //////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(void) {

    printf("Welcome to Frogger Game!\n");
    struct board_tile game_board[SIZE][SIZE];

    // TODO (Stage 1.1) Initialise the gameboard.
    init_board(game_board);

    // Read user input and place turtles.
    int num_turtle;
    printf("How many turtles? ");
    // TODO (Stage 1.2): Scan in the turtles, and place them on the map.
    scanf("%d", &num_turtle);
    int turtle_x, turtle_y;
    if(num_turtle > 0) printf ("Enter pairs:\n");

    for(int i=0; i<num_turtle; i++)
    {
        scanf ("%d %d", &turtle_x, &turtle_y);
        if ((turtle_x>0) && (turtle_x<8) && (turtle_y>0) &&
        (turtle_y<=8) && (!game_board[turtle_x][turtle_y].occupied))
        game_board[turtle_x][turtle_y].type = TURTLE;
    }

    // Start the game and print out the gameboard.
    printf("Game Started\n");
    print_board(game_board);

    char command;
    printf("Enter command: ");
    // TODO (Stage 1.3): Create a command loop, to read and execute commands!
    while (scanf("%c\n", &command) != EOF)
    {
        /*COMMANDS WILL BE EXECUTED HERE*/
        if (command != NULL)
        {
            print_board(game_board);
            printf("Enter Command: ");
        }
    }


    printf("Thank you for playing Frogger Game!\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// ADDITIONAL FUNCTIONS /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// TODO: Add more functions here!


////////////////////////////////////////////////////////////////////////////////
////////////////////////////// PROVIDED FUNCTIONS //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void print_board(struct board_tile board[SIZE][SIZE]) {
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            char type_char = '\0';
            if (board[row][col].occupied) {
                type_char = 'F';
            } else {
                type_char = type_to_char(board[row][col].type);
            }
            printf("%c ", type_char);
        }
        printf("\n");
    }
}

char type_to_char(enum tile_type type) {
    char type_char = ' ';
    if (type == LILLYPAD) {
        type_char = 'o';
    } else if (type == BANK) {
        type_char = 'x';
    } else if (type == WATER) {
        type_char = '~';
    } else if (type == TURTLE) {
        type_char = 'T';
    } else if (type == LOG) {
        type_char = 'L';
    }
    return type_char;
}
