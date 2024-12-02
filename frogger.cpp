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
//    4) November 15, 2024 (Phase 3.3)
//    5) November 25, 2024 (Added comments)
//    6) November 29, 2024 (Completed FTXUI implementation)
//
// This program is a simple Frogger game made to fulfill the Programming
// Fundamentals final project. The player controls the frog to reach the
// lilypads on the other side of the river by avoiding the water and bugs.
// The player can move the frog using the 'w', 's', 'a', and 'd' keys.
// The player can also place turtles, logs, and bugs on the board.
// The player can also clear a row, remove a log, and add a bug to the board.
// The player can quit the game by pressing 'q'.

#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/mouse.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/canvas.hpp"
#include "ftxui/screen/color.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/pixel.hpp"

using namespace std;
using namespace ftxui;

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  CONSTANTS  /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Provided constants
#define SIZE 15
#define TRUE 1
#define FALSE 0
#define XSTART SIZE - 1
#define YSTART SIZE / 2
#define LIVES 3

// Additional constants
#define TILE board[row][col]
#define TPS 20            // 20 ticks per second
#define MOVE_COOLDOWN 200 // 200 milliseconds delay
#define BUGS_MOVE_TICKS 5 // 5 ticks per bug movement

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

// Direction enum for the frogger and bugs.
enum direction
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    STAY
};

// Game state enum for the game loop.
enum game_state
{
    START,
    GAME,
    SETUP,
    WIN,
    LOSE,
    QUIT
};

