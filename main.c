#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ---------- 1. ساختارهای داده (Data Structures) ----------
// در این بخش ساختارهای اصلی برای نگهداری وضعیت بازی و اطلاعات بازیکنان تعریف شده‌اند.

typedef struct GameState {
    int rows;          // تعداد سطرهای زمین بازی
    int cols;          // تعداد ستون‌های زمین بازی
    char **grid;       // ماتریس دو بعدی (آرایه پویا) برای نگهداری وضعیت خانه‌های بازی
    int *move_history; // آرایه‌ای برای ذخیره تاریخچه حرکات (شماره ستون‌هایی که مهره در آن‌ها افتاده است)
    int move_count;    // تعداد کل حرکات انجام شده تا این لحظه (جهت پیمایش تاریخچه)
} GameState;

// تعریف یک 'اشاره‌گر به تابع' (Function Pointer) به نام MoveFn.
// این الگو به ما اجازه می‌دهد تا نحوه انتخاب حرکت (توسط انسان، هوش مصنوعی یا از روی فایل)
// را به صورت داینامیک (پویا) در زمان اجرا تعیین کنیم و کدهای تکراری ننویسیم.
// هر تابعی که خروجی int و ورودی‌های (const GameState*, void*) داشته باشد، می‌تواند در این الگو جای بگیرد.
typedef int (*MoveFn)(const GameState *st, void *ctx);

typedef struct {
    MoveFn move;       // اشاره‌گر به تابعی که منطق حرکت این بازیکن را پیاده‌سازی می‌کند
    void *ctx;         // متغیر context (متن/زمینه): برای ارسال داده‌های اختصاصی (مثل اشاره‌گرِ فایل یا سطح سختی هوش مصنوعی)
    char token;        // کاراکتر نمایشی مهره بازیکن در زمین بازی (مثلاً 'X' یا 'O')
} Player;

// ---------- 2. توابع مدیریت زمین بازی (Board Management) ----------
// این توابع مسئول تخصیص حافظه، مقداردهی اولیه، نمایش در کنسول و اعمال تغییرات روی زمین هستند.

GameState* create_board(int r, int c) {
    // تخصیص حافظه پویا (Dynamic Memory Allocation) برای ساختار وضعیت بازی
    GameState *board = (GameState*)malloc(sizeof(GameState));
    board->rows = r;
    board->cols = c;
    board->move_count = 0;

    // حداکثر تعداد حرکات ممکن برابر با مساحت کل زمین (تعداد سطر × تعداد ستون) است
    board->move_history = (int*)malloc(r * c * sizeof(int));

    // تخصیص حافظه برای ماتریس دو بعدی زمین بازی.
    // ابتدا یک آرایه از اشاره‌گرها (برای سطرها) می‌سازیم، سپس برای هر سطر یک آرایه از کاراکترها (برای ستون‌ها).
    board->grid = (char**)malloc(r * sizeof(char*));
    for (int i = 0; i < r; i++) {
        board->grid[i] = (char*)malloc(c * sizeof(char));
        for (int j = 0; j < c; j++) {
            board->grid[i][j] = '.'; // پر کردن اولیه تمامی خانه‌ها با کاراکتر نقطه (نشانگر خانه خالی)
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
    // چاپ شماره ستون‌ها در پایین زمین بازی جهت راهنمایی کاربر
    for (int j = 0; j < board->cols; j++) {
        printf("%d ", j);
    }
    printf("\n\n");
}

int drop_piece(GameState *board, int col, char token) {
    // در بازی دوز ایستاده، مهره‌ها تحت تاثیر جاذبه به پایین‌ترین خانه خالی ممکن می‌روند.
    // بنابراین از پایین‌ترین سطر (rows - 1) به سمت بالا چک می‌کنیم.
    for (int i = board->rows - 1; i >= 0; i--) {
        if (board->grid[i][col] == '.') {
            board->grid[i][col] = token; // قرار دادن مهره در اولین جای خالی یافت شده
            return 1; // موفقیت آمیز بودن حرکت
        }
    }
    return 0; // در صورتی که حلقه تمام شود و جای خالی پیدا نشود، یعنی ستون پر است
}

// این تابع پس از هر حرکت فراخوانی می‌شود تا بررسی کند آیا بازیکن (با مهره مشخص شده) برنده شده است یا خیر.
int check_win(GameState *board, char token) {
    // ۱. بررسی خطوط افقی: یافتن ۴ مهره همرنگ مجاور در یک سطر
    // محدوده جستجو در ستون‌ها به (cols - 4) محدود می‌شود تا از خطای Out of Bounds جلوگیری شود
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j <= board->cols - 4; j++) {
            if (board->grid[i][j] == token && board->grid[i][j+1] == token &&
                board->grid[i][j+2] == token && board->grid[i][j+3] == token) return 1;
        }
    }
    // ۲. بررسی خطوط عمودی: یافتن ۴ مهره همرنگ مجاور در یک ستون
    // محدوده جستجو در سطرها به (rows - 4) محدود می‌شود
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 0; j < board->cols; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j] == token &&
                board->grid[i+2][j] == token && board->grid[i+3][j] == token) return 1;
        }
    }
    // ۳. بررسی قطر اصلی (مورب از بالا-چپ به پایین-راست):
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 0; j <= board->cols - 4; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j+1] == token &&
                board->grid[i+2][j+2] == token && board->grid[i+3][j+3] == token) return 1;
        }
    }
    // ۴. بررسی قطر فرعی (مورب از بالا-راست به پایین-چپ):
    for (int i = 0; i <= board->rows - 4; i++) {
        for (int j = 3; j < board->cols; j++) {
            if (board->grid[i][j] == token && board->grid[i+1][j-1] == token &&
                board->grid[i+2][j-2] == token && board->grid[i+3][j-3] == token) return 1;
        }
    }
    return 0; // در صورتی که هیچ الگوی بردی یافت نشد
}

