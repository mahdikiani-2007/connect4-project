#include <stdio.h>
#include <stdlib.h>

// 1. تعریف ساختار وضعیت بازی (طبق داکیومنت)
typedef struct GameState {
    int rows;
    int cols;
    char **grid;
} GameState;

// 2. تعریف پوینتر تابع برای حرکت (طبق داکیومنت)
// این تابع وضعیت بازی را می‌گیرد و شماره ستون انتخابی را برمی‌گرداند
typedef int (*MoveFn)(const GameState *st, void *ctx);

// 3. ساختار بازیکن شامل تابع حرکت، دیتای اختصاصی و مهره (طبق داکیومنت)
typedef struct {
    MoveFn move;
    void *ctx; // برای دیتای خاص بازیکن (مثل فایل یا سطح سختی)
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
    return 0; // ستون پر است
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
    // قطری (شیب منفی)
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 0; j <= board->cols - 4; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j+1] == token &&
                board->grid[i+2][j+2] == token && board->grid[i+3][j+3] == token) return 1;
        }
    }
    // قطری (شیب مثبت)
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 3; j < board->cols; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j-1] == token &&
                board->grid[i+2][j-2] == token && board->grid[i+3][j-3] == token) return 1;
        }
    }
    return 0;
}

// ---------- توابع بازیکن‌ها ----------

// تابع حرکت اختصاصی برای بازیکنِ انسان
int human_move(const GameState *st, void *ctx) {
    int selected_col;
    while (1) {
        // گرفتن ورودی
        if (scanf("%d", &selected_col) != 1) {
            printf("ورودی نامعتبر! لطفا یک عدد وارد کنید: ");
            while(getchar() != '\n');
            continue;
        }
        // اعتبارسنجی اولیه محدوده
        if (selected_col < 0 || selected_col >= st->cols) {
            printf("خطا: ستون خارج از محدوده است! انتخاب مجدد (0 تا %d): ", st->cols - 1);
            continue;
        }
        return selected_col;
    }
}

// ---------- تابع اصلی (Game Engine) ----------

int main() {
    int r, c;

    while (1) {
        printf("تعداد سطرها (حداقل 4) و ستون‌ها (کمتر از 12) را وارد کنید (مثال: 6 7): ");
        if (scanf("%d %d", &r, &c) != 2) {
            printf("ورودی نامعتبر! لطفا عدد وارد کنید.\n");
            while(getchar() != '\n');
            continue;
        }
        if (r >= 4 && c < 12 && c > 0) break;
        else printf("خطا: ابعاد وارد شده با قوانین بازی همخوانی ندارد. دوباره تلاش کنید.\n");
    }

    GameState *board = create_board(r, c);
    print_board(board);

    // مقداردهی اولیه بازیکن‌ها با ساختاری که موتور بازی می‌فهمد
    Player p1 = { human_move, NULL, 'X' };
    Player p2 = { human_move, NULL, 'O' };

    Player *players[2] = { &p1, &p2 }; // آرایه‌ای از بازیکنان برای تعویض راحت نوبت
    int current_player_idx = 0;

    int turn = 0;
    int max_turns = r * c;

    // حلقه اصلی موتور بازی
    while (turn < max_turns) {
        Player *current_player = players[current_player_idx];

        printf("نوبت بازیکن (%c). شماره ستون را انتخاب کنید: ", current_player->token);

        // صدا زدن تابع حرکت (بدون اینکه موتور بدونه این انسانه یا کامپیوتر)
        int selected_col = current_player->move(board, current_player->ctx);

        if (drop_piece(board, selected_col, current_player->token)) {
            print_board(board);

            if (check_win(board, current_player->token)) {
                printf("\n*** تبریک! بازیکن (%c) برنده شد! ***\n\n", current_player->token);
                break;
            }

            turn++;
            if (turn == max_turns) {
                printf("\n*** بازی مساوی شد! هیچ خانه‌ای خالی نمانده است. ***\n\n");
                break;
            }

            // عوض کردن نوبت
            current_player_idx = 1 - current_player_idx;
        } else {
            printf("خطا: این ستون کاملاً پر شده است! ستون دیگری انتخاب کنید.\n");
        }
    }

    for (int i = 0; i < board->rows; i++) free(board->grid[i]);
    free(board->grid);
    free(board);

    return 0;
}
