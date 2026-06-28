#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ---------- 1. ساختارهای داده (Data Structures) ----------

typedef struct GameState {
    int rows;
    int cols;
    char **grid;
    int *move_history; // آرایه‌ای برای ذخیره تاریخچه حرکات
    int move_count;    // تعداد حرکات انجام شده
} GameState;

typedef int (*MoveFn)(const GameState *st, void *ctx);

typedef struct {
    MoveFn move;
    void *ctx;
    char token;
} Player;

// ---------- 2. توابع مدیریت زمین بازی ----------

GameState* create_board(int r, int c) {
    GameState *board = (GameState*)malloc(sizeof(GameState));
    board->rows = r;
    board->cols = c;
    board->move_count = 0;
    board->move_history = (int*)malloc(r * c * sizeof(int)); // حداکثر حرکات برابر مساحت زمین است

    board->grid = (char**)malloc(r * sizeof(char*));
    for (int i = 0; i < r; i++) {
        board->grid[i] = (char*)malloc(c * sizeof(char));
        for (int j = 0; j < c; j++) {
            board->grid[i][j] = '.';
        }
    }
    return board;
}

void print_board(GameState *board) {
    printf("\n=== Connect Four ===\n");
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            printf("%c ", board->grid[i][j]);
        }
        printf("\n");
    }
    printf("====================\n");
    for (int j = 0; j < board->cols; j++) {
        printf("%d ", j);
    }
    printf("\n\n");
}

int drop_piece(GameState *board, int col, char token) {
    for (int i = board->rows - 1; i >= 0; i--) {
        if (board->grid[i][col] == '.') {
            board->grid[i][col] = token;
            return 1;
        }
    }
    return 0;
}

int check_win(GameState *board, char token) {
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j <= board->cols - 4; j++) {
            if (board->grid[i][j] == token && board->grid[i][j+1] == token &&
                board->grid[i][j+2] == token && board->grid[i][j+3] == token) return 1;
        }
    }
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 0; j < board->cols; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j] == token &&
                board->grid[i+2][j] == token && board->grid[i+3][j] == token) return 1;
        }
    }
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 0; j <= board->cols - 4; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j+1] == token &&
                board->grid[i+2][j+2] == token && board->grid[i+3][j+3] == token) return 1;
        }
    }
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 3; j < board->cols; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j-1] == token &&
                board->grid[i+2][j-2] == token && board->grid[i+3][j-3] == token) return 1;
        }
    }
    return 0;
}

int can_drop(const GameState *board, int col) {
    return board->grid[0][col] == '.';
}

// ---------- 3. توابع سیستم ذخیره و بازپخش (Save & Replay) ----------

// ذخیره در فایل دیتابیس واحد. فرمت: Name Rows Cols MoveCount Move1 Move2 ...
void save_game(GameState *board) {
    char game_name[50];
    char choice;
    printf("\nآیا می‌خواهید این بازی را ذخیره کنید؟ (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        printf("یک نام بدون فاصله برای بازی انتخاب کنید (مثلا game1): ");
        scanf("%s", game_name);

        // باز کردن فایل در حالت Append برای اضافه کردن به انتهای فایل
        FILE *db = fopen("database.txt", "a");
        if (db == NULL) {
            printf("خطا در ایجاد فایل دیتابیس!\n");
            return;
        }

        fprintf(db, "%s %d %d %d", game_name, board->rows, board->cols, board->move_count);
        for (int i = 0; i < board->move_count; i++) {
            fprintf(db, " %d", board->move_history[i]);
        }
        fprintf(db, "\n");
        fclose(db);
        printf("بازی با موفقیت در database.txt ذخیره شد!\n");
    }
}

// تابع بازپخش مرحله به مرحله
void replay_game() {
    char search_name[50], current_name[50];
    int r, c, count, move_col;
    int found = 0;

    FILE *db = fopen("database.txt", "r");
    if (db == NULL) {
        printf("هیچ دیتابیسی یافت نشد. ابتدا یک بازی را ذخیره کنید.\n");
        return;
    }

    printf("نام بازی مورد نظر برای بازپخش را وارد کنید: ");
    scanf("%s", search_name);

    // خواندن فایل خط به خط تا پیدا کردن نام بازی
    while (fscanf(db, "%s %d %d %d", current_name, &r, &c, &count) == 4) {
        if (strcmp(current_name, search_name) == 0) {
            found = 1;
            GameState *board = create_board(r, c);
            char current_token = 'X';

            printf("\n--- شروع بازپخش بازی: %s ---\n", search_name);
            print_board(board);

            for (int i = 0; i < count; i++) {
                fscanf(db, "%d", &move_col); // خواندن حرکت از فایل

                printf("حرکت %d: بازیکن (%c) ستون %d را انتخاب کرد.\n", i + 1, current_token, move_col);
                printf("(برای دیدن حرکت بعدی Enter را بزنید...)");
                while(getchar() != '\n'); // تمیز کردن بافر
                getchar(); // منتظر ماندن برای فشردن Enter

                drop_piece(board, move_col, current_token);
                print_board(board);
                current_token = (current_token == 'X') ? 'O' : 'X';
            }
            printf("--- پایان بازپخش ---\n");

            for (int i = 0; i < board->rows; i++) free(board->grid[i]);
            free(board->grid);
            free(board->move_history);
            free(board);
            break;
        } else {
            // اگر این خط بازی ما نبود، حرکاتش را اسکیپ می‌کنیم تا به خط بعدی برسیم
            for (int i = 0; i < count; i++) fscanf(db, "%d", &move_col);
        }
    }

    if (!found) printf("بازی با این نام در دیتابیس پیدا نشد.\n");
    fclose(db);
}

// ---------- 4. توابع حرکت بازیکن‌ها ----------