// تابعی ساده برای بررسی اینکه آیا ستون ظرفیت خالی برای دریافت مهره جدید دارد یا خیر
// (اگر بالاترین سطر در یک ستون خالی باشد، آن ستون ظرفیت دارد)
int can_drop(const GameState *board, int col) {
    return board->grid[0][col] == '.';
}

// ---------- 3. توابع سیستم ذخیره و بازپخش (Save & Replay) ----------
// این بخش از فایل‌های متنی برای نگهداری داده‌های بازی (File Handling) استفاده می‌کند.

// تابع ذخیره بازی در فایل دیتابیس (database.txt)
// فرمت ذخیره‌سازی: [GameName] [Rows] [Cols] [MoveCount] [Move1] [Move2] ...
void save_game(GameState *board) {
    char game_name[50];
    char choice;
    printf("\nDo you want to save this game? (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        printf("Enter a name for the game (without spaces, e.g. game1): ");
        scanf("%s", game_name);

        // باز کردن فایل در حالت "a" (Append).
        // در این حالت اطلاعات در انتهای فایل نوشته می‌شوند تا اطلاعات بازی‌های قبلی پاک نشود.
        FILE *db = fopen("database.txt", "a");
        if (db == NULL) {
            printf("Error creating the database file!\n");
            return;
        }

        // ذخیره متادیتا (اطلاعات پایه) بازی
        fprintf(db, "%s %d %d %d", game_name, board->rows, board->cols, board->move_count);
        // پیمایش آرایه تاریخچه و ذخیره ستون‌های انتخاب شده به ترتیب
        for (int i = 0; i < board->move_count; i++) {
            fprintf(db, " %d", board->move_history[i]);
        }
        fprintf(db, "\n"); // رفتن به خط جدید برای جلوگیری از تداخل با بازی‌هایی که در آینده ذخیره می‌شوند
        fclose(db); // آزادسازی منابع فایل
        printf("Game successfully saved in database.txt!\n");
    }
}

