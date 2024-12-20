// Jumping Frogger
//
// This program was written by Group 5
// Christopher Marchel Mark Gijoh (01082240011)
// Farrel Fyelo Hollans Tamaela   (01082240024)
// Gian Jeconiah Sitompul         (01082240025)
// Kenny Lay                      (01082240010)
// on 1)  October 25, 2024  (Starter code)
//    2)  October 31, 2024  (Phase 1.3)
//    3)  November 7, 2024  (Phase 2.3)
//    4)  November 15, 2024 (Phase 3.3)
//    5)  November 25, 2024 (Added comments)
//    6)  November 29, 2024 (Phase 4.1: Completed FTXUI implementation)
//    7)  December 2, 2024  (Phase 4.2: Implemented game loop and threading)
//    8)  December 4, 2024  (Phase 4.3: Added new game mechanics and commands)
//    9)  December 5, 2024  (Phase 4.4: Bug fixing and added comments)
//    10) December 6, 2024  (Finalized code and submitted)
//
// This program is a simple Frogger game made to fulfill the Programming
// Fundamentals final project. The player controls the frog to reach the
// lilypads on the other side of the river by avoiding the water and bugs.
// The player can move the frog using the 'w', 's', 'a', and 'd' keys.
// The player can also configure the board by pressing 'o' to access the
// setup menu. In the setup menu, the player can add turtles, logs, bugs,
// clear a row, remove a log, and add a bank to the board. The board can
// also be loaded from a file by typing 'O' followed by the filename.
// The player can quit the game by pressing Ctrl+Q.

#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
#include <sstream>

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
#define DOUBLE_JUMP 5      // 5 double jumps
#define TPS 20             // 20 ticks per second
#define MOVE_COOLDOWN 200  // 200 milliseconds delay
#define BUGS_MOVE_TICKS 3  // 3 ticks per bug movement
#define LOG_MOVE_TICKS 12  // Move every 10 game ticks
#define LOG_SPEED 1        // Move 1 cell at a time
#define LOG_TRAP_TICKS 120 // 120 ticks for trap log

// Provided Enums
enum tile_type
{
    LILLYPAD,
    BANK,
    WATER,
    TURTLE,
    LOG,
    TRAP_LOG
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

// Log direction enum for the log movement.
enum log_direction
{
    LOG_LEFT,
    LOG_RIGHT
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////  STRUCTS  //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Additional structs

// Frog data struct to represent the frog on the board.
struct frog_data
{
    int x;                                           // The x-coordinate of the frog.
    int y;                                           // The y-coordinate of the frog.
    int lives;                                       // The number of lives the player has.
    int double_jump;                                 // The number of double jumps the player has.
    int score;                                       // The current score of the player.
    int high_score;                                  // The highest score achieved by the player.
    chrono::steady_clock::time_point last_move_time; // The last time the frogger moved.
};

// Bug struct to represent a bug on the board.
struct bug
{
    bool present;             // TRUE or FALSE based on if a bug is there.
    enum direction direction; // The direction the bug is moving.
};

// Log data struct to represent a log on the board.
struct log_data
{
    int row;                 // The row the log is on.
    int start_col;           // The starting column of the log.
    int length;              // The length of the log.
    log_direction direction; // The direction the log is moving.
    int move_counter;        // The counter to track log movement.
    bool trap;               // The log is a trap.
    bool trap_triggered;     // The trap log has been triggered.
    int trap_decay;          // The decay counter for the trap log.
};

// Log tile data struct to store additioinal log tile properties on the board.
struct log_tile_data
{
    log_direction direction; // The direction the log is moving.
    bool trap;               // The log is a trap.
    bool trap_triggered;     // The trap log has been triggered.
    int trap_decay;          // The decay counter for the trap log.
};

struct Command
{
    char setup_char;      // The character to trigger the command.
    string command_name;  // The name of the command.
    string command_usage; // The usage and parameters of the command.
    function<void(struct board_tile[SIZE][SIZE], int, int, int)> command_function;
};

// Provided structs
struct board_tile
{
    enum tile_type type;      // The type of piece it is (water, bank, etc.)
    int occupied;             // TRUE or FALSE based on if Frogger is there.
    struct bug bug;           // The bug that is on the tile.
    struct log_tile_data log; // The log data on the tile.
};

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  FUNCTION PROTOTYPES  ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Game loop functions
void init_board(struct board_tile board[SIZE][SIZE]);
void restart_game(struct board_tile board[SIZE][SIZE], frog_data &, game_state &,
                  game_event &, Element[2]);
string load_file(string);
void load_board(struct board_tile board[SIZE][SIZE], frog_data &, string);
game_event check_state(struct board_tile board[SIZE][SIZE], frog_data &, enum game_state &);
void update_score(frog_data &, game_event);
void move_frogger(struct board_tile board[SIZE][SIZE], frog_data &, enum direction, bool);

// Game thread function
mutex game_mutex;
void game_update_thread(struct board_tile board[SIZE][SIZE], frog_data &,
                        game_state &, game_event &, ScreenInteractive &, Element[2]);

// Game setup functions
void add_turtle(struct board_tile board[SIZE][SIZE], int, int);
void add_log(struct board_tile board[SIZE][SIZE], int, int, int, bool);
void clear_row(struct board_tile board[SIZE][SIZE], int);
void remove_log(struct board_tile board[SIZE][SIZE], int, int);
void add_bug(struct board_tile board[SIZE][SIZE], int, int);
void remove_bug(struct board_tile board[SIZE][SIZE], int, int);
void move_bugs(struct board_tile board[SIZE][SIZE]);
void add_bank(struct board_tile board[SIZE][SIZE], int);
void update_logs(struct board_tile board[SIZE][SIZE], frog_data &);
void add_turtles_random(struct board_tile board[SIZE][SIZE], int, int);

// Board displaying functions
Element print_board(struct board_tile board[SIZE][SIZE]);
Pixel type_to_pixel(enum tile_type type);

// FTXUI component functions
Component create_board_canvas(struct board_tile game_board[SIZE][SIZE], frog_data &frog,
                              game_state &state, game_event &event, Element message[2],
                              int &current_tab);
Component create_game_sidebar(frog_data &frog);
Component create_setup_sidebar(struct board_tile game_board[SIZE][SIZE], frog_data &frog,
                               game_state &state, vector<Command> &commands, int &setup_selected,
                               int &setup_cursor, string &setup_command, Element message[2],
                               int &current_tab);
Component create_game_container(Component &board_canvas, Component &sidebar);
Component create_message_bar(Element message[2]);
Component create_keypress_box(string &key_pressed);

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////  GLOBAL VARIABLES  //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int game_tick = 0;                     // Counter to track game ticks
vector<pair<int, int>> bugs_positions; // Vector to store bug positions
int bugs_move_counter = 0;             // Counter to track bug movement
vector<log_data> logs;                 // Vector to store log data

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
    frog_data frog =
        {.x = XSTART, .y = YSTART, .lives = LIVES, .double_jump = DOUBLE_JUMP, .score = 0, .high_score = 0, .last_move_time = chrono::steady_clock::now()};
    string key_pressed = " ";
    Element message[2] = {text(""), text("")};
    int setup_selected = 0, setup_cursor = 0;
    string setup_command = "";
    int current_tab = 0;