// Game event enum for the game loop.
enum game_event
{
    NO_EVENT,
    MOVED,
    REACHED_LILLYPAD,
    ON_WATER,
    HIT_BY_BUG,
    NO_LIVES
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////  STRUCTS  //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Additional structs

// Bug struct to represent a bug on the board.

struct frog_data
{
    int x;
    int y;
    int lives;
    chrono::steady_clock::time_point last_move_time;
};

struct bug
{
    bool present;             // TRUE or FALSE based on if a bug is there.
    enum direction direction; // The direction the bug is moving.
};

struct Command
{
    char setup_char;
    string command_name;
    function<void(struct board_tile[SIZE][SIZE], int, int, int)> command_function;
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

void init_board(struct board_tile board[SIZE][SIZE]);
game_event check_state(struct board_tile board[SIZE][SIZE], frog_data &, enum game_state &);

mutex game_mutex;
void game_update_thread(struct board_tile board[SIZE][SIZE], frog_data &,
                        game_state &, game_event &, ScreenInteractive &, Element[2]);

void add_turtle(struct board_tile board[SIZE][SIZE], int, int);
void add_log(struct board_tile board[SIZE][SIZE], int, int, int);
void clear_row(struct board_tile board[SIZE][SIZE], int);
void remove_log(struct board_tile board[SIZE][SIZE], int, int);
void move_frogger(struct board_tile board[SIZE][SIZE], frog_data &, enum direction);
void add_bug(struct board_tile board[SIZE][SIZE], int, int);
void remove_bug(struct board_tile board[SIZE][SIZE], int, int);
void move_bugs(struct board_tile board[SIZE][SIZE]);
void add_bank(struct board_tile board[SIZE][SIZE], int);

// Prints out the current state of the board.
Element print_board(struct board_tile board[SIZE][SIZE]);
Pixel type_to_pixel(enum tile_type type);

// FTXUI component functions
Component create_board_canvas(struct board_tile game_board[SIZE][SIZE], frog_data &,
                              game_state &state, game_event &event, Element message[2],
                              int &current_tab);
Component create_game_sidebar(int &lives);
Component create_setup_sidebar(struct board_tile game_board[SIZE][SIZE], vector<Command> &commands,
                               int &setup_selected, int &setup_cursor, string &setup_command,
                               Element message[2], int &current_tab);
Component create_game_container(Component &board_canvas, Component &sidebar);
Component create_message_bar(Element message[2]);
Component create_keypress_box(string &key_pressed);

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////  GLOBAL VARIABLES  //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

vector<pair<int, int>> bugs_positions; // Vector to store bug positions
int bugs_move_counter = 0;             // Counter to track bug movement

////////////////////////////////////////////////////////////////////////////////
//////////////////////////  FUNCTION IMPLEMENTATIONS  //////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(void)
{
    // Initialize game variables
    struct board_tile game_board[SIZE][SIZE];
    init_board(game_board);

    game_state state = GAME;
    game_event current_event = NO_EVENT;
    frog_data frog = {XSTART, YSTART, LIVES, chrono::steady_clock::now()};
    string key_pressed = " ";
    Element message[2] = {text(""), text("")};
    int setup_selected = 0, setup_cursor = 0;
    string setup_command = "";
    int current_tab = 0;

    // Initialize FTXUI components
    ScreenInteractive screen = ScreenInteractive::Fullscreen();

    Component game_sidebar = create_game_sidebar(frog.lives);

    vector<Command> commands = {
        {'t', "Add Turtle", [](struct board_tile board[SIZE][SIZE], int x, int y, int)
         { add_turtle(board, x, y); }},
        {'l', "Add Log", [](struct board_tile board[SIZE][SIZE], int x, int y_start, int y_end)
         { add_log(board, x, y_start, y_end); }},
        {'c', "Clear Row", [](struct board_tile board[SIZE][SIZE], int x, int, int)
         { clear_row(board, x); }},
        {'r', "Remove Log", [](struct board_tile board[SIZE][SIZE], int x, int y, int)
         { remove_log(board, x, y); }},
        {'b', "Add Bug", [](struct board_tile board[SIZE][SIZE], int x, int y, int)
         { add_bug(board, x, y); }},
        {'B', "Remove Bug", [](struct board_tile board[SIZE][SIZE], int x, int y, int)
         { remove_bug(board, x, y); }},
        {'k', "Add Bank", [](struct board_tile board[SIZE][SIZE], int x, int, int)
         { add_bank(board, x); }},
        {'o', "Initialize Board", [](struct board_tile board[SIZE][SIZE], int, int, int)
         { init_board(board); }},
        {'q', "Quit Setup", [](struct board_tile[SIZE][SIZE], int, int, int) { /* No action. */ }}};

    Component setup_sidebar =
        create_setup_sidebar(game_board, commands, setup_selected,
                             setup_cursor, setup_command, message, current_tab);
    Component sidebar = Container::Tab(
        {
            game_sidebar,
            setup_sidebar,
        },
        &current_tab);
    Component board_canvas =
        create_board_canvas(game_board, frog,
                            state, current_event, message, current_tab);
    Component game_container = create_game_container(board_canvas, sidebar);
    Component message_bar = create_message_bar(message);
    Component key_pressed_box = create_keypress_box(key_pressed);

    Component game_renderer =
        Renderer(game_container, [&]
                 { return vbox(
                       {
                           text("Frogger Game") | hcenter,
                           separator(),
                           filler(),
                           game_container->Render() |
                               center,
                           filler(),
                           hbox(
                               {key_pressed_box->Render(),
                                message_bar->Render() | xflex_grow}),
                       }); });

    // Initialize main renderer with event handling
    Component main_renderer =
        game_renderer | CatchEvent(
                            [&](Event event)
                            {
                                if (event.is_mouse())
                                {
                                    return true;
                                }
                                if (event.is_character())
                                {
                                    key_pressed = event.character();
                                }
                                if (event == Event::CtrlQ)
                                {
                                    message[0] = text("Thank you for playing Frogger Game!");
                                    screen.ExitLoopClosure()();
                                    return true;
                                }

                                if (event == Event::Character('o'))
                                {
                                    current_tab = 1 - current_tab;
                                    if (current_tab == 1)
                                        state = SETUP;
                                    else
                                        state = GAME;
                                    return true;
                                }
                                return false;
                            });

    // Create and start update thread
    thread update_thread(game_update_thread, game_board, ref(frog), ref(state),
                         ref(current_event), ref(screen), message);

    screen.Loop(main_renderer);

    // Cleanup thread
    state = QUIT;
    update_thread.join();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// THREAD FUNCTIONS ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * Function: game_update_thread
 * ---------------------
 * Updates the game state and checks for game events.
 * Updates the message based on the game event.
 * Triggers screen refresh if the game state is GAME.
 * Sleeps for the remainder of the tick duration.
 *
 * board: The 2D array representing the board.
 * frog: The frogger data struct.
 * state: The current state of the game.
 * current_event: The current event of the game.
 * screen: The interactive screen.
 * message: The message to be displayed.
 */
void game_update_thread(struct board_tile board[SIZE][SIZE], frog_data &frog,
                        game_state &state, game_event &current_event,
                        ScreenInteractive &screen, Element message[2])
{
    using namespace chrono;
    milliseconds tick_duration = milliseconds(1000 / TPS);

    while (state != QUIT)
    {
        time_point start_time = steady_clock::now();

        if (state == GAME)
        {
            lock_guard<mutex> lock(game_mutex);

            // Update game state
            move_bugs(board);
            current_event = check_state(board, frog, state);

            // Update message based on event
            switch (current_event)
            {
            case REACHED_LILLYPAD:
                message[0] = text("You won!");
                break;
            case ON_WATER:
                message[0] = text("You fell in the water!") | color(Color::Red);
                break;
            case HIT_BY_BUG:
                message[0] = text("You were hit by a bug!") | color(Color::Orange1);
                break;
            case NO_LIVES:
                message[0] = text("You ran out of lives!") | color(Color::Red);
                break;
            default:
                break;
            }
        }

        // Trigger screen refresh
        if (state == GAME)
            screen.PostEvent(Event::Custom);

        // Sleep for remainder of tick
        time_point end_time = steady_clock::now();
        milliseconds elapsed = duration_cast<milliseconds>(end_time - start_time);
        if (elapsed < tick_duration)
        {
            this_thread::sleep_for(tick_duration - elapsed);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// ///FTXUI FUNCTIONS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Component create_board_canvas(struct board_tile game_board[SIZE][SIZE], frog_data &frog,
                              game_state &state, game_event &current_event,
                              Element message[2], int &current_tab)
{
    Component board_canvas = Renderer([&, game_board]
                                      { 
                                          lock_guard<mutex> lock(game_mutex);
                                          return print_board(game_board); });
    board_canvas |=
        CatchEvent(
            [&, game_board, message](Event event)
            {
                if (current_tab == 1)
                    return false;

                direction move_direction = STAY;
                if (event == Event::Character('w'))
                    move_direction = FORWARD;
                else if (event == Event::Character('s'))
                    move_direction = BACKWARD;
                else if (event == Event::Character('a'))
                    move_direction = LEFT;
                else if (event == Event::Character('d'))
                    move_direction = RIGHT;

                if (move_direction != STAY)
                {
                    move_frogger(game_board, frog, move_direction);
                }
                return false;
            });
    return board_canvas;
}

Component create_game_sidebar(int &lives)
{
    Component game_sidebar =
        Renderer([&]
                 { return vbox(
                       {filler(),
                        text("Lives:"),
                        hbox([&]
                             { Elements hearts;
                                 for (int i = 0; i < lives; i++)
                                     hearts.push_back(text("💗"));
                                 return hearts; }())}); });
    return game_sidebar;
}

Component create_setup_sidebar(struct board_tile game_board[SIZE][SIZE], vector<Command> &commands,
                               int &setup_selected, int &setup_cursor, string &setup_command,
                               Element message[2], int &current_tab)
{
    InputOption setup_input_option = InputOption::Default();
    setup_input_option.cursor_position = &setup_cursor;
    Component setup_input = Input(&setup_command, "Input command", setup_input_option);
    Component setup_menu = Container::Vertical(
        [&]
        {
            Components entries;
            for (Command &command : commands)
                entries.push_back(MenuEntry(command.command_name));
            return entries;
        }(),
        &setup_selected);

    setup_input |= CatchEvent(
        [&, game_board, setup_menu, message](Event event)
        {
            if (event == Event::Return)
            {
                if (setup_command.empty())
                    return false;
                int command_args[3] = {0, 0, 0};
                char command = setup_command[0];

                int i = 0;
                for (int j = 2; j < setup_command.size(); j++)
                {
                    if (setup_command[j] == ' ')
                    {
                        i++;
                        continue;
                    }
                    command_args[i] = command_args[i] * 10 + (setup_command[j] - '0');
                }

                auto it = find_if(commands.begin(), commands.end(), [&](const Command &cmd)
                                  { return cmd.setup_char == command; });
                if (it != commands.end())
                {
                    if (command == 'q')
                    {
                        current_tab = 0;
                        setup_command = "";
                        return true;
                    }
                    it->command_function(game_board, command_args[0], command_args[1], command_args[2]);
                }
                else
                {
                    message[0] = text("Invalid command.") | color(Color::Red);
                }
                // TODO: Add error message if command did not execute.
                message[0] = text("Command executed.") | color(Color::Green);
                setup_command = "";
                setup_menu->TakeFocus();
                return true;
            }
            return false;
        });

    setup_menu |=
        CatchEvent(
            [&, setup_input](Event event)
            {
                if (event.is_character())
                {
                    setup_input->TakeFocus();
                    setup_command = event.character();
                    setup_cursor = setup_command.size();
                    return true;
                }
                if (event == Event::Return)
                {
                    setup_command = commands[setup_selected].setup_char;
                    setup_command += ' ';
                    setup_input->TakeFocus();
                    setup_cursor = setup_command.size();
                    return true;
                }
                return false;
            });

    Component setup_sidebar =
        Renderer(Container::Vertical({setup_menu, setup_input}), [&, setup_menu, setup_input]
                 { return vbox({hbox(text("Selected = "), text(string(1, commands[setup_selected].setup_char))),
                                separator(),
                                setup_menu->Render() | frame,
                                filler(),
                                setup_input->Render()}); });
    return setup_sidebar;
}

Component create_game_container(Component &board_canvas, Component &sidebar)
{
    Component game_container =
        Renderer(Container::Stacked({sidebar, board_canvas}),
                 [&, board_canvas, sidebar]
                 { return hbox({board_canvas->Render() | center | size(WIDTH, EQUAL, 36) | size(HEIGHT, EQUAL, 17) | border,
                                sidebar->Render() | border | size(WIDTH, GREATER_THAN, 20)}); });
    return game_container;
}

Component create_message_bar(Element message[2])
{
    Component message_bar =
        Renderer([&, message]
                 { return hbox({message[0],
                                message[1]}) |
                          size(HEIGHT, EQUAL, 1) | xflex_shrink | border; });
    return message_bar;
}

Component create_keypress_box(string &key_pressed)
{
    Component keypress_box =
        Renderer([&]
                 { return text(key_pressed) | center | bold | size(HEIGHT, EQUAL, 1) | size(WIDTH, EQUAL, 3) | border; });
    return keypress_box;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// ADDITIONAL FUNCTIONS /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * Function: init_board
 * ---------------------
 * Initializes the board with the initial values.
 * Sets the type of the tiles based on the row and column.
 *
 * board: The 2D array representing the board.
 */
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
                // Set the first row with even columns as lillypads.
                if (col % 2 == 0)
                    board[row][col].type = LILLYPAD;
                else
                    board[row][col].type = WATER;
            }
            else if (row == SIZE - 1)
            {
                // Set the last row as the bank.
                board[row][col].type = BANK;

                // Set the middle column of the bank as frogger.
                if (col == SIZE / 2)
                    board[row][col].occupied = TRUE;
            }
            else
                board[row][col].type = WATER;
        }
    }
}

/*
 * Function: check_state
 * ---------------------
 * Checks the state of the game based on the frogger's position.
 * Updates the game state and returns the game event.
 *
 * board: The 2D array representing the board.
 * x_frog: The x-coordinate of the frogger.
 * y_frog: The y-coordinate of the frogger.
 * lives: The number of lives the player has.
 * state: The current state of the game.
 */
game_event check_state(board_tile board[SIZE][SIZE], frog_data &frog, game_state &state)
{
    board_tile &current_tile = board[frog.x][frog.y];
    if (current_tile.type == LILLYPAD)
    {
        state = WIN;
        return REACHED_LILLYPAD;
    }
    else if ((current_tile.type == WATER) ||
             (current_tile.bug.present))
    {
        frog.lives--;
        if (frog.lives <= 0)
        {
            state = LOSE;
            return NO_LIVES;
        }
        else
        {
            game_event current_event = MOVED;
            if (current_tile.bug.present)
                current_event = HIT_BY_BUG;
            if (current_tile.type == WATER)
                current_event = ON_WATER;
            current_tile.occupied = FALSE;
            frog.x = XSTART;
            frog.y = YSTART;
            board[frog.x][frog.y].occupied = TRUE;
            return current_event;
        }
    }
    return MOVED;
}

/*
 * Function: add_turtle
 * ---------------------
 * Adds a turtle to the board at the given position.
 *
 * board: The 2D array representing the board.
 * x: The x-coordinate of the turtle.
 * y: The y-coordinate of the turtle.
 */
void add_turtle(struct board_tile board[SIZE][SIZE], int x, int y)
{
    // Turtle will not be added if the tile is not water.
    if (board[x][y].occupied || x < 1 || x >= SIZE - 1 || y < 0 || y >= SIZE)
        return;
    if (board[x][y].type == WATER)
        board[x][y].type = TURTLE;
}

/*
 * Function: add_log
 * ---------------------
 * Adds a log to the board at the given position.
 *
 * board: The 2D array representing the board.
 * x: The x-coordinate of the log.
 * y_start: The starting y-coordinate of the log.
 * y_end: The ending y-coordinate of the log.
 */
void add_log(struct board_tile board[SIZE][SIZE], int x, int y_start, int y_end)
{
    if (x < 1 || x >= SIZE - 1)
        return;

    // Log will not be added if there is a turtle in the row.
    for (int i = 0; i < SIZE; i++)
        if (board[x][i].type == TURTLE)
            return;

    // If y_start or y_end is out of bounds, it will be set to the edge of the board.
    if (y_start < 0)
        y_start = 0;
    if (y_end > SIZE - 1)
        y_end = SIZE - 1;
    for (int i = y_start; i <= y_end; i++)
        board[x][i].type = LOG;
}

/*
 * Function: clear_row
 * ---------------------
 * Clears a row on the board.
 *
 * board: The 2D array representing the board.
 * x: The row to be cleared.
 */
void clear_row(struct board_tile board[SIZE][SIZE], int x)
{
    if (x < 1 || x >= SIZE - 2)
        return;

    // Row will not be cleared if there is an occupied tile in the row.
    for (int i = 0; i < SIZE; i++)
        if (board[x][i].occupied)
            return;
    for (int i = 0; i < SIZE; i++)
    {
        board[x][i].type = WATER;
        remove_bug(board, x, i);
    }
}

/*
 * Function: remove_log
 * ---------------------
 * Removes a log from the board at the given position.
 *
 * board: The 2D array representing the board.
 * x: The x-coordinate of the log.
 * y: The y-coordinate of the log.
 */
void remove_log(struct board_tile board[SIZE][SIZE], int x, int y)
{
    if (x < 1 || x >= SIZE - 1)
        return;
    if (board[x][y].type != LOG)
        return;
    else
    {
        board[x][y].type = WATER;
        remove_bug(board, x, y);
    }

    // If the row is occupied, the log will not be removed.
    for (int i = 0; i < SIZE; i++)
        if (board[x][i].occupied)
            return;

    // Removes connected logs in the row.
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

/*
 * Function: move_frogger
 * ---------------------
 * Moves the frogger on the board based on the given direction.
 *
 * board: The 2D array representing the board.
 * x: The x-coordinate of the frogger.
 * y: The y-coordinate of the frogger.
 * move_direction: The direction the frogger will move.
 */
void move_frogger(struct board_tile board[SIZE][SIZE], frog_data &frog, direction move_direction)
{
    if (move_direction == STAY)
        return;

    auto current_time = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(
        current_time - frog.last_move_time);
    if (elapsed.count() < MOVE_COOLDOWN)
        return;

    lock_guard<mutex> lock(game_mutex);
    frog.last_move_time = current_time;

    int new_x = frog.x, new_y = frog.y;
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

    // Frogger will not move if the new position is out of bounds.
    if (new_x < 0 || new_x >= SIZE || new_y < 0 || new_y >= SIZE)
        return;

    board[frog.x][frog.y].occupied = FALSE;
    board[new_x][new_y].occupied = TRUE;
    frog.x = new_x;
    frog.y = new_y;
}

/*
 * Function: add_bug
 * ---------------------
 * Adds a bug to the board at the given position.
 *
 * board: The 2D array representing the board.
 * x: The x-coordinate of the bug.
 * y: The y-coordinate of the bug.
 */
void add_bug(struct board_tile board[SIZE][SIZE], int x, int y)
{
    if (x < 1 || x >= SIZE - 1 || y < 0 || y >= SIZE)
        return;

    // Bug will only be added if the tile is log or turtle.
    if (board[x][y].type == LOG || board[x][y].type == TURTLE)
    {
        board[x][y].bug.present = TRUE;
        board[x][y].bug.direction = RIGHT;
        bugs_positions.push_back({x, y});
    }
}

/*
 * Function: remove_bug
 * ---------------------
 * Removes a bug from the board at the given position.
 *
 * board: The 2D array representing the board.
 * x: The x-coordinate of the bug.
 * y: The y-coordinate of the bug.
 */
void remove_bug(struct board_tile board[SIZE][SIZE], int x, int y)
{
    if (x < 1 || x >= SIZE - 1 || y < 0 || y >= SIZE)
        return;

    if (board[x][y].bug.present)
    {
        board[x][y].bug.present = FALSE;
        board[x][y].bug.direction = RIGHT;
        bugs_positions.erase(remove(bugs_positions.begin(), bugs_positions.end(), make_pair(x, y)), bugs_positions.end());
    }
}

/*
 * Function: move_bugs
 * ---------------------
 * Moves the bugs on the board based on the direction.
 *
 * board: The 2D array representing the board.
 */
void move_bugs(struct board_tile board[SIZE][SIZE])
{
    if (bugs_move_counter < BUGS_MOVE_TICKS)
    {
        bugs_move_counter++;
        return;
    }
    bugs_move_counter = 0;

    vector<pair<int, int>> new_bugs_positions; // Vector to store new bug positions

    // Sort bugs_positions to ensure bugs are moved from top to bottom, left to right
    sort(bugs_positions.begin(), bugs_positions.end(),
         [](pair<int, int> &p1, pair<int, int> &p2)
         { return tie(p1.second, p1.first) < tie(p2.second, p2.first); });

    for (auto &[row, col] : bugs_positions)
    {
        if (!board[row][col].bug.present)
            continue;

        // Retrieve the direction of the bug and the next column.
        direction dir = board[row][col].bug.direction;
        int next_col = dir == RIGHT ? col + 1 : col - 1;

        // If the next column is out of bounds, the bug will change direction.
        if (next_col < 0 || next_col >= SIZE)
        {
            dir = dir == RIGHT ? LEFT : RIGHT;
            next_col = dir == RIGHT ? col + 1 : col - 1;
            // If the next column is occupied by a bug, the bug will not move.
            if (board[row][next_col].bug.present)
            {
                new_bugs_positions.push_back({row, col});
                continue;
            }
        }

        tile_type current_type = board[row][col].type;
        tile_type next_type = board[row][next_col].type;

        // If the next tile is not log or turtle, the bug will change direction.
        if (!(next_type == LOG || next_type == TURTLE) || board[row][next_col].bug.present)
        {
            dir = dir == RIGHT ? LEFT : RIGHT;
            next_col = dir == RIGHT ? col + 1 : col - 1;
            next_type = board[row][next_col].type;

            // If the next tile is also out of bounds, the bug will not move.
            if (!(next_type == LOG || next_type == TURTLE) || board[row][next_col].bug.present)
            {
                new_bugs_positions.push_back({row, col});
                continue;
            }
        }

        // Move the bug to the next tile.
        if (current_type == LOG || current_type == TURTLE)
        {
            board[row][col].bug.present = FALSE;
            board[row][next_col].bug.present = TRUE;
            board[row][next_col].bug.direction = dir;
            new_bugs_positions.push_back({row, next_col}); // Add new bug position to vector
        }
    }

    bugs_positions = new_bugs_positions; // Update bugs_positions with new positions
}

/*
 * Function: add_bank
 * ---------------------
 * Adds a bank to the board at the given position.
 *
 * board: The 2D array representing the board.
 * x: The x-coordinate of the bank.
 */
void add_bank(struct board_tile board[SIZE][SIZE], int x)
{
    if (x < 1 || x >= SIZE - 1)
        return;

    // Bank will not be added if there is a turtle or log in the row.
    for (int i = 0; i < SIZE; i++)
        if (board[x][i].type == TURTLE || board[x][i].type == LOG)
            return;
    for (int i = 0; i < SIZE; i++)
        board[x][i].type = BANK;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// PROVIDED FUNCTIONS //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * Function: print_board
 * ---------------------
 * Returns a canvas with the current state of the board.
 *
 * board: The 2D array representing the board.
 */
Element print_board(struct board_tile board[SIZE][SIZE])
{
    auto c = Canvas(SIZE * 4, SIZE * 4);
    for (int row = 0; row < SIZE; row++)
    {
        for (int col = 0; col < SIZE; col++)
        {
            // Prioritizes printing frogger then bug after other tile types.
            Pixel type_pixel;
            if (board[row][col].occupied)
            {
                type_pixel.background_color = Color::Green;
                type_pixel.character = "🐸";
            }
            else if (board[row][col].bug.present)
            {
                type_pixel = type_to_pixel(board[row][col].type);
                type_pixel.foreground_color = Color::Red;
                type_pixel.character = "🐞";
            }
            else
            {
                type_pixel = type_to_pixel(board[row][col].type);
            }
            c.DrawPixel(col * 4, row * 4, type_pixel);
        }
    }
    return canvas(move(c));
}

/*
 * Function: type_to_pixel
 * ---------------------
 * Converts the tile type to a canvas pixel.
 *
 * type: The type of the tile.
 *
 * Returns: The pixel representation of the tile type.
 */
Pixel type_to_pixel(enum tile_type type)
{
    Pixel p;
    p.bold = TRUE;

    // Pixel must be 2 characters long
    switch (type)
    {
    case LILLYPAD:
        p.foreground_color = Color::Green;
        p.background_color = Color::DarkGreen;
        p.character = "ↈ ";
        break;
    case BANK:
        p.foreground_color = Color::Yellow;
        p.background_color = Color::Yellow;
        p.character = "XX";
        break;
    case WATER:
        p.foreground_color = Color::Blue;
        p.background_color = Color::DarkBlue;
        p.bold = FALSE;
        p.character = "\u259A\u259A";
        break;
    case TURTLE:
        p.foreground_color = Color::Green;
        p.background_color = Color::Green;
        p.character = "🐢";
        break;
    case LOG:
        p.foreground_color = Color::DarkOrange;
        p.background_color = Color::Orange3;
        p.character = "LL";
        break;
    default:
        p.foreground_color = Color::White;
        p.foreground_color = Color::White;
        p.character = "  ";
    }
    return p;
}