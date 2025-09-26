#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <yx1.h>
// #include <trainB.h>



char *bmp_name[] = {
	"2.bmp",
	"4.bmp",
	"8.bmp",
	"16.bmp",
	"32.bmp",
	"64.bmp",
	"128.bmp",
	"256.bmp",
	"512.bmp",
	"1024.bmp",
	"2048.bmp",
};
int matrix[4][4] = {//初始化矩阵
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
};

int get_rand_int_1(){//随机生成1或2
	int k = 0;
	int a;
    srand((unsigned)time(NULL));//随机数种子
    a = rand();
    return a;
}


int get_rand_int_2(){//随机生成1-4
	int k = 0;
	int a;
    srand((unsigned) (time(NULL) + 1));
    a = rand();
    // printf("%d\n", a);
    return a;
}

int get_rand_int_3(){//随机生成1-4
	int k = 0;
	int a;
    srand((unsigned) (time(NULL) + 2));
    a = rand();
    // printf("%d\n", a);
    return a;
}

int get_rand_int_4(){//随机生成1-4
	int k = 0;
	int a;
    srand((unsigned) (time(NULL) + 3));
    a = rand();
    // printf("%d\n", a);
    return a;
}


// 初始化二维数组随机值
void init_matrix(){			//初始化矩阵
	int a = get_rand_int_1();//随机生成1或2
	int b = get_rand_int_2();//随机生成1-4
	// printf("%d %d \n",a, b );
    int rand1_x = a % 4;
    int rand1_y = b % 4;
    // printf("%d %d\n",rand1_x, rand1_y );
    int c = get_rand_int_2();
	int d = get_rand_int_4();
    int rand2_x = c % 4;
    int rand2_y = d % 4;
    // 防止生成两个相同位置的块
    if(rand1_x == rand2_x && rand1_y == rand2_y){
    	rand2_x = (rand2_x + 1) % 4;	//右移一位，对四取余
    }
    // printf("%d %d\n",rand2_x, rand2_y );
    matrix[rand1_x][rand1_y] = 2;
    matrix[rand2_x][rand2_y] = 4;
}


void draw_point(int x,int y,int color){
	if(x>=0 && x<800 && y>=0 && y<480 ){
		*(plcd+800*y+x) = color ;
	}
}


//实现一个函数，在坐标为x0，y0的位置显示一个矩形，宽度为w，高度为h，颜色为color
void lcd_draw_rect(int x0, int y0, int w, int h, int color){
	int x, y;
	for (y = y0; y < y0+h; y++)
	{
		for (x = x0; x < x0+w; x++)
			draw_point(x, y, color);
	}
}


void lcd_draw_bmp(char *file, int x, int y){
	int photo_file = open(file, O_RDONLY);

	if(photo_file == -1){
		perror("open error\n");
		return ;
	}
	int width, height;
	short depth;
	unsigned char buf[4] ;
	//读取宽度
	lseek(photo_file, 0x12, SEEK_SET);
	read(photo_file, buf, 4);
	width = buf[3]<<24 | buf[2]<<16 | buf[1]<<8 | buf[0];
	//读取高度
	read(photo_file, buf, 4);
	height  = buf[3]<<24 | buf[2]<<16 | buf[1]<<8 | buf[0];
	//读取色深
	lseek(photo_file, 0x1c, SEEK_SET);
	read(photo_file, buf, 2);
	depth = buf[1]<<8 | buf[0];
	//打印信息
	// printf("width=%d | height=%d | depth=%d \n", width, height, depth);

	//像素数组 
	int line_valid_bytes = abs(width) * depth / 8 ; //一行本有的有效字节
	int laizi = 0;//填充字节
	if( (line_valid_bytes % 4) !=0   ){
		laizi = 4 - line_valid_bytes % 4;
	}
	int line_bytes = line_valid_bytes + laizi;//一行所有的字节数
	
	int total_bytes = line_bytes * abs(height); //整个像素数组的大小
	
	unsigned char * photo_contents = malloc(total_bytes);
	
	lseek(photo_file, 54, SEEK_SET);
	read(photo_file, photo_contents, total_bytes);
	
	//调用draw_point 函数 。
	unsigned char a, r, g, b;
	int i = 0;//用来做指针运动的
	int x_0 = 0, y_0 = 0; //用来循环计数
	int color;
	for(y_0=0; y_0<abs(height); y_0++){	//画满每一列
		for(x_0=0; x_0<abs(width); x_0++){	
			//画满每一行
			//现在开始一个字节一个字节写入颜色
			// i++  先用后加     
			// ++i  先加后用
			b = photo_contents[i++];
			g = photo_contents[i++];
			r = photo_contents[i++];
			if(depth == 32)
			{
				a = photo_contents[i++];
			}
			if(depth == 24)
			{
				a = 0;
			}
			color = a<<24 | r<<16 | g<<8 | b ;
			draw_point(width>0 ? x+x_0 : abs(width)+x-1-x_0, 
				height>0 ? y+height-1-y_0 : y+y_0, color);
		}
		i = i + laizi ;//每一行后面的癞子数 跳过去。	
	}
	free(photo_contents);
	close(photo_file);
}