    // Initialize FTXUI components
    ScreenInteractive screen = ScreenInteractive::Fullscreen();

    Component game_sidebar = create_game_sidebar(frog);

    vector<Command> commands = {
        {'t', "Add Turtle", "t <row> <col>", [](struct board_tile board[SIZE][SIZE], int x, int y, int)
         { add_turtle(board, x, y); }},
        {'T', "Add Turtles Randomly", "T <row> <num_turtles>", [](struct board_tile board[SIZE][SIZE], int x, int num_turtles, int)
         { add_turtles_random(board, x, num_turtles); }},
        {'l', "Add Log", "l <row> <col_start> <length>", [](struct board_tile board[SIZE][SIZE], int x, int y_start, int length)
         { add_log(board, x, y_start, length, FALSE); }},
        {'L', "Add Trap Log", "L <row> <col_start> <length>", [](struct board_tile board[SIZE][SIZE], int x, int y_start, int length)
         { add_log(board, x, y_start, length, TRUE); }},
        {'c', "Clear Row", "c <row>", [](struct board_tile board[SIZE][SIZE], int x, int, int)
         { clear_row(board, x); }},
        {'r', "Remove Log", "r <row> <col>", [](struct board_tile board[SIZE][SIZE], int x, int y, int)
         { remove_log(board, x, y); }},
        {'b', "Add Bug", "b <row> <col>", [](struct board_tile board[SIZE][SIZE], int x, int y, int)
         { add_bug(board, x, y); }},
        {'B', "Remove Bug", "B <row> <col>", [](struct board_tile board[SIZE][SIZE], int x, int y, int)
         { remove_bug(board, x, y); }},
        {'k', "Add Bank", "k <row>", [](struct board_tile board[SIZE][SIZE], int x, int, int)
         { add_bank(board, x); }},
        {'o', "Initialize Board", "Resets the board to initial state", [](struct board_tile board[SIZE][SIZE], int, int, int)
         { init_board(board); }},
        {'O', "Load Board", "O <filename>", [](struct board_tile board[SIZE][SIZE], int, int, int) { /* No action. */ }},
        {'q', "Quit Setup", "Return to game", [](struct board_tile[SIZE][SIZE], int, int, int) { /* No action. */ }},
        {'X', "Restart Game", "Reset the board and restart the game", [](struct board_tile[SIZE][SIZE], int, int, int) { /* No action. */ }}};

