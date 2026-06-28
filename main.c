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
    void *ctx; // برای دیتای خاص بازیکن (در اینجا اشاره‌گر فایل FILE* قرار می‌گیرد)
    char token;
} Player;

// ---------- توابع مدیریت زمین بازی ----------

// تابع برای ساختن زمین بازی (تخصیص حافظه پویا)
GameState* create_board(int r, int c) {
    GameState *board = (GameState*)malloc(sizeof(GameState));
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

// تابع برای چاپ کردن زمین بازی در ترمینال اوبونتو
void print_board(GameState *board) {
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
// خروجی: اگر حرکت موفق بود 1 و اگر ستون پر بود 0 برمی‌گرداند
int drop_piece(GameState *board, int col, char token) {
    // از پایین‌ترین سطر شروع می‌کنیم و به سمت بالا می‌آییم
    for (int i = board->rows - 1; i >= 0; i--) {
        if (board->grid[i][col] == '.') {
            board->grid[i][col] = token;
            return 1; // حرکت موفقیت‌آمیز بود
        }
    }
    return 0; // ستون پر است
}

// تابع برای بررسی شرایط برد (چهار مهره هم‌رنگ پشت سر هم)
int check_win(GameState *board, char token) {
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

// ---------- توابع حرکت بازیکن‌ها ----------

// تابع حرکت اختصاصی برای بازیکنِ انسان
int human_move(const GameState *st, void *ctx) {
    int selected_col;
    while (1) {
        if (scanf("%d", &selected_col) != 1) {
            printf("ورودی نامعتبر! لطفا یک عدد وارد کنید: ");
            while(getchar() != '\n'); // پاک کردن بافر ورودی
            continue;
        }
        if (selected_col < 0 || selected_col >= st->cols) {
            printf("خطا: ستون خارج از محدوده است! انتخاب مجدد (0 تا %d): ", st->cols - 1);
            continue;
        }
        return selected_col;
    }
}

// تابع جدید: حرکت اختصاصی برای خواندن از فایل
int file_move(const GameState *st, void *ctx) {
    FILE *file = (FILE*)ctx; // تبدیل تیکت void* به پوینتر فایل
    int selected_col;

    // خواندن یک عدد از فایل متنی
    if (fscanf(file, "%d", &selected_col) == 1) {
        printf("حرکت خوانده شده از فایل: %d\n", selected_col);

        // اعتبارسنجی اولیه ستون خوانده شده از فایل
        if (selected_col < 0 || selected_col >= st->cols) {
            printf("خطا در فایل: شماره ستون %d خارج از محدوده زمین است!\n", selected_col);
            return -1; // کد خطا برای پایان یا خرابی فایل
        }
        return selected_col;
    }

    printf("هشدار: حرکات فایل تمام شد یا فایل معتبر نیست!\n");
    return -1; // پایان فایل (EOF)
}

// ---------- تابع اصلی (Game Engine) ----------

int main() {
    int r, c;

    // گرفتن ابعاد زمین بازی با رعایت شروط داکیومنت (R >= 4 و C < 12)
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

    // منوی انتخاب حالت بازی
    int mode = 1;
    printf("\nحالت بازی را انتخاب کنید:\n1. بازیکن علیه بازیکن (Human vs Human)\n2. خواندن حرکات بازیکن دوم از فایل (File Mode)\nانتخاب شما (1 یا 2): ");
    scanf("%d", &mode);

    GameState *board = create_board(r, c);
    print_board(board);

    // مقداردهی اولیه بازیکن‌ها بر اساس معماری موتور بازی
    Player p1 = { human_move, NULL, 'X' };
    Player p2 = { human_move, NULL, 'O' }; // پیش‌فرض انسان است

    FILE *inputFile = NULL;

    if (mode == 2) {
        char filename[100];
        printf("نام فایل متنی حاوی حرکات را وارد کنید (مثال: moves.txt): ");
        scanf("%s", filename);

        inputFile = fopen(filename, "r");
        if (inputFile == NULL) {
            printf("خطا: فایل پیدا نشد! بازی به صورت دو نفره عادی ادامه می‌یابد.\n");
        } else {
            // تغییر تابع حرکت بازیکن دوم به تابع فایل و فرستادن پوینتر فایل در ctx
            p2.move = file_move;
            p2.ctx = inputFile;
            printf("فایل با موفقیت باز شد. حرکات بازیکن O از فایل خوانده می‌شود.\n");
        }
    }

    Player *players[2] = { &p1, &p2 };
    int current_player_idx = 0;

    int turn = 0;
    int max_turns = r * c;

    // حلقه اصلی موتور بازی
    while (turn < max_turns) {
        Player *current_player = players[current_player_idx];

        printf("نوبت بازیکن (%c): ", current_player->token);

        // صدا زدن تابع حرکت (کامپایلر بر اساس پوینتر توابع خودش می‌فهمد کدام تابع اجرا شود)
        int selected_col = current_player->move(board, current_player->ctx);

        // اگر فایل تمام شد یا دیتای غلط داشت
        if (selected_col == -1) {
            printf("بازی به علت اتمام یا خطای فایل متوقف شد.\n");
            break;
        }

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

            current_player_idx = 1 - current_player_idx;
        } else {
            printf("خطا: ستون %d پر است! حرکت نامعتبر بود.\n", selected_col);
            if (mode == 2 && current_player_idx == 1) {
                printf("حرکت فایل نامعتبر بود. بازی متوقف می‌شود.\n");
                break;
            }
        }
    }

    // بستن فایل در صورت باز بودن
    if (inputFile != NULL) {
        fclose(inputFile);
    }

    // آزاد کردن حافظه پویا برای جلوگیری از Memory Leak
    for (int i = 0; i < board->rows; i++) free(board->grid[i]);
    free(board->grid);
    free(board);

    return 0;
}
