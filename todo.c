#include <ncurses.h>
#include <string.h>
#include <stdio.h>

#define MAX_TODOS 10
#define TODO_FILE "todos.txt"

char todos[MAX_TODOS][256];
int todo_count = 0;
int selected_todo = 0;
int completed[MAX_TODOS] = {0};

void save_todos() {
    FILE *file = fopen(TODO_FILE, "w");
    if (file) {
        for (int i = 0; i < todo_count; i++) {
            fprintf(file, "%d %s\n", completed[i], todos[i]);
        }
        fclose(file);
    }
}

void load_todos() {
    FILE *file = fopen(TODO_FILE, "r");
    if (file) {
        while (todo_count < MAX_TODOS && fscanf(file, "%d %[^\n]", &completed[todo_count], todos[todo_count]) != EOF) {
            todo_count++;
        }
        fclose(file);
    }
}

void draw_window() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    attron(COLOR_PAIR(1));
    box(stdscr, 0, 0);
    attroff(COLOR_PAIR(1));
    
    attron(A_BOLD);
    mvprintw(0, (max_x - 4) / 2, "TODO");
    attroff(A_BOLD);
    
    refresh();
}

void draw_instruction_window() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int win_height = 5;
    int win_width = max_x / 2;
    int start_y = (max_y / 6) - 2;
    int start_x = (max_x - win_width) / 2;
    
    WINDOW *inst_win = newwin(win_height, win_width, start_y, start_x);
    
    wattron(inst_win, COLOR_PAIR(4));
    box(inst_win, 0, 0);
    wattroff(inst_win, COLOR_PAIR(4));
    
    wattron(inst_win, COLOR_PAIR(4));
    mvwprintw(inst_win, 1, 2, "a - Add todo");
    mvwprintw(inst_win, 2, 2, "t - Toggle completion, d - Delete");
    mvwprintw(inst_win, 3, 2, "j/k or Up/Down - Navigate, q - Quit");
    wattroff(inst_win, COLOR_PAIR(4));
    
    wrefresh(inst_win);
}

void draw_centered_window() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int win_height = max_y / 3;
    int win_width = max_x / 2;
    int start_y = (max_y - win_height) / 2;
    int start_x = (max_x - win_width) / 2;
    
    WINDOW *center_win = newwin(win_height, win_width, start_y, start_x);
    
    wattron(center_win, COLOR_PAIR(2));
    box(center_win, 0, 0);
    wattroff(center_win, COLOR_PAIR(2));
    
    if (todo_count == 0) {
        mvwprintw(center_win, win_height / 2, (win_width - 9) / 2, "No todos...");
    } else {
        int start_display_y = (win_height - todo_count) / 2;
        int text_x = 2;
        for (int i = 0; i < todo_count; i++) {
            if (i == selected_todo) {
                wattron(center_win, A_REVERSE);
            }
            mvwprintw(center_win, start_display_y + i, text_x, "%d. [%c] %s", i + 1, completed[i] ? 'X' : ' ', todos[i]);
            if (i == selected_todo) {
                wattroff(center_win, A_REVERSE);
            }
        }
    }
    
    wrefresh(center_win);
}

void add_todo() {
    if (todo_count >= MAX_TODOS) return;

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int center_win_height = max_y / 3;
    int center_win_width = max_x / 2;
    int center_win_start_y = (max_y - center_win_height) / 2;
    int center_win_start_x = (max_x - center_win_width) / 2;

    int input_win_height = 5;
    int input_win_width = center_win_width;
    int input_win_start_y = center_win_start_y + center_win_height + 1;
    int input_win_start_x = center_win_start_x;

    WINDOW *input_win = newwin(input_win_height, input_win_width, input_win_start_y, input_win_start_x);
    wattron(input_win, COLOR_PAIR(3));
    box(input_win, 0, 0);
    wattroff(input_win, COLOR_PAIR(3));

    mvwprintw(input_win, 1, 2, "Add new TODO:");
    wrefresh(input_win);

    echo();
    curs_set(1);
    mvwgetnstr(input_win, 2, 2, todos[todo_count], 255);
    noecho();
    curs_set(0);

    completed[todo_count] = 0;
    todo_count++;
    save_todos();

    delwin(input_win);
    draw_window();
    draw_centered_window();
    draw_instruction_window();
}

void delete_todo() {
    if (todo_count > 0) {
        for (int i = selected_todo; i < todo_count - 1; i++) {
            strcpy(todos[i], todos[i + 1]);
            completed[i] = completed[i + 1];
        }
        todo_count--;
        if (selected_todo >= todo_count) {
            selected_todo = todo_count - 1;
        }
        save_todos();
        draw_centered_window();
    }
}

void toggle_todo() {
    if (todo_count > 0) {
        completed[selected_todo] = !completed[selected_todo];
        save_todos();
        draw_centered_window();
    }
}

int main() {
    initscr();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    load_todos();
    draw_window();
    draw_instruction_window();
    draw_centered_window();
    
    int ch;
    while ((ch = getch()) != 'q') {
        if (ch == 'a') {
            add_todo();
        } else if ((ch == KEY_UP || ch == 'k') && todo_count > 0) {
            selected_todo = (selected_todo > 0) ? selected_todo - 1 : todo_count - 1;
            draw_centered_window();
        } else if ((ch == KEY_DOWN || ch == 'j') && todo_count > 0) {
            selected_todo = (selected_todo < todo_count - 1) ? selected_todo + 1 : 0;
            draw_centered_window();
        } else if (ch == 't') {
            toggle_todo();
        } else if (ch == 'd') {
            delete_todo();
        }
    }
    
    endwin();
    return 0;
}
