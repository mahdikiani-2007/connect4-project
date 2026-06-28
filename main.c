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

// تابع اصلی سقوط مهره در پایین‌ترین خانه خالی ستون
// خروجی: اگر حرکت موفق بود ۱ و اگر ستون پر بود ۰ برمی‌گرداند
int drop_piece(GameBoard *board, int col, char token) {
    // از پایین‌ترین سطر شروع می‌کنیم و به سمت بالا می‌آییم
    for (int i = board->rows - 1; i >= 0; i--) {
        if (board->grid[i][col] == '.') {
            board->grid[i][col] = token;
            return 1; // حرکت موفقیت‌آمیز بود
        }
    }
    return 0; // ستون پر است
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

    GameBoard *board = create_board(r, c);
    print_board(board);

    // یک حلقه ساده برای تست نوبت بازیکن‌ها و سقوط مهره‌ها
    char current_token = 'X'; // بازیکن اول X و بازیکن دوم O
    int turn = 0;
    int max_turns = r * c;

    while (turn < max_turns) {
        int selected_col;
        printf("نوبت بازیکن (%c). شماره ستون (0 تا %d) را انتخاب کنید: ", current_token, c - 1);

        if (scanf("%d", &selected_col) != 1) {
            printf("ورودی نامعتبر! لطفا یک عدد وارد کنید.\n");
            while(getchar() != '\n');
            continue;
        }

        // اعتبارسنجی محدوده ستون
        if (selected_col < 0 || selected_col >= c) {
            printf("خطا: شماره ستون خارج از محدوده است! دوباره تلاش کنید.\n");
            continue;
        }

        // تلاش برای انداختن مهره
        if (drop_piece(board, selected_col, current_token)) {
            print_board(board);
            // عوض کردن نوبت بازیکن
            current_token = (current_token == 'X') ? 'O' : 'X';
            turn++;
        } else {
            printf("خطا: این ستون کاملاً پر شده است! ستون دیگری انتخاب کنید.\n");
        }
    }

    // آزاد کردن حافظه
    for (int i = 0; i < board->rows; i++) {
        free(board->grid[i]);
    }
    free(board->grid);
    free(board);

    return 0;
}