    Component setup_sidebar =
        create_setup_sidebar(game_board, frog, state, commands, setup_selected,
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
                           text("  JUMPING FROGGER  ") | color(Color::Honeydew2) | bgcolor(Color::DarkSeaGreen4Bis) | hcenter,
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
                                    message[0] = text("Thank you for playing Jumping Frogger!");
                                    screen.ExitLoopClosure()();
                                    return true;
                                }

                                if (current_tab == 0 && event == Event::Character('o'))
                                {
                                    current_tab = 1;
                                    state = SETUP;
                                    return true;
                                }

                                // Restart game when space is pressed if game is over
                                if (state == WIN || state == LOSE)
                                {
                                    if (event == Event::Character(' '))
                                    {
                                        restart_game(game_board, frog, state, current_event, message);
                                        return true;
                                    }
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
    frog_data prev_frog = frog;

    while (state != QUIT)
    {
        time_point start_time = steady_clock::now();

        if (state == GAME)
        {
            // Lock game mutex to prevent concurrent access
            lock_guard<mutex> lock(game_mutex);

            // Update game state
            update_logs(board, frog);
            move_bugs(board);
            current_event = check_state(board, frog, state);

            // Determine if the frogger moved from previous position
            if (current_event == NO_EVENT &&
                (frog.x != prev_frog.x || frog.y != prev_frog.y))
                current_event = MOVED;

            update_score(frog, current_event);

            // Update message based on event
            switch (current_event)
            {
            case REACHED_LILLYPAD:
                message[0] = text("You reached the lilypad!") | color(Color::Green);
                break;
            case ON_WATER:
                message[0] = text("You fell in the water!") | color(Color::DodgerBlue3);
                break;
            case HIT_BY_BUG:
                message[0] = text("You were hit by a bug!") | color(Color::Orange1);
                break;
            case NO_LIVES:
                message[0] = text("You ran out of lives!") | color(Color::Red);
                break;
            case MOVED:
                message[0] = text("You moved.");
                break;
            default:
                break;
            }

            // date previous frog position and increment game tick
            prev_frog = frog;
            game_tick++;
            if (game_tick == INT_MAX)
                game_tick = 0;

            // Trigger screen refresh
            screen.PostEvent(Event::Custom);
        }

        // Check for game over conditions and display message
        if (state == WIN)
        {
            this_thread::sleep_for(2s);
            message[0] = text("Congratulations! You won!") | color(Color::Gold1) | blink;
            screen.PostEvent(Event::Custom);
            this_thread::sleep_for(3s);
            message[0] = text("Press SPACE to restart.") | color(Color::Orange1) | blink;
            screen.PostEvent(Event::Custom);
        }
        else if (state == LOSE)
        {
            this_thread::sleep_for(2s);
            message[0] = text("Game over! You lost!") | color(Color::Red3) | blink;
            screen.PostEvent(Event::Custom);
            this_thread::sleep_for(3s);
            message[0] = text("Press SPACE to restart.") | color(Color::Orange1) | blink;
            screen.PostEvent(Event::Custom);
        }

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
//////////////////////////////// FTXUI FUNCTIONS ///////////////////////////////
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
                bool double_jump = FALSE;
                if (event == Event::Character('w'))
                    move_direction = FORWARD;
                else if (event == Event::Character('s'))
                    move_direction = BACKWARD;
                else if (event == Event::Character('a'))
                    move_direction = LEFT;
                else if (event == Event::Character('d'))
                    move_direction = RIGHT;
                else if (event == Event::Character('W'))
                {
                    move_direction = FORWARD;
                    double_jump = TRUE;
                }
                else if (event == Event::Character('S'))
                {
                    move_direction = BACKWARD;
                    double_jump = TRUE;
                }
                else if (event == Event::Character('A'))
                {
                    move_direction = LEFT;
                    double_jump = TRUE;
                }
                else if (event == Event::Character('D'))
                {
                    move_direction = RIGHT;
                    double_jump = TRUE;
                }

                if (state == GAME && move_direction != STAY)
                {
                    move_frogger(game_board, frog, move_direction, double_jump);
                }
                return false;
            });
    return board_canvas;
}

Component create_game_sidebar(frog_data &frog)
{
    Component game_sidebar =
        Renderer([&]
                 { return vbox(
                       {text("Score:"),
                        text(to_wstring(frog.score)) | bold | color(Color::Yellow),
                        text("High Score:"),
                        text(to_wstring(frog.high_score)) | bold | color(Color::Yellow),
                        filler(),
                        text("Lives:"),
                        hbox([&]
                             { Elements hearts;
                             int i = 0;
                                 for (i; i < frog.lives; i++)
                                     hearts.push_back(text("💗"));
                                 for (i; i < LIVES; i++)
                                     hearts.push_back(text("💔"));
                                 return hearts; }()),
                        text("Double Jump:"),
                        hbox([&]
                             { Elements jumps;
                                 for (int i = 0; i < frog.double_jump; i++)
                                     jumps.push_back(text("⏫"));
                                if (frog.double_jump == 0)
                                    jumps.push_back(text("❌"));
                                 return jumps; }())}); });
    return game_sidebar;
}

Component create_setup_sidebar(struct board_tile game_board[SIZE][SIZE], frog_data &frog, game_state &state,
                               vector<Command> &commands, int &setup_selected, int &setup_cursor,
                               string &setup_command, Element message[2], int &current_tab)
{
    InputOption setup_input_option = InputOption::Default();
    setup_input_option.cursor_position = &setup_cursor;
    Component setup_input = Input(&setup_command, "Input command", setup_input_option);
    Component setup_menu =
        Menu([&]
             {
                vector<string> entries;
            for (Command &command : commands)
                    entries.push_back(command.command_name);
                return entries; }(),
             &setup_selected, MenuOption());

    setup_input |= CatchEvent(
        [&, game_board, setup_menu, message](Event event)
        {
            if (event == Event::Return)
            {
                if (setup_command.empty())
                    return false;
                int command_args[3] = {-1, -1, -1};
                char command = setup_command[0];

                string temp;
                vector<string> args;
                stringstream s(setup_command);
                while (s >> temp)
                    args.push_back(temp);

                for (int i = 1; i < args.size(); i++)
                    command_args[i - 1] = stoi(args[i]);

                bool valid_command = FALSE;
                auto it = find_if(commands.begin(), commands.end(), [&](const Command &cmd)
                                  { return cmd.setup_char == command; });
                if (it != commands.end())
                {
                    if (command == 'q')
                    {
                        current_tab = 0;
                        state = GAME;
                        setup_command = "";
                        return true;
                    }
                    it->command_function(game_board, command_args[0], command_args[1], command_args[2]);
                    valid_command = TRUE;
                }

                if (command == 'O')
                {
                    string filename = "game_board.frogger";
                    if (setup_command.size() > 2)
                        filename = setup_command.substr(2);
                    string boardfile = load_file(filename);
                    if (boardfile == "")
                    {
                        message[0] = text("File not found.") | color(Color::Red);
                        setup_command = "";
                        setup_menu->TakeFocus();
                        return true;
                    }
                    load_board(game_board, frog, boardfile);
                }

                if (command == 'X')
                {
                    game_event event = NO_EVENT;
                    restart_game(game_board, frog, state, event, message);
                    current_tab = 0;
                    setup_command = "";
                    return true;
                }

                // TODO: Add error message if command did not execute.
                if (!valid_command)
                    message[0] = text("Invalid command.") | color(Color::Red);
                else
                    message[0] = text("Command executed.") | color(Color::Green);
                setup_command = "";
                setup_menu->TakeFocus();
                return true;
            }
            return false;
        });

    setup_menu |=
        CatchEvent(
            [&, setup_input, message](Event event)
            {
                if (event.is_character())
                {
                    setup_input->TakeFocus();
                    setup_command = event.character();
                    setup_cursor = setup_command.size();
                    message[0] = text("Enter your command.");
                    return true;
                }
                if (event == Event::Return)
                {
                    setup_command = commands[setup_selected].setup_char;
                    setup_command += ' ';
                    message[0] =
                        text("Usage: " + commands[setup_selected].command_usage);
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
    bugs_positions.clear();
    logs.clear();
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
 * Function: restart_game
 * ---------------------
 * Restarts the game by reinitializing the board and frogger.
 *
 * board: The 2D array representing the board.
 * frog: The frog data struct.
 * state: The current state of the game.
 * current_event: The current event of the game.
 * message: The message to be displayed.
 */
void restart_game(struct board_tile board[SIZE][SIZE], frog_data &frog, game_state &state,
                  game_event &current_event, Element message[2])
{
    init_board(board);
    game_tick = 0;
    bugs_move_counter = 0;
    state = GAME;
    current_event = NO_EVENT;
    int high_score = frog.high_score;
    frog =
        {.x = XSTART, .y = YSTART, .lives = LIVES, .double_jump = DOUBLE_JUMP, .score = 0, .high_score = high_score, .last_move_time = chrono::steady_clock::now()};
    message[0] = text("Game restarted!") | color(Color::Cyan) | blink;
}

/*
 * Function: load_board
 * ---------------------
 * Loads the board from a string.
 * The board is represented by the 2D array and the string.
 *
 * board: The 2D array representing the board.
 * board_string: The string representing the board.
 */
void load_board(struct board_tile board[SIZE][SIZE], frog_data &frog, string board_string)
{
    int row = 0, col = 0;
    init_board(board);
    for (int i = 0; i < board_string.size(); i++)
    {
        if (col >= SIZE)
        {
            // Move to the next row if the column exceeds the board size.
            if (board_string[i] == '\n')
            {
                row++;
                col = 0;
                continue;
            }
            else
                continue;
        }
        if (row >= SIZE)
            break;
        else
        {
            board[row][col].occupied = FALSE;
            switch (board_string[i])
            {
            case '~':
                board[row][col].type = WATER;
                break;
            case 'L':
                board[row][col].type = LOG;
                board[row][col].log.trap = FALSE;
                board[row][col].log.direction = LOG_LEFT;
                break;
            case 'R':
                board[row][col].type = LOG;
                board[row][col].log.trap = FALSE;
                board[row][col].log.direction = LOG_RIGHT;
                break;
            case 'l':
                board[row][col].type = TRAP_LOG;
                board[row][col].log.trap = TRUE;
                board[row][col].log.trap_triggered = FALSE;
                board[row][col].log.trap_decay = LOG_TRAP_TICKS;
                board[row][col].log.direction = LOG_LEFT;
                break;
            case 'r':
                board[row][col].type = TRAP_LOG;
                board[row][col].log.trap = TRUE;
                board[row][col].log.trap_triggered = FALSE;
                board[row][col].log.trap_decay = LOG_TRAP_TICKS;
                board[row][col].log.direction = LOG_RIGHT;
                break;
            case 'x':
                board[row][col].type = BANK;
                break;
            case 'T':
                board[row][col].type = TURTLE;
                break;
            case 'o':
                board[row][col].type = LILLYPAD;
                break;
            case 'F':
                board[row][col].type = BANK;
                board[row][col].occupied = TRUE;
                frog.x = row;
                frog.y = col;
                break;
            case 'B': // bug on log
                board[row][col].type = LOG;
                add_bug(board, row, col);
                break;
            case 'b': // bug on turtle
                board[row][col].type = TURTLE;
                add_bug(board, row, col);
                break;
            default:
                break;
            }
            col++;
        }
    }

    // Add logs to the logs vector
    for (int row = 0; row < SIZE; row++)
    {
        for (int col = 0; col < SIZE; col++)
        {
            if (board[row][col].type == LOG || board[row][col].type == TRAP_LOG)
            {
                bool found = FALSE;
                for (auto it = logs.begin(); it != logs.end(); ++it)
                {
                    if (it->row == row && it->start_col + it->length == col)
                    {
                        it->length++;
                        found = TRUE;
                        break;
                    }
                    else if (it->row == row && it->start_col - 1 == col)
                    {
                        it->start_col--;
                        it->length++;
                        found = TRUE;
                        break;
                    }
                }
                if (!found)
                {
                    log_data new_log = {
                        .row = row,
                        .start_col = col,
                        .length = 1,
                        .direction = board[row][col].log.direction,
                        .move_counter = game_tick % LOG_MOVE_TICKS,
                        .trap = board[row][col].log.trap,
                        .trap_triggered = FALSE,
                        .trap_decay = board[row][col].log.trap_decay};
                    logs.push_back(new_log);
                }
            }
        }
    }

    // Join logs if they are adjacent
    for (auto it = logs.begin(); it != logs.end();)
    {
        bool found = FALSE;
        for (auto it2 = logs.begin(); it2 != logs.end();)
        {
            if (it != it2 && it->row == it2->row && it->start_col + it->length == it2->start_col)
            {
                it->length += it2->length;
                it2 = logs.erase(it2);
                found = TRUE;
                break;
            }
            else
                ++it2;
        }
        if (!found)
            ++it;
    }
}

/*
 * Function: load_file
 * ---------------------
 * Loads the contents of a file into a string.
 *
 * filename: The name of the file to be loaded.
 */
string load_file(string filename)
{
    string str;
    ifstream file(filename);
    if (!file.is_open())
    {
        return "";
    }
    string line;
    while (getline(file, line))
        str += line + '\n';
    file.close();
    return str;
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
 * frog: The frog data struct.
 * state: The current state of the game.
 */
game_event check_state(board_tile board[SIZE][SIZE], frog_data &frog, game_state &state)
{
    board_tile &current_tile = board[frog.x][frog.y];
    // Sets game state to WIN if the frogger reaches the lilypad.
    if (current_tile.type == LILLYPAD)
    {
        state = WIN;
        return REACHED_LILLYPAD;
    }
    // Resets the frogger if it falls in the water or is hit by a bug.
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
    return NO_EVENT;
}

/*
 * Function: update_score
 * ---------------------
 * Updates the score based on the game event.
 *
 * frog: The frog data struct.
 * event: The game event.
 */
void update_score(frog_data &frog, game_event event)
{
    // Updates the score based on the game event.
    switch (event)
    {
    case MOVED:
        frog.score += 10;
        break;
    case ON_WATER:
        frog.score -= 15;
        break;
    case HIT_BY_BUG:
        frog.score -= 15;
        break;
    case NO_LIVES:
        frog.score -= 15;
        break;
    case REACHED_LILLYPAD:
        frog.score += 100;
        break;
    default:
        break;
    }
    // Ensures the score does not go below 0 and updates the high score.
    if (frog.score < 0)
        frog.score = 0;
    if (frog.score > frog.high_score)
        frog.high_score = frog.score;
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
void add_log(struct board_tile board[SIZE][SIZE], int x, int y_start, int length, bool is_trap)
{
    if (x < 1 || x >= SIZE - 1)
        return;

    tile_type log_type = is_trap ? TRAP_LOG : LOG;

    // Log will not be added if there is a turtle in the row.
    for (int i = 0; i < SIZE; i++)
        if (board[x][i].type == TURTLE)
            return;

    log_tile_data log_tile = {
        .direction = (x % 2 == 0) ? LOG_RIGHT : LOG_LEFT, // Alternate directions
        .trap = is_trap,                                  // Set trap log
        .trap_triggered = FALSE,                          // Set trap trigger
        .trap_decay = is_trap ? LOG_TRAP_TICKS : 0        // Set trap timer
    };

    // Check if another log is already present in the row, extend if it overlaps
    for (auto it = logs.begin(); it != logs.end(); ++it)
    {
        if (it->row == x)
        {
            if ((y_start >= it->start_col && y_start <= it->start_col + it->length) ||                   // New log start is within existing log
                (y_start + length >= it->start_col && y_start + length <= it->start_col + it->length) || // New log end is within existing log
                (y_start <= it->start_col && y_start + length >= it->start_col + it->length)             // New log overlaps existing log
            )
            {
                // Extend existing log
                int new_start_col = min(it->start_col, y_start);
                int new_end_col = max(it->start_col + it->length, y_start + length);
                it->start_col = new_start_col;
                it->length = new_end_col - new_start_col;

                // Add log to board
                for (int i = new_start_col; i < new_end_col && i < SIZE; i++)
                {
                    if (i >= 0 && i < SIZE)
                    {
                        board[x][i].type = log_type;
                        board[x][i].log = log_tile;
                    }
                }

                return;
            }
        }
    }

    // Add log to vector
    log_data new_log = {
        .row = x,
        .start_col = y_start,
        .length = length,
        .direction = (x % 2 == 0) ? LOG_RIGHT : LOG_LEFT, // Alternate directions
        .move_counter = game_tick % LOG_MOVE_TICKS,       // Sync log movement
        .trap = is_trap,                                  // Set trap log
        .trap_triggered = FALSE,                          // Set trap trigger
        .trap_decay = is_trap ? LOG_TRAP_TICKS : 0        // Set trap timer
    };
    logs.push_back(new_log);

    // Add log to board
    for (int i = y_start; i < y_start + length && i < SIZE; i++)
    {
        if (i >= 0 && i < SIZE)
        {
            board[x][i].type = log_type;
            board[x][i].log = log_tile;
        }
    }
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
    if (x < 1 || x >= SIZE - 1)
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
    while (i < SIZE && board[x][i].type == LOG)
    {
        board[x][i].type = WATER;
        remove_bug(board, x, i);
        i++;
    }
    while (j >= 0 && board[x][j].type == LOG)
    {
        board[x][j].type = WATER;
        remove_bug(board, x, j);
        j--;
    }

    // Remove log from logs vector
    auto it = logs.begin();
    while (it != logs.end())
    {
        if (it->row == x && y >= it->start_col && y < it->start_col + it->length)
            it = logs.erase(it);
        else
            ++it;
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
 * double_jump: A boolean indicating if the frogger will double jump.
 */
void move_frogger(struct board_tile board[SIZE][SIZE], frog_data &frog, direction move_direction, bool double_jump = FALSE)
{
    // Frogger will not move if the direction is STAY.
    if (move_direction == STAY)
        return;

    // Frogger will not double jump if the double jump is not available.
    if (double_jump && frog.double_jump <= 0)
        double_jump = FALSE;

    // Frogger will not move if the move cooldown has not elapsed.
    auto current_time = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(
        current_time - frog.last_move_time);
    if (elapsed.count() < MOVE_COOLDOWN)
        return;

    // Lock game mutex to prevent concurrent access
    lock_guard<mutex> lock(game_mutex);
    frog.last_move_time = current_time;

    int new_x = frog.x, new_y = frog.y;
    int move_distance = double_jump ? 2 : 1;
    switch (move_direction)
    {
    case FORWARD:
        new_x -= move_distance;
        break;
    case BACKWARD:
        new_x += move_distance;
        break;
    case LEFT:
        new_y -= move_distance;
        break;
    case RIGHT:
        new_y += move_distance;
        break;
    default:
        break;
    }

    // Frogger will not move if the new position is out of bounds.
    if (new_x < 0 || new_x >= SIZE || new_y < 0 || new_y >= SIZE)
        return;

    // Decrease double jump if it is used
    if (double_jump)
        frog.double_jump--;

    // Update frogger position on the board
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

    // Remove bug from the board and bugs_positions vector.
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
    // Bugs will only move every BUGS_MOVE_TICKS ticks.
    if (bugs_move_counter < BUGS_MOVE_TICKS)
    {
        bugs_move_counter++;
        return;
    }
    bugs_move_counter = 0;

    // Vector to store new bug positions
    vector<pair<int, int>> new_bugs_positions;

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

/*
 * Function: update_logs
 * ---------------------
 * Moves the position of the logs on the board.
 * Also moves the frog and bugs on the log.
 *
 * board: The 2D array representing the board.
 * frog: The frogger data struct.
 */

void update_logs(struct board_tile board[SIZE][SIZE], frog_data &frog)
{
    vector<log_data>::iterator it = logs.begin();
    while (it != logs.end() && !logs.empty())
    {
        if (++(it->move_counter) >= LOG_MOVE_TICKS)
        {
            // Reset move counter
            it->move_counter = 0;

            // Vector to store log tiles before moving
            vector<board_tile> log_tiles;
            tile_type log_type = it->trap ? TRAP_LOG : LOG;

            // Determine direction of log movement
            int move_dir = (it->direction == LOG_RIGHT) ? LOG_SPEED : -LOG_SPEED;

            // Move log tiles to queue and clear log tiles
            for (int i = it->start_col; i < it->start_col + it->length && i < SIZE; i++)
            {
                if (i >= 0 && i < SIZE)
                {
                    // Handle bugs on the edge of the log
                    if (board[it->row][i].bug.present)
                        if (i == SIZE - 1 && it->direction == LOG_RIGHT &&
                            log_tiles.size() > 0 &&
                            !log_tiles[log_tiles.size() - 1].bug.present &&
                            log_tiles[log_tiles.size() - 1].type == log_type)
                        {
                            log_tiles[log_tiles.size() - 1].bug =
                                {.present = TRUE,
                                 .direction = it->direction == LOG_RIGHT ? RIGHT : LEFT};
                            remove_bug(board, it->row, i);
                        }
                        else if (i == 0 && it->direction == LOG_LEFT &&
                                 !board[it->row][i - move_dir].bug.present &&
                                 board[it->row][i - move_dir].type == log_type)
                        {
                            board[it->row][i - move_dir].bug =
                                {.present = TRUE,
                                 .direction = it->direction == LOG_RIGHT ? RIGHT : LEFT};
                            remove_bug(board, it->row, i);
                        }
                    log_tiles.push_back(board[it->row][i]);
                    if (board[it->row][i].bug.present)
                        remove_bug(board, it->row, i);
                    board[it->row][i] =
                        {.type = WATER, .occupied = FALSE, .bug = {.present = FALSE, .direction = RIGHT}, .log = {.trap = FALSE}};
                }
                else if (i < 0)
                {
                    log_tiles.push_back(
                        board_tile{
                            .type = log_type,
                            .occupied = FALSE,
                            .bug = {.present = FALSE, .direction = RIGHT},
                            .log =
                                {.direction = it->direction, .trap = it->trap, .trap_decay = it->trap_decay}});
                }
            }

            // Erase if log is completely out of bounds
            it->start_col += move_dir;
            if (it->start_col + it->length < 0 || it->start_col >= SIZE)
            {
                it = logs.erase(it);
                continue;
            }

            // Add log tiles back to board
            for (int i = it->start_col; i < it->start_col + it->length && i < SIZE; i++)
            {
                if (i < 0 && !log_tiles.empty())
                {
                    log_tiles.erase(log_tiles.begin());
                    continue;
                }
                else if (i >= 0 && !log_tiles.empty())
                {
                    board[it->row][i] = log_tiles.front();
                    if (board[it->row][i].bug.present)
                        bugs_positions.push_back({it->row, i});
                }
                if (!log_tiles.empty())
                    log_tiles.erase(log_tiles.begin());
                else
                {
                    board[it->row][i].type = log_type;
                    board[it->row][i].log =
                        {.direction = it->direction, .trap = it->trap, .trap_decay = it->trap_decay};
                }
            }

            // Determine if frogger is on the log, then move frogger
            if (frog.x == it->row)
            {
                if (frog.y >= it->start_col && frog.y < it->start_col + it->length)
                {
                    frog.y += move_dir;
                    if (frog.y < 0 || frog.y >= SIZE)
                    {
                        frog.y -= move_dir;
                        board[frog.x][frog.y].occupied = TRUE;
                    }
                }
            }
        }
        if (!logs.empty() && it->trap)
        {
            // Trigger trap if frog is on the log
            for (int i = it->start_col; !it->trap_triggered && i < it->start_col + it->length && i < SIZE; i++)
            {
                if (i >= 0 && i < SIZE && board[it->row][i].occupied)
                    it->trap_triggered = TRUE;
            }

            // Do not start decay until trap is triggered
            if (!it->trap_triggered)
            {
                ++it;
                continue;
            }

            // Remove log if trap fully decayed
            if (--(it->trap_decay) <= 0)
            {
                for (int i = it->start_col; i < it->start_col + it->length && i < SIZE; i++)
                {
                    if (i >= 0 && i < SIZE)
                    {
                        board[it->row][i].type = WATER;
                        remove_bug(board, it->row, i);
                    }
                }
                it = logs.erase(it);
                continue;
            }
            else
            {
                // Update trap decay on board tiles
                for (int i = it->start_col; i < it->start_col + it->length && i < SIZE; i++)
                {
                    if (i >= 0 && i < SIZE)
                        board[it->row][i].log.trap_decay = it->trap_decay;
                }
            }
        }
        ++it;
    }
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
            else if (board[row][col].type == TRAP_LOG)
            {
                // Blend trap log color based on decay
                type_pixel = type_to_pixel(TRAP_LOG);
                int log_alpha = 255 * board[row][col].log.trap_decay / LOG_TRAP_TICKS;
                type_pixel.background_color =
                    Color::Blend(Color::DarkBlue, Color::HSVA(24, 255, 150, log_alpha));
                type_pixel.foreground_color =
                    Color::Blend(Color::DarkBlue, Color::HSVA(24, 255, 180, log_alpha));
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
        p.character = "▪▪";
        break;
    case WATER:
        p.foreground_color = Color::Blue;
        p.background_color = Color::DarkBlue;
        p.bold = FALSE;
        p.character = "╭╯";
        break;
    case TURTLE:
        p.foreground_color = Color::Green;
        p.background_color = Color::Green;
        p.character = "🐢";
        break;
    case LOG:
        p.foreground_color = Color::DarkOrange;
        p.background_color = Color::Orange3;
        p.character = "▣▣";
        break;
    case TRAP_LOG:
        p.foreground_color = Color::DarkOrange;
        p.background_color = Color::DarkOrange3;
        p.character = "▢▢";
        break;
    default:
        p.foreground_color = Color::White;
        p.foreground_color = Color::White;
        p.character = "  ";
    }
    return p;
}

/*
 * Function: add_turtles_random
 * ---------------------
 * Adds a number of turtles to the board at the given row.
 * The turtles will be placed in random columns.
 *
 * board: The 2D array representing the board.
 * row: The row to add the turtles.
 * num_turtles: The number of turtles to add.
 */
void add_turtles_random(struct board_tile board[SIZE][SIZE], int row, int num_turtles)
{
    if (row < 1 || row >= SIZE - 1)
        return;

    random_device rd;
    mt19937 gen(rd());
    // Random column for the turtle
    uniform_int_distribution<> random_col(0, SIZE - 1);
    // Random length for consecutive turtles
    uniform_int_distribution<> random_length(0, 4);

    // Check if there is enough water tiles to place the turtles
    int water_tiles = 0;
    for (int i = 0; i < SIZE; i++)
    {
        if (board[row][i].type == WATER)
            water_tiles++;
        if (water_tiles >= num_turtles)
            break;
    }
    // If there are not enough water tiles, the turtles will not be placed.
    if (water_tiles < num_turtles)
        return;

    int turtles_placed = 0;
    while (turtles_placed < num_turtles)
    {
        int col = random_col(gen);
        int length = random_length(gen);
        if (col + length >= SIZE)
            length = SIZE - col - 1;
        for (int i = col; i < col + length && turtles_placed < num_turtles && i < SIZE; i++)
        {
            if (i >= 0 && board[row][i].type == WATER)
            {
                add_turtle(board, row, i);
                turtles_placed++;
            }
        }
    }
}