char *get_filename_by_digit(int digit){
	int index;
	switch (digit)
	{
		case 2:
			index = 0;
			break;
		case 4:
			index = 1;
			break;
		case 8:
			index = 2;
			break;
		case 16:
			index = 3;
			break;
		case 32:
			index = 4;
			break;
		case 64:
			index = 5;
			break;
		case 128:
			index = 6;
			break;
		case 256:
			index = 7;
			break;
		case 512:
			index = 8;
			break;
		case 1024:
			index = 9;
			break;
		case 2048:
			index = 10;
			break;
		default:
			index = 0;
			break;
	}
	// printf("%s\n", bmp_name[index]);x
	return bmp_name[index];
}

void lcd_draw_matrix(){
	int i, j;
	
	for (i = 0 ; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			if (matrix[i][j] == 0)
			{
				//画一个颜色矩形
			//	void lcd_draw_rect(int x0, int y0, int //w, int h, int color)
				lcd_draw_rect(20+ 120*j, 20+120*i,100, 100, 0x16a08500);
			}
			else
			{
				//画一张对应数字的bmp图片
				//	void lcd_draw_bmp(char *file, int x0, //int y0)
				lcd_draw_bmp(get_filename_by_digit(matrix[i][j]),20+ 120*j, 20+120*i);
			}
		}
	}
}


//获得手指的滑动方向
int get_finger_move_direction()
{

	int direction;
	int fd = open("/dev/input/event0", O_RDONLY);

	if(fd == -1){
		perror("/dev/input/event0 open error\n");
		return -1;
	}

	int x,y;
	int x_start, y_start, x_stop, y_stop;
	struct input_event ev ;

	while(1){
		int r = read(fd,&ev,sizeof(ev));
		if(r != sizeof(ev)){
			perror("read ev error\n");
			return -2;
		}
		if(ev.type == EV_ABS  && ev.code == ABS_X){
			x= ev.value ;
		}
		if(ev.type == EV_ABS  && ev.code == ABS_Y){
			y = ev.value;
		}

		if(ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value != 0 ){
			printf("FINGTER DOWN(%d ,%d )\n",x,y);
			x_start = x;
			y_start = y;
		}
		//弹起来
		if(ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0 ){
			printf("FINGTER UP(%d ,%d )\n",x,y);
			x_stop = x;
			y_stop = y ;
			if(y > 0)	break;
		}

	}

	if(abs(x_start - x_stop)  - abs(y_start - y_stop)  > 0 && abs(x_start - x_stop) > 30){	//X方向滑动
		if(x_stop  -  x_start  > 0){
			printf("SLIP RIGHT\n\n");
			direction = RIGHT;
		}
		else{
			printf("SLIP LEFT\n\n");
			direction = LEFT;
		}
	}

	if(abs(x_start - x_stop)  - abs(y_start - y_stop)  < 0 && abs(y_start - y_stop) > 30){	//y方向滑动
		 if(y_stop - y_start >0){
		 	printf("SLIP DOWN\n\n");
		 	direction = DOWN;
		 }
		 else{
		 	printf("SLIP UP\n\n");
			direction = UP;
		 }
	}

	close(fd);
	return direction;
}


