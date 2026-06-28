#include <stdio.h>
#include <stdlib.h>

// تعریف ساختار زمین بازی
typedef struct {
    int rows;
    int cols;
    char **grid;
} GameBoard;

// تابع برای ساختن زمین بازی (تخصیص حافظه پویا)
GameBoard* create_board(int r, int c) {
    GameBoard *board = (GameBoard*)malloc(sizeof(GameBoard));
    board->rows = r;
    board->cols = c;

    board->grid = (char**)malloc(r * sizeof(char*));
    for (int i = 0; i < r; i++) {
        board->grid[i] = (char*)malloc(c * sizeof(char));
        for (int j = 0; j < c; j++) {
            board->grid[i][j] = '.'; // نقطه به معنای خانه خالی است
        }
    }
    return board;
}

// تابع برای چاپ کردن زمین بازی در ترمینال
void print_board(GameBoard *board) {
    printf("\n=== Connect Four ===\n");
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            printf("%c ", board->grid[i][j]);
        }
        printf("\n");
    }
    printf("====================\n");
    // چاپ شماره ستون‌ها برای راهنمایی کاربر
    for (int j = 0; j < board->cols; j++) {
        printf("%d ", j);
    }
    printf("\n\n");
}

int main() {
    int r, c;

    // گرفتن ابعاد زمین بازی با رعایت شروط داکیومنت
    while (1) {
        printf("تعداد سطرها (حداقل 4) و ستون‌ها (کمتر از 12) را وارد کنید (مثال: 6 7): ");
        if (scanf("%d %d", &r, &c) != 2) {
            printf("ورودی نامعتبر! لطفا عدد وارد کنید.\n");
            while(getchar() != '\n'); // پاک کردن بافر ورودی
            continue;
        }

        if (r >= 4 && c < 12 && c > 0) {
            break;
        } else {
            printf("خطا: ابعاد وارد شده با قوانین بازی همخوانی ندارد. دوباره تلاش کنید.\n");
        }
    }

    // ساخت و نمایش زمین
    GameBoard *board = create_board(r, c);
    print_board(board);

    // آزاد کردن حافظه در پایان (برای جلوگیری از Memory Leak)
    for (int i = 0; i < board->rows; i++) {
        free(board->grid[i]);
    }
    free(board->grid);
    free(board);

    return 0;
}
