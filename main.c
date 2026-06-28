#include <stdio.h>
#include <stdlib.h>
#include <time.h> // برای تولید اعداد تصادفی هوش مصنوعی

typedef struct GameState {
    int rows;
    int cols;
    char **grid;
} GameState;

typedef int (*MoveFn)(const GameState *st, void *ctx);

typedef struct {
    MoveFn move;
    void *ctx;
    char token;
} Player;

// ---------- توابع مدیریت زمین بازی ----------

GameState* create_board(int r, int c) {
    GameState *board = (GameState*)malloc(sizeof(GameState));
    board->rows = r;
    board->cols = c;

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
    // افقی
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j <= board->cols - 4; j++) {
            if (board->grid[i][j] == token && board->grid[i][j+1] == token &&
                board->grid[i][j+2] == token && board->grid[i][j+3] == token) return 1;
        }
    }
    // عمودی
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 0; j < board->cols; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j] == token &&
                board->grid[i+2][j] == token && board->grid[i+3][j] == token) return 1;
        }
    }
    // قطری شیب منفی
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 0; j <= board->cols - 4; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j+1] == token &&
                board->grid[i+2][j+2] == token && board->grid[i+3][j+3] == token) return 1;
        }
    }
    // قطری شیب مثبت
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 3; j < board->cols; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j-1] == token &&
                board->grid[i+2][j-2] == token && board->grid[i+3][j-3] == token) return 1;
        }
    }
    return 0;
}

// تابع کمکی برای هوش مصنوعی: آیا ستون جای خالی دارد؟
int can_drop(const GameState *board, int col) {
    return board->grid[0][col] == '.';
}

// ---------- توابع حرکت بازیکن‌ها ----------

int human_move(const GameState *st, void *ctx) {
    int selected_col;
    while (1) {
        if (scanf("%d", &selected_col) != 1) {
            printf("ورودی نامعتبر! لطفا یک عدد وارد کنید: ");
            while(getchar() != '\n');
            continue;
        }
        if (selected_col < 0 || selected_col >= st->cols) {
            printf("خطا: ستون خارج از محدوده است! انتخاب مجدد (0 تا %d): ", st->cols - 1);
            continue;
        }
        return selected_col;
    }
}

int file_move(const GameState *st, void *ctx) {
    FILE *file = (FILE*)ctx;
    int selected_col;
    if (fscanf(file, "%d", &selected_col) == 1) {
        printf("حرکت فایل: %d\n", selected_col);
        if (selected_col < 0 || selected_col >= st->cols) return -1;
        return selected_col;
    }
    return -1;
}

// تابع جدید: حرکت هوش مصنوعی (کامپیوتر)
int computer_move(const GameState *st, void *ctx) {
    int level = *(int*)ctx; // خواندن سطح سختی از ctx
    char ai_token = 'O';
    char human_token = 'X';

    // سطح 2: بررسی بلاک کردن حریف یا برنده شدن خودش
    if (level == 2) {
        // اولویت اول: آیا کامپیوتر می‌تواند با این حرکت برنده شود؟
        for (int c = 0; c < st->cols; c++) {
            if (can_drop(st, c)) {
                // شبیه‌سازی حرکت
                int r;
                for (r = st->rows - 1; r >= 0; r--) if (st->grid[r][c] == '.') break;
                st->grid[r][c] = ai_token;

                if (check_win((GameState*)st, ai_token)) {
                    st->grid[r][c] = '.'; // خنثی کردن حرکت شبیه‌سازی شده
                    printf("کامپیوتر تصمیم هوشمندانه گرفت (حمله در ستون %d)\n", c);
                    return c;
                }
                st->grid[r][c] = '.'; // خنثی کردن
            }
        }

        // اولویت دوم: آیا انسان در حرکت بعدی برنده می‌شود؟ (بلاک کردن)
        for (int c = 0; c < st->cols; c++) {
            if (can_drop(st, c)) {
                // شبیه‌سازی حرکت انسان
                int r;
                for (r = st->rows - 1; r >= 0; r--) if (st->grid[r][c] == '.') break;
                st->grid[r][c] = human_token;

                if (check_win((GameState*)st, human_token)) {
                    st->grid[r][c] = '.';
                    printf("کامپیوتر جلوی برد شما را گرفت! (دفاع در ستون %d)\n", c);
                    return c;
                }
                st->grid[r][c] = '.';
            }
        }
    }

    // سطح 1 (یا اگر در سطح 2 هیچ خطر و فرصتی نبود): حرکت تصادفی در ستون خالی
    int col;
    do {
        col = rand() % st->cols;
    } while (!can_drop(st, col));

    printf("کامپیوتر ستون %d را انتخاب کرد.\n", col);
    return col;
}

// ---------- تابع اصلی ----------

int main() {
    srand(time(NULL)); // مقداردهی اولیه برای اعداد تصادفی هوش مصنوعی
    int r, c;

    while (1) {
        printf("تعداد سطرها (حداقل 4) و ستون‌ها (کمتر از 12) را وارد کنید: ");
        if (scanf("%d %d", &r, &c) != 2) {
            printf("ورودی نامعتبر!\n");
            while(getchar() != '\n');
            continue;
        }
        if (r >= 4 && c < 12 && c > 0) break;
        else printf("خطا: ابعاد نامعتبر.\n");
    }

    int mode = 1;
    printf("\nحالت بازی را انتخاب کنید:\n");
    printf("1. انسان در برابر انسان\n2. انسان در برابر فایل\n3. انسان در برابر کامپیوتر\nانتخاب: ");
    scanf("%d", &mode);

    GameState *board = create_board(r, c);
    print_board(board);

    Player p1 = { human_move, NULL, 'X' };
    Player p2 = { human_move, NULL, 'O' };

    FILE *inputFile = NULL;
    int ai_level = 1; // متغیر برای ذخیره سطح سختی در حافظه

    if (mode == 2) {
        char filename[100];
        printf("نام فایل را وارد کنید: ");
        scanf("%s", filename);
        inputFile = fopen(filename, "r");
        if (inputFile != NULL) {
            p2.move = file_move;
            p2.ctx = inputFile;
        }
    } else if (mode == 3) {
        printf("سطح سختی کامپیوتر (1: آسان، 2: سخت) را انتخاب کنید: ");
        scanf("%d", &ai_level);
        p2.move = computer_move;
        // آدرس متغیر سطح سختی را به ctx می‌دهیم تا کامپیوتر آن را بخواند
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
            print_board(board);

            if (check_win(board, current_player->token)) {
                printf("\n*** بازیکن (%c) برنده شد! ***\n\n", current_player->token);
                break;
            }

            turn++;
            if (turn == max_turns) {
                printf("\n*** بازی مساوی شد! ***\n\n");
                break;
            }

            current_player_idx = 1 - current_player_idx;
        } else {
            if (current_player->move == human_move) {
                printf("خطا: ستون پر است!\n");
            }
        }
    }

    if (inputFile != NULL) fclose(inputFile);
    for (int i = 0; i < board->rows; i++) free(board->grid[i]);
    free(board->grid);
    free(board);

    return 0;
}
