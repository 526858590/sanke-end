#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

// 常量定义
#define MAP_WIDTH 80
#define MAP_HEIGHT 25
#define INIT_SNAKE_LENGTH 5
#define MAX_SNAKE_LENGTH 100
#define MAX_FOOD 5
#define MIN_FOOD 1
#define FOOD_INTERVAL_MIN 3000
#define FOOD_INTERVAL_MAX 8000

// 方向定义
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

// 颜色定义
#define COLOR_SNAKE 14  // 黄色
#define COLOR_FOOD_NORMAL 12   // 红色
#define COLOR_FOOD_GOLD 14     // 金色
#define COLOR_FOOD_BLUE 9      // 蓝色
#define COLOR_WALL 15   // 白色
#define COLOR_TEXT 10   // 绿色

// 食物类型
#define FOOD_TYPE_NORMAL 0
#define FOOD_TYPE_GOLD 1
#define FOOD_TYPE_BLUE 2

// 位置结构体
typedef struct {
    int x;
    int y;
    int active;
    int type;  // 食物类型
    int score; // 食物得分
} Position;

// 蛇结构体
typedef struct {
    Position pos[MAX_SNAKE_LENGTH];
    int length;
    int direction;
    int speed;
    int score;
    int is_accelerated;
    int base_score;
    int move_count;  // 移动计数，用于动画
} Snake;

// 游戏统计结构体
typedef struct {
    int total_score;
    int games_played;
    int highest_score;
    int total_food_eaten;
    int total_gold_food;
    int total_blue_food;
    int total_play_time;  // 总游戏时间（秒）
} GameStats;

// 全局变量
Snake snake;
Position foods[MAX_FOOD];
int foodCount = 0;
int highScore = 0;
int sleepTime = 200;
DWORD lastFoodTime = 0;
DWORD nextFoodInterval;
GameStats gameStats;
DWORD gameStartTime;
int lastKeyPressTime = 0;
const int KEY_REPEAT_DELAY = 50;  // 按键重复延迟（毫秒）

// 函数声明
void initConsole(void);
void gotoxy(int x, int y);
void setColor(int color);
void printsnake(void);
void welcometogame(void);
void createMap(void);
void scoreandtips(void);
void File_out(void);
void initsnake(void);
void createfood(void);
int biteself(void);
void cantcrosswall(void);
void speedup(void);
void speeddown(void);
void snakemove(void);
void keyboardControl(void);
void gameOver(void);
void explation(void);
void saveScore(int score);
void createSingleFood(void);
void initFoodSystem(void);
void updateFoodSystem(void);
void initGameStats(void);
void updateGameStats(void);
void showGameStats(void);

// 初始化控制台
void initConsole(void) {
    system("chcp 936");
    system("mode con cols=120 lines=35");
    system("title 贪吃蛇大作战");
}

// 光标定位
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 设置颜色
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// 初始化游戏统计
void initGameStats(void) {
    gameStats.total_score = 0;
    gameStats.games_played = 0;
    gameStats.highest_score = 0;
    gameStats.total_food_eaten = 0;
    gameStats.total_gold_food = 0;
    gameStats.total_blue_food = 0;
    gameStats.total_play_time = 0;
    gameStartTime = GetTickCount();
}

// 更新游戏统计
void updateGameStats(void) {
    DWORD currentTime = GetTickCount();
    gameStats.total_play_time = (currentTime - gameStartTime) / 1000;
}

// 显示游戏统计
void showGameStats(void) {
    gotoxy(MAP_WIDTH + 5, 20);
    printf("┌────────────────────┐");
    gotoxy(MAP_WIDTH + 5, 21);
    printf("│     游戏统计       │");
    gotoxy(MAP_WIDTH + 5, 22);
    printf("├────────────────────┤");
    gotoxy(MAP_WIDTH + 5, 23);
    printf("│ 总得分: %-10d │", gameStats.total_score);
    gotoxy(MAP_WIDTH + 5, 24);
    printf("│ 游戏次数: %-8d │", gameStats.games_played);
    gotoxy(MAP_WIDTH + 5, 25);
    printf("│ 最高分: %-10d │", gameStats.highest_score);
    gotoxy(MAP_WIDTH + 5, 26);
    printf("│ 总食物: %-10d │", gameStats.total_food_eaten);
    gotoxy(MAP_WIDTH + 5, 27);
    printf("│ 金色食物: %-8d │", gameStats.total_gold_food);
    gotoxy(MAP_WIDTH + 5, 28);
    printf("│ 蓝色食物: %-8d │", gameStats.total_blue_food);
    gotoxy(MAP_WIDTH + 5, 29);
    printf("│ 游戏时间: %-8d秒 │", gameStats.total_play_time);
    gotoxy(MAP_WIDTH + 5, 30);
    printf("└────────────────────┘");
}

