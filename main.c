#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int rows;
    int cols;
    char **grid;
} GameBoard;

GameBoard* create_board(int r, int c) {
    GameBoard *board = (GameBoard*)malloc(sizeof(GameBoard));
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

void print_board(GameBoard *board) {
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

int drop_piece(GameBoard *board, int col, char token) {
    for (int i = board->rows - 1; i >= 0; i--) {
        if (board->grid[i][col] == '.') {
            board->grid[i][col] = token;
            return 1;
        }
    }
    return 0;
}

// تابع جدید برای بررسی برد
int check_win(GameBoard *board, char token) {
    // 1. بررسی افقی (چپ به راست)
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j <= board->cols - 4; j++) {
            if (board->grid[i][j] == token &&
                board->grid[i][j+1] == token &&
                board->grid[i][j+2] == token &&
                board->grid[i][j+3] == token) {
                return 1;
            }
        }
    }

    // 2. بررسی عمودی (بالا به پایین)
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 0; j < board->cols; j++) {
            if (board->grid[i][j] == token &&
                board->grid[i+1][j] == token &&
                board->grid[i+2][j] == token &&
                board->grid[i+3][j] == token) {
                return 1;
            }
        }
    }

    // 3. بررسی قطری (شیب منفی - از بالا چپ به پایین راست)
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 0; j <= board->cols - 4; j++) {
            if (board->grid[i][j] == token &&
                board->grid[i+1][j+1] == token &&
                board->grid[i+2][j+2] == token &&
                board->grid[i+3][j+3] == token) {
                return 1;
            }
        }
    }

    // 4. بررسی قطری (شیب مثبت - از بالا راست به پایین چپ)
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 3; j < board->cols; j++) {
            if (board->grid[i][j] == token &&
                board->grid[i+1][j-1] == token &&
                board->grid[i+2][j-2] == token &&
                board->grid[i+3][j-3] == token) {
                return 1;
            }
        }
    }

    return 0; // هیچ حالت بردی پیدا نشد
}

int main() {
    int r, c;

    while (1) {
        printf("تعداد سطرها (حداقل 4) و ستون‌ها (کمتر از 12) را وارد کنید (مثال: 6 7): ");
        if (scanf("%d %d", &r, &c) != 2) {
            printf("ورودی نامعتبر! لطفا عدد وارد کنید.\n");
            while(getchar() != '\n');
            continue;
        }

        if (r >= 4 && c < 12 && c > 0) {
            break;
        } else {
            printf("خطا: ابعاد وارد شده با قوانین بازی همخوانی ندارد. دوباره تلاش کنید.\n");
        }
    }

    GameBoard *board = create_board(r, c);
    print_board(board);

    char current_token = 'X';
    int turn = 0;
    int max_turns = r * c;

    // حلقه اصلی بازی
    while (turn < max_turns) {
        int selected_col;
        printf("نوبت بازیکن (%c). شماره ستون (0 تا %d) را انتخاب کنید: ", current_token, c - 1);

        if (scanf("%d", &selected_col) != 1) {
            printf("ورودی نامعتبر! لطفا یک عدد وارد کنید.\n");
            while(getchar() != '\n');
            continue;
        }

        if (selected_col < 0 || selected_col >= c) {
            printf("خطا: شماره ستون خارج از محدوده است! دوباره تلاش کنید.\n");
            continue;
        }

        if (drop_piece(board, selected_col, current_token)) {
            print_board(board);

            // بررسی برد بلافاصله بعد از افتادن مهره
            if (check_win(board, current_token)) {
                printf("\n*** تبریک! بازیکن (%c) برنده شد! ***\n\n", current_token);
                break; // خروج از حلقه بازی
            }

            turn++;
            // بررسی مساوی شدن بازی
            if (turn == max_turns) {
                printf("\n*** بازی مساوی شد! هیچ خانه‌ای خالی نمانده است. ***\n\n");
                break;
            }

            current_token = (current_token == 'X') ? 'O' : 'X';
        } else {
            printf("خطا: این ستون کاملاً پر شده است! ستون دیگری انتخاب کنید.\n");
        }
    }

    // آزادسازی حافظه
    for (int i = 0; i < board->rows; i++) {
        free(board->grid[i]);
    }
    free(board->grid);
    free(board);

    return 0;
}