void handle_up(){
	int i, j, k, l;
	int zero[4] = {-1, -1, -1, -1};
	// 挪动部分
	for(j=0; j<=3; j++){
		for(l=0; l<=3; l++){
				zero[l] = -1;
			}
		for(i=0; i<=3; i++){
			
			if(matrix[i][j] == 0){
				printf("(%d, %d) ---> 0\n", i, j);
				zero[i] = 0;
			}
			else{
				// printf("Start to scan zero\n");
				for(k=0; k<=3; k++){	//从右到左遍历zero
					printf("%d\t", zero[k]);
				}
				printf("\n");

				for(k=0; k<=3; k++){	//从右到左遍历zero
					if(zero[k] == 0){
						// printf("zero[%d] is empty!\n", k);
						matrix[k][j] = matrix[i][j];	//将左侧的数值移动到右侧的空白处
						matrix[i][j] = 0;
						zero[k] = -1;					//将移入的位置标志为-1
						zero[i] = 0;					//将移走的位置标志为0
						// printf("after move: zero[k]: %d \t zero[j]: %d\n", zero[k]);
						// for(k=0; k<=3; k++){			//从右到左遍历zero
						// 	printf("%d\t", zero[k]);
						// }
						printf("\n");
					}
				}
			}
		}
	}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}
	
	printf("---------------------------------------\n");
	printf("ADD\n");
	//加和部分
	for(j=0; j<=3; j++){
		for(i=0; i<=2; i++){
			if(matrix[i][j] != matrix[i+1][j]){		//相邻的不想等，j左移
				continue;
			}
			else{
				matrix[i][j] = 2 * matrix[i][j];	//相邻的相等，向右合并
				matrix[i+1][j] = 0;
				printf("%d\t%d\n", matrix[i][j], matrix[i+1][j]);
				i += 1;								//j指向到置零格子的位置
			}
		}
	}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}


	printf("---------------------------------------\n");
	printf("Move again\n");
	// 再次向右挪动
	for(j=0; j<=3; j++){
		for(l=0; l<=3; l++){
				zero[l] = -1;
			}
		for(i=0; i<=3; i++){
			
			if(matrix[i][j] == 0){
				printf("(%d, %d) ---> 0\n", i, j);
				zero[i] = 0;
			}
			else{
				// printf("Start to scan zero\n");
				// for(k=0; k<=3; k++){	//从右到左遍历zero
				// 	printf("%d\t", zero[k]);
				// }
				// printf("\n");

				for(k=0; k<=3; k++){	//从右到左遍历zero
					if(zero[k] == 0){
						printf("zero[%d] is empty!\n", k);
						matrix[k][j] = matrix[i][j];	//将左侧的数值移动到右侧的空白处
						matrix[i][j] = 0;
						zero[k] = -1;					//将移入的位置标志为-1
						zero[i] = 0;					//将移走的位置标志为0
						// printf("after move: zero[k]: %d \t zero[j]: %d\n", zero[k]);
						// for(k=0; k<=3; k++){			//从右到左遍历zero
						// 	printf("%d\t", zero[k]);
						// }
						// printf("\n");
					}
				}
			}
		}
	}

	// printf("Start to scan zero\n");
	// 			for(k=0; k<=3; k++){	//从右到左遍历zero
	// 				printf("%d\t", zero[k]);
	// 			}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}
}