// تابع بازپخش (Replay) مرحله به مرحله یک بازی ذخیره شده
void replay_game() {
    char search_name[50], current_name[50];
    int r, c, count, move_col;
    int found = 0;

    // باز کردن فایل در حالت "r" (Read) برای خواندن اطلاعات
    FILE *db = fopen("database.txt", "r");
    if (db == NULL) {
        printf("No database found. Please save a game first.\n");
        return;
    }

    printf("Enter the name of the game you want to replay: ");
    scanf("%s", search_name);

    // اسکن فایل به صورت خط به خط برای پیدا کردن بازی.
    // تابع fscanf تا زمانی که ۴ متغیر اصلی (متادیتا) را با موفقیت در ابتدای خط بخواند، حلقه را ادامه می‌دهد.
    while (fscanf(db, "%s %d %d %d", current_name, &r, &c, &count) == 4) {
        if (strcmp(current_name, search_name) == 0) {
            found = 1; // بازی مورد نظر پیدا شد
            GameState *board = create_board(r, c); // ساخت یک زمین خالی بر اساس ابعاد ذخیره شده
            char current_token = 'X'; // همیشه اولین حرکت با مهره 'X' است

            printf("\n--- Starting Replay: %s ---\n", search_name);
            print_board(board);

            // حلقه بازپخش حرکات بازی
            for (int i = 0; i < count; i++) {
                fscanf(db, "%d", &move_col); // خواندن شماره ستون از فایل دیتابیس

                printf("Move %d: Player (%c) chose column %d.\n", i + 1, current_token, move_col);
                printf("(Press Enter to see the next move...)");

                // پاکسازی بافر ورودی تا مطمئن شویم فقط منتظر فشردن دکمه Enter جدید می‌ماند
                while(getchar() != '\n');
                getchar(); // توقف موقت برنامه منتظر ماندن برای فشردن Enter

                drop_piece(board, move_col, current_token);
                print_board(board);

                // جابجایی نوبت بازیکن برای حرکت بعدی در بازپخش
                current_token = (current_token == 'X') ? 'O' : 'X';
            }
            printf("--- End of Replay ---\n");

            // آزادسازی حافظه (Free) برای جلوگیری از نشت حافظه (Memory Leak)
            for (int i = 0; i < board->rows; i++) free(board->grid[i]);
            free(board->grid);
            free(board->move_history);
            free(board);
            break; // پس از یافتن و پخش بازی، از حلقه جستجو خارج می‌شویم
        } else {
            // اگر خطی که خواندیم متعلق به بازی موردنظر ما نبود،
            // باید اعداد باقیمانده‌ی آن خط (حرکات) را بخوانیم تا اشاره‌گر فایل به ابتدای خط بازی بعدی برسد
            for (int i = 0; i < count; i++) fscanf(db, "%d", &move_col);
        }
    }

    if (!found) printf("No game found with this name in the database.\n");
    fclose(db);
}

// ---------- 4. توابع حرکت بازیکن‌ها (Player Moves) ----------
// تمام این توابع یک قالب (Signature) یکسان دارند تا بتوانند به عنوان اشاره‌گر به تابع MoveFn استفاده شوند.

// تابع دریافت حرکت از کاربر انسانی از طریق کیبورد
int human_move(const GameState *st, void *ctx) {
    int selected_col;
    while (1) {
        // اطمینان از اینکه ورودی یک عدد صحیح است
        if (scanf("%d", &selected_col) != 1) {
            printf("Invalid input! Please enter a number: ");
            while(getchar() != '\n'); // پاکسازی کاراکترهای نامعتبر و حروف از بافر ورودی
            continue;
        }
        // بررسی اینکه آیا ستون انتخاب شده در بازه مجاز ابعاد زمین قرار دارد
        if (selected_col < 0 || selected_col >= st->cols) {
            printf("Error: Column out of range! (0 to %d): ", st->cols - 1);
            continue;
        }
        return selected_col;
    }
}

// تابع دریافت حرکت از طریق خواندن از یک فایل متنی مجزا (حالت Human vs File)
int file_move(const GameState *st, void *ctx) {
    FILE *file = (FILE*)ctx; // تبدیل (Cast) متغیر عمومی ctx به اشاره‌گر از نوع فایل
    int selected_col;

    // تلاش برای خواندن یک عدد از فایل
    if (fscanf(file, "%d", &selected_col) == 1) {
        printf("Move read from file: %d\n", selected_col);
        if (selected_col < 0 || selected_col >= st->cols) return -1; // اگر ستون خارج از محدوده بود متوقف می‌شود
        return selected_col;
    }
    return -1; // نشان‌دهنده رسیدن به انتهای فایل (EOF) یا بروز خطا در خواندن اطلاعات
}