// 打印蛇标题
void printsnake(void) {
    system("cls");
    setColor(COLOR_SNAKE);
    printf("\n\n");
    printf("                  贪  吃  蛇  大  作  战\n");
    printf("                ========================\n");
    printf("                    Powered by C++\n");
    printf("\n\n");
    setColor(COLOR_WALL);
    printf("                  ┌──────────────────┐\n");
    printf("                  │    1. 开始游戏   │\n");
    printf("                  │    2. 游戏说明   │\n");
    printf("                  │    3. 退出游戏   │\n");
    printf("                  └──────────────────┘\n");
}

// 欢迎界面
void welcometogame(void) {
    char choice;
    while (1) {
        printsnake();
        choice = _getch();
        switch (choice) {
        case '1':
            system("cls");
            createMap();
            initsnake();
            initFoodSystem();
            keyboardControl();
            break;
        case '2':
            explation();
            break;
        case '3':
            exit(0);
        default:
            continue;
        }
    }
}

// 创建地图
void createMap(void) {
    int i, j;
    setColor(COLOR_WALL);  // 使用固定的白色

    // 绘制上边框
    gotoxy(0, 0);
    printf("┌");
    for (i = 0; i < MAP_WIDTH - 2; i++)
        printf("─");
    printf("┐");

    // 绘制中间部分
    for (i = 1; i < MAP_HEIGHT - 1; i++) {
        gotoxy(0, i);
        printf("│");
        for (j = 0; j < MAP_WIDTH - 2; j++)
            printf(" ");
        printf("│");
    }

    // 绘制下边框
    gotoxy(0, MAP_HEIGHT - 1);
    printf("└");
    for (i = 0; i < MAP_WIDTH - 2; i++)
        printf("─");
    printf("┘");
}

// 显示分数和提示
void scoreandtips(void) {
    gotoxy(MAP_WIDTH + 5, 3);
    printf("┌────────────────────┐");
    gotoxy(MAP_WIDTH + 5, 4);
    printf("│     游戏信息       │");
    gotoxy(MAP_WIDTH + 5, 5);
    printf("├────────────────────┤");
    gotoxy(MAP_WIDTH + 5, 6);
    printf("│ 当前得分: %-8d │", snake.score);
    gotoxy(MAP_WIDTH + 5, 7);
    printf("│ 历史最高: %-8d │", highScore);
    gotoxy(MAP_WIDTH + 5, 8);
    printf("├────────────────────┤");
    gotoxy(MAP_WIDTH + 5, 9);
    printf("│     操作说明       │");
    gotoxy(MAP_WIDTH + 5, 10);
    printf("│ ↑↓←→: 移动方向     │");
    gotoxy(MAP_WIDTH + 5, 11);
    printf("│ F1: 加速           │");
    gotoxy(MAP_WIDTH + 5, 12);
    printf("│ F2: 减速           │");
    gotoxy(MAP_WIDTH + 5, 13);
    printf("│ ESC: 退出          │");
    gotoxy(MAP_WIDTH + 5, 14);
    printf("├────────────────────┤");
    gotoxy(MAP_WIDTH + 5, 15);
    printf("│     游戏状态       │");
    gotoxy(MAP_WIDTH + 5, 16);
    printf("│ 蛇的长度: %-8d │", snake.length);
    gotoxy(MAP_WIDTH + 5, 17);
    printf("│ 当前速度: %-8d │", (400 - sleepTime) / 10);
    gotoxy(MAP_WIDTH + 5, 18);
    printf("└────────────────────┘");
}