void handle_down(){
	int i, j, k, l;
	int zero[4] = {-1, -1, -1, -1};
	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}

	// 挪动部分
	for(j=0; j<=3; j++){
		for(l=0; l<=3; l++){
				zero[l] = -1;
			}
		for(i=3; i>=0; i--){
			
			if(matrix[i][j] == 0){
				printf("(%d, %d) ---> 0\n", i, j);
				zero[i] = 0;
			}
			else{
				// printf("Start to scan zero\n");
				// for(k=0; k<=3; k++){	//从右到左遍历zero
				// 	printf("%d\n", zero[k]);
				// }
				// printf("\n");

				for(k=3; k>=0; k--){	//从右到左遍历zero
					if(zero[k] == 0){
						printf("zero[%d] is empty!\n", k);
						matrix[k][j] = matrix[i][j];	//将左侧的数值移动到右侧的空白处
						matrix[i][j] = 0;
						zero[k] = -1;					//将移入的位置标志为-1
						zero[i] = 0;					//将移走的位置标志为0
						// printf("after move: zero[k]: %d \t zero[j]: %d\n", zero[k]);
						// for(k=0; k<=3; k++){			//从右到左遍历zero
						// 	printf("%d\n", zero[k]);
						// }
					}
				}
			}
		}
	}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}
	
	printf("---------------------------------------\n");
	printf("ADD\n");
	//加和部分
	for(j=0; j<=3; j++){
		for(i=3; i>=1; i--){
			if(matrix[i][j] != matrix[i-1][j]){		//相邻的不想等，i上移
				continue;
			}
			else{
				matrix[i][j] = 2 * matrix[i][j];	//相邻的相等，向右合并
				matrix[i-1][j] = 0;
				printf("%d\t%d\n", matrix[i][j], matrix[i-1][j]);
				i -= 1;								//i指向到置零格子的位置
			}
		}
	}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}


	printf("---------------------------------------\n");
	printf("Move again\n");
	// 再次向右挪动
	for(j=0; j<=3; j++){
		for(l=0; l<=3; l++){
				zero[l] = -1;
			}
		for(i=3; i>=0; i--){
			
			if(matrix[i][j] == 0){
				printf("(%d, %d) ---> 0\n", i, j);
				zero[i] = 0;
			}
			else{
				// printf("Start to scan zero\n");
				// for(k=0; k<=3; k++){	//从右到左遍历zero
				// 	printf("%d\n", zero[k]);
				// }
				// printf("\n");

				for(k=3; k>=0; k--){	//从右到左遍历zero
					if(zero[k] == 0){
						printf("zero[%d] is empty!\n", k);
						matrix[k][j] = matrix[i][j];	//将左侧的数值移动到右侧的空白处
						matrix[i][j] = 0;
						zero[k] = -1;					//将移入的位置标志为-1
						zero[i] = 0;					//将移走的位置标志为0
						// printf("after move: zero[k]: %d \t zero[j]: %d\n", zero[k]);
						printf("\n");
					}
				}
			}
		}
	}

	// printf("Start to scan zero\n");
	// 			for(k=0; k<=3; k++){	//从右到左遍历zero
	// 				printf("%d\n", zero[k]);
	// 			}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}
}