// تابع تولید حرکت توسط هوش مصنوعی (کامپیوتر)
int computer_move(const GameState *st, void *ctx) {
    int level = *(int*)ctx; // بازیابی سطح سختی بازی با Cast کردن اشاره‌گر ctx به int
    char ai_token = 'O';    // فرض بر این است که کامپیوتر همیشه بازیکن دوم است
    char human_token = 'X'; // مهره رقیب انسانی

    if (level == 2) { // منطق حالت سخت (Hard Mode)
        // اولویت اول (حمله): آیا کامپیوتر می‌تواند با انتخاب یکی از ستون‌ها در همین حرکت برنده شود؟
        for (int c = 0; c < st->cols; c++) {
            if (can_drop(st, c)) {
                int r;
                // شبیه‌سازی حرکت: یافتن پایین‌ترین خانه در ستون
                for (r = st->rows - 1; r >= 0; r--) if (st->grid[r][c] == '.') break;

                st->grid[r][c] = ai_token; // قرار دادن موقت مهره در خانه
                if (check_win((GameState*)st, ai_token)) {
                    st->grid[r][c] = '.';  // بازگردانی وضعیت به حالت قبل (لغو شبیه‌سازی)
                    printf("Computer made a smart move (column %d)\n", c);
                    return c; // انتخاب این ستون برای پیروزی قطعی
                }
                st->grid[r][c] = '.'; // بازگردانی وضعیت در صورت عدم برد
            }
        }

        // اولویت دوم (دفاع): اگر هوش مصنوعی حرکت برنده‌ای نداشت، بررسی می‌کند که آیا بازیکن انسانی
        // در حرکت بعدی می‌تواند برنده شود؟ اگر بله، آن خانه را پر می‌کند تا جلوی برد حریف را بگیرد.
        for (int c = 0; c < st->cols; c++) {
            if (can_drop(st, c)) {
                int r;
                for (r = st->rows - 1; r >= 0; r--) if (st->grid[r][c] == '.') break;

                st->grid[r][c] = human_token; // شبیه‌سازی حرکت برای حریف
                if (check_win((GameState*)st, human_token)) {
                    st->grid[r][c] = '.';
                    printf("Computer defended (column %d)\n", c);
                    return c; // مسدود کردن مسیر برد حریف
                }
                st->grid[r][c] = '.';
            }
        }
    }

    // منطق حالت آسان (Easy Mode) یا عدم وجود موقعیت بحرانی (دفاع/حمله) در حالت سخت:
    // یک ستون تصادفی (Random) را انتخاب می‌کند که هنوز پر نشده است.
    int col;
    do { col = rand() % st->cols; } while (!can_drop(st, col));
    printf("Computer chose column %d.\n", col);
    return col;
}

// ---------- 5. تابع اصلی (Main) ----------

