// #ifndef AF280C4C_5BDF_4E30_B1EA_6C5FA5F9E504
// #define AF280C4C_5BDF_4E30_B1EA_6C5FA5F9E504

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define SUCCESS_NUM 2048

int * plcd; // 指向LCD的指针
// char *bmp_name[] = {
// 	"2.bmp",
// 	"4.bmp",
// 	"8.bmp",
// 	"16.bmp",
// 	"32.bmp",
// 	"64.bmp",
// 	"128.bmp",
// 	"256.bmp",
// 	"512.bmp",
// 	"1024.bmp",
// 	"2048.bmp",
// };
// int matrix[4][4] = {//初始化矩阵
// 	0,0,0,0,
// 	0,0,0,0,
// 	0,0,0,0,
// 	0,0,0,0,
// };

int get_rand_int_1();
int get_rand_int_2();
int get_rand_int_3();
int get_rand_int_4();
void init_matrix();
void draw_point(int x,int y,int color);
void lcd_draw_rect(int x0, int y0, int w, int h, int color);
void lcd_draw_bmp(char *file, int x, int y);
char *get_filename_by_digit(int digit);
void lcd_draw_matrix();
int get_finger_move_direction();
void handle_up();
void handle_down();
void handle_left();
void handle_right();
void rand_digital();
int False();
int Success();
void pfalse();
void psuccess();
void My_2048();
// int m2048(int argc, char *argv[]);



// #endif /* AF280C4C_5BDF_4E30_B1EA_6C5FA5F9E504 */