void handle_left(){
	int i, j, k, l;
	int zero[4] = {-1, -1, -1, -1};
	// 挪动部分
	for(i=0; i<=3; i++){
		for(l=0; l<=3; l++){
				zero[l] = -1;
			}
		for(j=0; j<=3; j++){
			
			if(matrix[i][j] == 0){
				printf("(%d, %d) ---> 0\n", i, j);
				zero[j] = 0;
			}
			else{
				// printf("Start to scan zero\n");
				// for(k=0; k<=3; k++){	//从右到左遍历zero
				// 	printf("%d\t", zero[k]);
				// }
				// printf("\n");

				for(k=0; k<=3; k++){	//从右到左遍历zero
					if(zero[k] == 0){
						printf("zero[%d] is empty!\n", k);
						matrix[i][k] = matrix[i][j];	//将左侧的数值移动到右侧的空白处
						matrix[i][j] = 0;
						zero[k] = -1;					//将移入的位置标志为-1
						zero[j] = 0;					//将移走的位置标志为0
						// printf("after move: zero[k]: %d \t zero[j]: %d\n", zero[k]);
						// for(k=0; k<=3; k++){			//从右到左遍历zero
						// 	printf("%d\t", zero[k]);
						// }
						// printf("\n");
					}
				}
			}
		}
	}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}
	
	printf("---------------------------------------\n");
	printf("ADD\n");
	//加和部分
	for(i=0; i<=3; i++){
		for(j=0; j<=2; j++){
			if(matrix[i][j] != matrix[i][j+1]){		//相邻的不想等，j左移
				continue;
			}
			else{
				matrix[i][j] = 2 * matrix[i][j];	//相邻的相等，向右合并
				matrix[i][j+1] = 0;
				printf("%d\t%d\n", matrix[i][j], matrix[i][j+1]);
				j += 1;								//j指向到置零格子的位置
			}
		}
	}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}


	printf("---------------------------------------\n");
	printf("Move again\n");
	// 再次向右挪动
	for(i=0; i<=3; i++){
		for(l=0; l<=3; l++){
				zero[l] = -1;
			}
		for(j=0; j<=3; j++){
			if(matrix[i][j] == 0){
				printf("(%d, %d) ---> 0\n", i, j);
				zero[j] = 0;
				// printf("Start to scan zero\n");
				// for(k=0; k<=3; k++){	//从左到右遍历zero
				// 	printf("%d\t", zero[k]);
				// }
				printf("\n");
			}
			else{
				printf("(%d, %d) NOT 0\n", i, j);
				for(k=0; k<=3; k++){	//从左到右遍历zero
					if(zero[k] == 0){
						printf("zero[%d] is empty!\n", k);
						matrix[i][k] = matrix[i][j];	//将左侧的数值移动到右侧的空白处
						matrix[i][j] = 0;
						printf("%d \t %d \n", matrix[i][k], matrix[i][j] );
						zero[k] = -1;					//将移入的位置标志为-1
						zero[j] = 0;					//将移走的位置标志为0
						// printf("after move: zero[k]: %d \t zero[j]: %d\n", zero[k]);
					}
				}
			}
		}
	}

	// printf("Start to scan zero\n");
	// 			for(k=0; k<=3; k++){	//从右到左遍历zero
	// 				printf("%d\t", zero[k]);
	// 			}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}
}