// 读取最高分
void File_out(void) {
    FILE* fp = fopen("save.txt", "r");
    if (fp) {
        fscanf(fp, "%d", &highScore);
        fclose(fp);
    }
}

// 初始化蛇
void initsnake(void) {
    int i;
    snake.length = INIT_SNAKE_LENGTH;
    snake.direction = RIGHT;
    snake.speed = 1;
    snake.score = 0;
    snake.is_accelerated = 0;
    snake.base_score = 10;
    snake.move_count = 0;

    for (i = 0; i < snake.length; i++) {
        snake.pos[i].x = MAP_WIDTH / 3 - i * 2;
        snake.pos[i].y = MAP_HEIGHT / 2;
        gotoxy(snake.pos[i].x, snake.pos[i].y);
        setColor(COLOR_SNAKE);
        if (i == 0) {
            printf("★");
        }
        else {
            printf("○");
        }
    }
}

// 创建单个食物
void createSingleFood(void) {
    int i, flag;
    Position newFood;

    do {
        flag = 0;
        newFood.x = (rand() % ((MAP_WIDTH - 4) / 2)) * 2 + 2;
        newFood.y = rand() % (MAP_HEIGHT - 2) + 1;

        // 检查是否与蛇身重叠
        for (i = 0; i < snake.length; i++) {
            if (newFood.x == snake.pos[i].x && newFood.y == snake.pos[i].y) {
                flag = 1;
                break;
            }
        }

        // 检查是否与其他食物重叠
        for (i = 0; i < MAX_FOOD; i++) {
            if (foods[i].active && newFood.x == foods[i].x && newFood.y == foods[i].y) {
                flag = 1;
                break;
            }
        }
    } while (flag);

    // 随机决定食物类型
    int foodType = rand() % 100;
    if (foodType < 70) {  // 70%概率普通食物
        newFood.type = FOOD_TYPE_NORMAL;
        newFood.score = 10;
    }
    else if (foodType < 90) {  // 20%概率金色食物
        newFood.type = FOOD_TYPE_GOLD;
        newFood.score = 30;
    }
    else {  // 10%概率蓝色食物
        newFood.type = FOOD_TYPE_BLUE;
        newFood.score = 20;
    }

    for (i = 0; i < MAX_FOOD; i++) {
        if (!foods[i].active) {
            foods[i] = newFood;
            foods[i].active = 1;
            foodCount++;

            gotoxy(foods[i].x, foods[i].y);
            switch (foods[i].type) {
            case FOOD_TYPE_NORMAL:
                setColor(COLOR_FOOD_NORMAL);
                printf("●");
                break;
            case FOOD_TYPE_GOLD:
                setColor(COLOR_FOOD_GOLD);
                printf("★");
                break;
            case FOOD_TYPE_BLUE:
                setColor(COLOR_FOOD_BLUE);
                printf("◆");
                break;
            }
            break;
        }
    }
}

// 初始化食物系统
void initFoodSystem(void) {
    int i;
    foodCount = 0;
    for (i = 0; i < MAX_FOOD; i++) {
        foods[i].active = 0;
    }
    lastFoodTime = GetTickCount();
    nextFoodInterval = rand() % (FOOD_INTERVAL_MAX - FOOD_INTERVAL_MIN + 1) + FOOD_INTERVAL_MIN;
    createSingleFood();
}

// 更新食物系统
void updateFoodSystem(void) {
    DWORD currentTime = GetTickCount();
    if (currentTime - lastFoodTime >= nextFoodInterval) {
        if (foodCount < MAX_FOOD) {
            createSingleFood();
        }
        lastFoodTime = currentTime;
        nextFoodInterval = rand() % (FOOD_INTERVAL_MAX - FOOD_INTERVAL_MIN + 1) + FOOD_INTERVAL_MIN;
    }
}

// 创建食物
void createfood(void) {
    if (foodCount < MIN_FOOD) {
        createSingleFood();
    }
}

// 检测是否咬到自己
int biteself(void) {
    int i;
    for (i = 1; i < snake.length; i++) {
        if (snake.pos[0].x == snake.pos[i].x && snake.pos[0].y == snake.pos[i].y)
            return 1;
    }
    return 0;
}