int main() {
    // مقداردهی اولیه به هسته (Seed) تولیدکننده اعداد تصادفی با استفاده از زمان فعلی سیستم
    // تا الگوهای تصادفی در هر بار اجرای برنامه متفاوت باشند.
    srand(time(NULL));

    int mode;
    // نمایش منوی اصلی و دریافت مود بازی از کاربر
    printf("=== Main Menu ===\n");
    printf("1. New Game (Human vs Human)\n");
    printf("2. New Game (Human vs File)\n");
    printf("3. New Game (Human vs Computer)\n");
    printf("4. Replay a saved game\n");
    printf("Your choice: ");

    if (scanf("%d", &mode) != 1 || mode < 1 || mode > 4) {
        printf("Invalid choice! Exiting.\n");
        return 1;
    }

    // اگر کاربر درخواست بازپخش کرد، فرآیند را به تابع مربوطه سپرده و سپس برنامه را خاتمه می‌دهیم
    if (mode == 4) {
        replay_game();
        return 0;
    }

    int r, c;
    // یک حلقه نامحدود برای دریافت ابعاد استاندارد زمین از کاربر
    while (1) {
        printf("Enter the number of rows (minimum 4) and columns (less than 12): ");
        if (scanf("%d %d", &r, &c) != 2) {
            printf("Invalid input!\n");
            while(getchar() != '\n'); // پاکسازی بافر
            continue;
        }
        if (r >= 4 && c < 12 && c > 0) break; // شرایط صحیح ابعاد
        else printf("Error: Dimensions do not match the rules.\n");
    }

    // ساخت وضعیت اولیه زمین بازی و چاپ آن
    GameState *board = create_board(r, c);
    print_board(board);

    // تعریف اطلاعات دو بازیکن با مقادیر پیش‌فرض (بازی دو نفره انسانی)
    Player p1 = { human_move, NULL, 'X' };
    Player p2 = { human_move, NULL, 'O' };
    FILE *inputFile = NULL;
    int ai_level = 1;

    // شخصی‌سازی رفتار بازیکن دوم بر اساس حالت بازی انتخابی
    if (mode == 2) { // حالت بازی با فایل حرکات
        char filename[100];
        printf("Enter the moves file name: ");
        scanf("%s", filename);
        inputFile = fopen(filename, "r");

        if (inputFile != NULL) {
            // جایگزینی تابع حرکت با file_move و پاس دادن فایل به ctx
            p2.move = file_move;
            p2.ctx = inputFile;
        } else {
            // در صورتی که فایل یافت نشد، برنامه به جای کرش کردن به حالت انسان مقابل انسان بازمی‌گردد
            printf("Error! File not found. Proceeding with a two-player game.\n");
        }

    } else if (mode == 3) { // حالت بازی با هوش مصنوعی کامپیوتر
        printf("Computer difficulty level (1: Easy, 2: Hard): ");
        scanf("%d", &ai_level);

        // جایگزینی تابع حرکت با computer_move و پاس دادن آدرس متغیر سطح سختی به ctx
        p2.move = computer_move;
        p2.ctx = &ai_level;
    }

    // ساخت یک آرایه از اشاره‌گرها به بازیکنان جهت ساده‌سازی مدیریت نوبت در حلقه بازی
    Player *players[2] = { &p1, &p2 };
    int current_player_idx = 0; // متغیر ایندکس: 0 نشان‌دهنده نوبت p1 و 1 نشان‌دهنده نوبت p2
    int turn = 0; // شمارشگر مجموع حرکات بازی تا این لحظه
    int max_turns = r * c; // شرط پایان در حالت تساوی (پر شدن کامل جدول)

    // حلقه اصلی (Main Game Loop)
    while (turn < max_turns) {
        Player *current_player = players[current_player_idx]; // اشاره‌گر به بازیکن فعلی

        // صرفاً زمانی که بازیکن از نوع انسانی است، اعلان نوبت چاپ می‌شود تا خروجی گرافیکی نامنظم نشود
        if (mode == 1 || current_player_idx == 0) {
            printf("Player (%c)'s turn: ", current_player->token);
        }

        // فراخوانی تابع حرکت بازیکنِ فعلی بصورت داینامیک توسط اشاره‌گر (Polymorphism ساده در زبان C)
        int selected_col = current_player->move(board, current_player->ctx);

        // اگر تابع حرکت مقدار منفی بازگرداند (معمولاً بخاطر پایان یافتن خطوط درون فایل ورودی)، بازی پایان می‌یابد
        if (selected_col == -1) break;

        // اقدام به انداختن مهره داخل ستون انتخاب شده
        if (drop_piece(board, selected_col, current_player->token)) {

            // ثبت حرکت بازیکن در آرایه پویا جهت استفاده در سیستم Save/Replay
            board->move_history[board->move_count++] = selected_col;

            print_board(board); // تازه‌سازی صفحه و نمایش زمین جدید

            // پس از هر حرکت موفق باید فوراً بررسی کنیم که آیا این حرکت منجر به پیروزی شده است؟
            if (check_win(board, current_player->token)) {
                printf("\n*** Player (%c) wins! ***\n", current_player->token);
                break; // شکستن حلقه و پایان بازی
            }

            turn++;
            if (turn == max_turns) { // رسیدن به بن‌بست و پر شدن جدول
                printf("\n*** The game is a draw! ***\n");
                break;
            }

            // سوییچ کردن نوبت بازیکن بعدی (اگر 0 باشد تبدیل به 1 می‌شود و بالعکس)
            current_player_idx = 1 - current_player_idx;
        } else {
            // هندلینگ خطا: در صورتی که ستون انتخاب شده کاملاً پر باشد
            if (current_player->move == human_move) {
                printf("Error: This column is full!\n");
            }
        }
    }

    // فراخوانی تابع برای پیشنهاد ذخیره‌سازی این بازی در فایل دیتابیس
    save_game(board);

    // فرآیند پایانی (Clean-up): بستن دسترسی فایل‌ها و آزادسازی حافظه‌های تخصیص یافته روی Heap
    // تا از Memory Leak در سیستم‌عامل جلوگیری شود.
    if (inputFile != NULL) fclose(inputFile);

    for (int i = 0; i < board->rows; i++) free(board->grid[i]);
    free(board->grid);
    free(board->move_history);
    free(board);

    return 0; // خروج موفقیت‌آمیز از برنامه
}