void handle_right(){
	int i, j, k, l;
	int zero[4] = {-1, -1, -1, -1};
	// 挪动部分
	for(i=0; i<=3; i++){
		for(l=0; l<=3; l++){
				zero[l] = -1;
			}
		for(j=3; j>=0; j--){
			
			if(matrix[i][j] == 0){
				printf("(%d, %d) ---> 0\n", i, j);
				zero[j] = 0;
			}
			else{
				// printf("Start to scan zero\n");
				// for(k=3; k>=0; k--){	//从右到左遍历zero
				// 	printf("%d\t", zero[k]);
				// }
				printf("\n");

				for(k=3; k>=0; k--){	//从右到左遍历zero
					if(zero[k] == 0){
						printf("zero[%d] is empty!\n", k);
						matrix[i][k] = matrix[i][j];	//将左侧的数值移动到右侧的空白处
						matrix[i][j] = 0;
						zero[k] = -1;					//将移入的位置标志为-1
						zero[j] = 0;					//将移走的位置标志为0
						// printf("after move: zero[k]: %d \t zero[j]: %d\n", zero[k]);
						// for(k=3; k>=0; k--){			//从右到左遍历zero
						// 	printf("%d\t", zero[k]);
						// }
						printf("\n");
					}
				}
			}
		}
	}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}
	
	printf("---------------------------------------\n");
	printf("ADD\n");
	//加和部分
	for(i=0; i<=3; i++){
		for(j=3; j>=1; j--){
			if(matrix[i][j] != matrix[i][j-1]){		//相邻的不想等，j左移
				continue;
			}
			else{
				matrix[i][j] = 2 * matrix[i][j];	//相邻的相等，向右合并
				matrix[i][j-1] = 0;
				printf("%d\t%d\n", matrix[i][j], matrix[i][j-1]);
				j -= 1;								//j指向到置零格子的位置
			}
		}
	}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}


	printf("---------------------------------------\n");
	printf("Move again\n");
	// 再次向右挪动
	for(i=0; i<=3; i++){
		for(l=0; l<=3; l++){
				zero[l] = -1;
			}
		for(j=3; j>=0; j--){
			if(matrix[i][j] == 0){
				printf("(%d, %d) ---> 0\n", i, j);
				zero[j] = 0;
				// printf("Start to scan zero\n");
				// for(k=0; k<=3; k++){	//从右到左遍历zero
				// 	printf("%d\t", zero[k]);
				// }
				// printf("\n");
			}
			else{
				printf("(%d, %d) NOT 0\n", i, j);
				for(k=3; k>=0; k--){	//从右到左遍历zero
					if(zero[k] == 0){
						printf("zero[%d] is empty!\n", k);
						matrix[i][k] = matrix[i][j];	//将左侧的数值移动到右侧的空白处
						matrix[i][j] = 0;
						printf("%d \t %d \n", matrix[i][k], matrix[i][j] );
						zero[k] = -1;					//将移入的位置标志为-1
						zero[j] = 0;					//将移走的位置标志为0
						// printf("after move: zero[k]: %d \t zero[j]: %d\n", zero[k]);
					}
				}
			}
		}
	}

	// printf("Start to scan zero\n");
	// 			for(k=0; k<=3; k++){	//从右到左遍历zero
	// 				printf("%d\t", zero[k]);
	// 			}

	printf("Start to scan matrix\n");
	// int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}
}


void rand_digital(){
	int b[32], k=0, m=0, i, j;
	for(i=0;i<4;i++)
	for(j=0;j<4;j++){
		if(matrix[i][j] == 0){
			// printf("(%d, %d)\n", i, j);
			b[k] = i;
			b[k+1] = j;
			k += 2;
			m += 1;
		}
	}
	if(m>0){
		int a;
    	srand((unsigned)time(NULL));
		a = rand();
		printf("%d\n", a);
		int t = a % m;
		// if(t % 2 != 0) t -= 1;
		matrix[b[2*t]][b[2*t+1]] = 2;
	}


	
}


int False(){
	// printf("enter false\n");
	int i,j;
	for(i=0;i<4;i++)
	for(j=0;j<3;j++)
	if(matrix[i][j]==0||matrix[i][j]==matrix[i][j+1]) return 0;	//遍历左右
	for(j=0;j<4;j++)
	for(i=0;i<3;i++)
	if(matrix[i][j]==0||matrix[i+1][j]==matrix[i][j]) return 0;	//遍历上下
	if(matrix[3][3]==0) return 0;
	return 1;
}


int Success(){
	// printf("Start to scan matrix\n");
	int i,j;
	for(i=0;i<4;i++)
	for(j=0;j<4;j++){
		// printf("%d\n", matrix[i][j]);
		if(matrix[i][j]==SUCCESS_NUM) return 1;				//遍历所有格，如果有2048，即成功！
	}
	return 0;
}


void pfalse(){
	lcd_draw_rect(0, 0, 800, 480, 0x0);
	lcd_draw_bmp("false.bmp",0,0);
}


void psuccess(){
	lcd_draw_rect(0, 0, 800, 480, 0x0);
	lcd_draw_bmp("success.bmp",0,0);
}