// 检测是否撞墙
void cantcrosswall(void) {
    if (snake.pos[0].x <= 0 || snake.pos[0].x >= MAP_WIDTH - 1 ||
        snake.pos[0].y <= 0 || snake.pos[0].y >= MAP_HEIGHT - 1) {
        gameOver();
    }
}

// 加速
void speedup(void) {
    if (sleepTime >= 100) {
        sleepTime -= 10;
        snake.is_accelerated = 1;
        if (snake.base_score < 20) {
            snake.base_score += 1;
        }
    }
}

// 减速
void speeddown(void) {
    if (sleepTime <= 320) {
        sleepTime += 30;
        snake.is_accelerated = 0;
        if (snake.base_score > 1) {
            snake.base_score -= 1;
        }
    }
}

// 蛇的移动
void snakemove(void) {
    int i;
    // 清除蛇尾
    gotoxy(snake.pos[snake.length - 1].x, snake.pos[snake.length - 1].y);
    printf(" ");

    // 更新蛇身位置
    for (i = snake.length - 1; i > 0; i--) {
        snake.pos[i] = snake.pos[i - 1];
    }

    // 根据方向移动蛇头
    switch (snake.direction) {
    case UP:
        snake.pos[0].y--;
        break;
    case DOWN:
        snake.pos[0].y++;
        break;
    case LEFT:
        snake.pos[0].x -= 2;
        break;
    case RIGHT:
        snake.pos[0].x += 2;
        break;
    }

    // 绘制蛇身（带动画效果）
    for (i = 0; i < snake.length; i++) {
        gotoxy(snake.pos[i].x, snake.pos[i].y);
        setColor(COLOR_SNAKE);
        if (i == 0) {
            // 蛇头动画
            if (snake.move_count % 2 == 0) {
                printf("★");
            }
            else {
                printf("☆");
            }
        }
        else {
            // 蛇身动画
            if ((snake.move_count + i) % 2 == 0) {
                printf("○");
            }
            else {
                printf("◎");
            }
        }
    }
    snake.move_count++;

    // 检查是否吃到食物
    for (i = 0; i < MAX_FOOD; i++) {
        if (foods[i].active && snake.pos[0].x == foods[i].x && snake.pos[0].y == foods[i].y) {
            snake.length++;
            snake.score += foods[i].score;
            gameStats.total_score += foods[i].score;
            gameStats.total_food_eaten++;

            // 更新食物统计
            switch (foods[i].type) {
            case FOOD_TYPE_GOLD:
                gameStats.total_gold_food++;
                break;
            case FOOD_TYPE_BLUE:
                gameStats.total_blue_food++;
                break;
            }

            foods[i].active = 0;
            foodCount--;
            if (foodCount < MIN_FOOD) {
                createfood();
            }
        }
    }

    updateFoodSystem();
    updateGameStats();
    showGameStats();

    // 优化碰撞检测
    if (snake.pos[0].x <= 0 || snake.pos[0].x >= MAP_WIDTH - 1 ||
        snake.pos[0].y <= 0 || snake.pos[0].y >= MAP_HEIGHT - 1) {
        gameOver();
    }

    if (biteself()) {
        gameOver();
    }

    scoreandtips();
    Sleep(sleepTime);
}

// 键盘控制
void keyboardControl(void) {
    DWORD lastKeyTime = 0;
    while (1) {
        DWORD currentTime = GetTickCount();

        // 检查按键状态
        if (currentTime - lastKeyTime >= KEY_REPEAT_DELAY) {
            if (GetAsyncKeyState(VK_UP) && snake.direction != DOWN) {
                snake.direction = UP;
                lastKeyTime = currentTime;
            }
            else if (GetAsyncKeyState(VK_DOWN) && snake.direction != UP) {
                snake.direction = DOWN;
                lastKeyTime = currentTime;
            }
            else if (GetAsyncKeyState(VK_LEFT) && snake.direction != RIGHT) {
                snake.direction = LEFT;
                lastKeyTime = currentTime;
            }
            else if (GetAsyncKeyState(VK_RIGHT) && snake.direction != LEFT) {
                snake.direction = RIGHT;
                lastKeyTime = currentTime;
            }
            else if (GetAsyncKeyState(VK_F1)) {
                speedup();
                lastKeyTime = currentTime;
            }
            else if (GetAsyncKeyState(VK_F2)) {
                speeddown();
                lastKeyTime = currentTime;
            }
            else if (GetAsyncKeyState(VK_ESCAPE)) {
                gameOver();
            }
        }

        snakemove();
    }
}