int human_move(const GameState *st, void *ctx) {
    int selected_col;
    while (1) {
        if (scanf("%d", &selected_col) != 1) {
            printf("ورودی نامعتبر! عدد وارد کنید: ");
            while(getchar() != '\n');
            continue;
        }
        if (selected_col < 0 || selected_col >= st->cols) {
            printf("خطا: ستون خارج از محدوده است! (0 تا %d): ", st->cols - 1);
            continue;
        }
        return selected_col;
    }
}

int file_move(const GameState *st, void *ctx) {
    FILE *file = (FILE*)ctx;
    int selected_col;
    if (fscanf(file, "%d", &selected_col) == 1) {
        printf("حرکت خوانده شده از فایل: %d\n", selected_col);
        if (selected_col < 0 || selected_col >= st->cols) return -1;
        return selected_col;
    }
    return -1;
}

int computer_move(const GameState *st, void *ctx) {
    int level = *(int*)ctx;
    char ai_token = 'O';
    char human_token = 'X';

    if (level == 2) {
        for (int c = 0; c < st->cols; c++) {
            if (can_drop(st, c)) {
                int r;
                for (r = st->rows - 1; r >= 0; r--) if (st->grid[r][c] == '.') break;
                st->grid[r][c] = ai_token;
                if (check_win((GameState*)st, ai_token)) {
                    st->grid[r][c] = '.';
                    printf("کامپیوتر حرکت هوشمندانه کرد (ستون %d)\n", c);
                    return c;
                }
                st->grid[r][c] = '.';
            }
        }
        for (int c = 0; c < st->cols; c++) {
            if (can_drop(st, c)) {
                int r;
                for (r = st->rows - 1; r >= 0; r--) if (st->grid[r][c] == '.') break;
                st->grid[r][c] = human_token;
                if (check_win((GameState*)st, human_token)) {
                    st->grid[r][c] = '.';
                    printf("کامپیوتر دفاع کرد (ستون %d)\n", c);
                    return c;
                }
                st->grid[r][c] = '.';
            }
        }
    }

    int col;
    do { col = rand() % st->cols; } while (!can_drop(st, col));
    printf("کامپیوتر ستون %d را انتخاب کرد.\n", col);
    return col;
}

// ---------- 5. تابع اصلی ----------

int main() {
    srand(time(NULL));

    int mode;
    printf("=== منوی اصلی ===\n");
    printf("1. بازی جدید (انسان در برابر انسان)\n");
    printf("2. بازی جدید (انسان در برابر فایل)\n");
    printf("3. بازی جدید (انسان در برابر کامپیوتر)\n");
    printf("4. بازپخش یک بازی ذخیره شده (Replay)\n");
    printf("انتخاب شما: ");

    if (scanf("%d", &mode) != 1 || mode < 1 || mode > 4) {
        printf("انتخاب نامعتبر! خروج.\n");
        return 1;
    }

    if (mode == 4) {
        replay_game();
        return 0; // بعد از بازپخش، برنامه تمام می‌شود
    }

    int r, c;
    while (1) {
        printf("تعداد سطرها (حداقل 4) و ستون‌ها (کمتر از 12) را وارد کنید: ");
        if (scanf("%d %d", &r, &c) != 2) {
            printf("ورودی نامعتبر!\n");
            while(getchar() != '\n');
            continue;
        }
        if (r >= 4 && c < 12 && c > 0) break;
        else printf("خطا: ابعاد با قوانین داکیومنت همخوانی ندارد.\n");
    }

    GameState *board = create_board(r, c);
    print_board(board);

    Player p1 = { human_move, NULL, 'X' };
    Player p2 = { human_move, NULL, 'O' };
    FILE *inputFile = NULL;
    int ai_level = 1;

    if (mode == 2) {
        char filename[100];
        printf("نام فایل حرکات را وارد کنید: ");
        scanf("%s", filename);
        inputFile = fopen(filename, "r");
        if (inputFile != NULL) {
            p2.move = file_move;
            p2.ctx = inputFile;
        } else {
            printf("خطا! فایل پیدا نشد. بازی دو نفره ادامه می‌یابد.\n");
        }
    } else if (mode == 3) {
        printf("سطح سختی کامپیوتر (1: آسان، 2: سخت): ");
        scanf("%d", &ai_level);
        p2.move = computer_move;
        p2.ctx = &ai_level;
    }

    Player *players[2] = { &p1, &p2 };
    int current_player_idx = 0;
    int turn = 0;
    int max_turns = r * c;

    while (turn < max_turns) {
        Player *current_player = players[current_player_idx];

        if (mode == 1 || current_player_idx == 0) {
            printf("نوبت بازیکن (%c): ", current_player->token);
        }

        int selected_col = current_player->move(board, current_player->ctx);
        if (selected_col == -1) break;

        if (drop_piece(board, selected_col, current_player->token)) {
            // ثبت حرکت در تاریخچه
            board->move_history[board->move_count++] = selected_col;

            print_board(board);

            if (check_win(board, current_player->token)) {
                printf("\n*** بازیکن (%c) برنده شد! ***\n", current_player->token);
                break;
            }
            turn++;
            if (turn == max_turns) {
                printf("\n*** بازی مساوی شد! ***\n");
                break;
            }
            current_player_idx = 1 - current_player_idx;
        } else {
            if (current_player->move == human_move) {
                printf("خطا: این ستون پر است!\n");
            }
        }
    }

    // فراخوانی تابع ذخیره در پایان بازی
    save_game(board);

    if (inputFile != NULL) fclose(inputFile);
    for (int i = 0; i < board->rows; i++) free(board->grid[i]);
    free(board->grid);
    free(board->move_history);
    free(board);

    return 0;
}