// 游戏结束
void gameOver(void) {
    system("cls");
    setColor(COLOR_TEXT);
    gotoxy(MAP_WIDTH / 3, MAP_HEIGHT / 2);

    // 更新游戏统计
    gameStats.games_played++;
    if (snake.score > gameStats.highest_score) {
        gameStats.highest_score = snake.score;
    }

    if (snake.score > highScore) {
        printf("恭喜你创造了新记录！得分：%d", snake.score);
        saveScore(snake.score);
    }
    else {
        printf("游戏结束！得分：%d（最高记录：%d）", snake.score, highScore);
    }

    // 显示本局游戏统计
    gotoxy(MAP_WIDTH / 3, MAP_HEIGHT / 2 + 2);
    printf("本局游戏时间：%d秒", gameStats.total_play_time);
    gotoxy(MAP_WIDTH / 3, MAP_HEIGHT / 2 + 3);
    printf("总游戏次数：%d", gameStats.games_played);
    gotoxy(MAP_WIDTH / 3, MAP_HEIGHT / 2 + 4);
    printf("总得分：%d", gameStats.total_score);

    gotoxy(MAP_WIDTH / 3, MAP_HEIGHT / 2 + 6);
    printf("1. 再玩一次");
    gotoxy(MAP_WIDTH / 3, MAP_HEIGHT / 2 + 7);
    printf("2. 退出游戏");

    while (1) {
        char choice = _getch();
        if (choice == '1') {
            system("cls");
            initGameStats();  // 重置游戏统计
            welcometogame();
        }
        else if (choice == '2') {
            exit(0);
        }
    }
}

// 游戏说明
void explation(void) {
    system("cls");
    setColor(COLOR_WALL);  // 使用固定的白色边框
    printf("\n\n");
    printf("  ┌──────────────────────────────────────┐\n");
    printf("  │          贪吃蛇游戏说明              │\n");
    printf("  ├──────────────────────────────────────┤\n");
    printf("  │                                      │\n");
    printf("  │    1. 使用方向键 ↑↓←→ 控制蛇的移动   │\n");
    printf("  │    2. 食物类型：                     │\n");
    setColor(COLOR_FOOD_NORMAL);
    printf("  │       ● 红色：10分                   │\n");
    setColor(COLOR_FOOD_GOLD);
    printf("  │       ★ 金色：30分                   │\n");
    setColor(COLOR_FOOD_BLUE);
    printf("  │       ◆ 蓝色：20分                   │\n");
    setColor(COLOR_WALL);  // 恢复白色边框
    printf("  │    3. 撞到墙壁或咬到自己会游戏结束   │\n");
    printf("  │    4. 按F1加速：每次得分加1分        │\n");
    printf("  │    5. 按F2减速：每次得分减1分        │\n");
    printf("  │    6. 得分范围：最低1分最高20分      │\n");
    printf("  │    7. 按ESC键可以随时退出游戏        │\n");
    printf("  │                                      │\n");
    printf("  ├──────────────────────────────────────┤\n");
    printf("  │        按任意键返回主菜单...         │\n");
    printf("  └──────────────────────────────────────┘\n");

    while (_kbhit()) {
        _getch();
    }
    _getch();
    while (_kbhit()) {
        _getch();
    }
    system("cls");
}

// 保存最高分
void saveScore(int score) {
    FILE* fp = fopen("save.txt", "w");
    if (fp) {
        fprintf(fp, "%d", score);
        fclose(fp);
        highScore = score;
    }
}

// 主函数
int main() {
    srand((unsigned)time(NULL));
    initConsole();
    File_out();
    initGameStats();  // 初始化游戏统计
    welcometogame();
    return 0;
}