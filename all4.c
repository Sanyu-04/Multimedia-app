#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<linux/input.h>
#include <errno.h>
#include <sys/mman.h>
#include <yx1.h>

//1.进入主界面，显示菜单
//2.选择功能，进入子菜单

//定义全局变量
int lcd;                            //定义lcd的文件描述符
int ts;                             //定义触摸屏的文件描述符
int fifo_fd;                        //定义管道文件描述符
int num_V=0;                        //定义视频数组的下标
int ts,lcd,bmp_video;                     //event0，fb0，视频背景的文件描述符
int touch_x,touch_y;                //保存读取到的坐标

char *bmp_path_first[1];            //存放选择界面图片路径
char *video_path[4];                //定义视频路径数组
char str1_v[100];                   //定义视频字符串
int *mmap_star;                     //内存映射空间申请指针
struct input_event touch;           //进行数据分类

int w,h,n;                          //图片的长和宽
int music_num;                      //补齐的像素点
int max;                            //图片的像素点
int bmp_music;                      //图片文件描述符
int mus;                            //音乐标志位
int pho=0;                          //图片标志位
//音乐的播放
char *mus_path[4];                  //指针数组-->二维数组
int b=0;                            //切换音乐路径的标志位
char *bmp_path[8];//指针数组-->二维数组
int c=2;                            //切换上下首音乐的标志位

int color;                          //画笔颜色变量
int draw_x;                         //画笔的x坐标
int draw_y;                         //画笔的y坐标
//相册全局变量
int num;
int bmp_w_qq,bmp_h_qq;                                                                                //图片宽高
char *imagePathsA[] = {"/yu.bmp", "/tutu.bmp", "/qiyu.bmp","/black.bmp","/p.bmp"};              //图片路径(Page==0)
struct input_event touch;
int touch_bmpX,touch_bmpY;                                                                            //保存读取到的坐标
//int Currnum =0,numall=5;                                                                      //当前图片序号(currnum)，图片总数(numall)     
int flag_x_y;                                                                                  //判断是否划动情况标志位   
int Page=0;                                                                                    //判断是否翻页标志位
struct xxx *currentNode;                                                                       //当前节点
struct xxx *head;                                                                       //头节点
struct xxx *tail;                                                                       //尾节点

struct xxx
{
    char bmp_path[100];
    struct xxx *next;     
    struct xxx *prev;     
};

/*功能函数声明*/
int Dev_Init();                     //打开设备文件 
int show_bmp_first(char path[]);    //显示主界面的bmp图片
int get_touch_xy();                 //获取触摸屏坐标
int dev_free();                     //关闭设备,释放内存空间
int pic_first();                    //显示主界面图片
int video_init();                   //初始化视频显示设备
int video_show();                   //视频界面背景
int video_selcet();                 //切换视频的循环
int video_touch();                  //视频触摸模块 读取按钮

int get_bmp_size();                 //获取图片长宽
int show_back();                    //显示背景图
int show_stop(char path[]);         //显示暂停图
int play_music();                   //播放音乐
int mpic();                         //音乐的播放和触摸屏的坐标相结合
int dev_free();                     //释放内存空间
int show_bmp();
int music_chose();                  //音乐选择

int m2048(int argc, char *argv[]);  //2048游戏
int show_drawback();                //显示画图图片
int get_draw_xy();                  //获取画图坐标
int draw_back();                    //显示画图背景
int show_drawdelete();
void draw();                        //画图
int show_gamemulu();                //显示游戏菜单
int game_choose();                  //选择游戏目录

void displayImage(int lcd, char* imagePath);           //显示图片函数
void get_touch();                                      //触摸事件函数（获取点击坐标）
int get_touch_event();                                 //获取触摸事件函数(左划、右划)
struct xxx *createNode(char *path);                   //创建节点函数
void insertNode(struct xxx **head, char *path);       //插入节点函数(尾部)
void freeLinkedList(struct xxx *head);                //释放链表函数
struct xxx *buildLinkedList();                        //构建链表函数
void traverseLinkedList(struct xxx *head);            //遍历链表函数
void nextImage(struct xxx *head);                     //切换下一张图片函数
void lastImage(struct xxx *head);                     //切换上一张图片函数
void block_collision(int *mmap_star);                 //碰撞检测函数
int Page_change();                                    //页面切换函数

int Dev_Init()
{
    //打开fb0和event0文件
    lcd = open("/dev/fb0", O_RDWR);
    ts = open("/dev/input/event0", O_RDONLY);
    if (lcd == -1 || ts == -1)
    {
        printf("打开文件失败\n");
        return -1;
    }
    else
    {
        printf("打开文件成功\n");
    }
    //内存映射空间申请
    mmap_star = (int *)mmap(NULL, 800*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd, 0);
    if (mmap_star == (int *)-1)
    {
        printf("内存空间映射失败\n");
        return -1;
    }
    else
    {
        printf("内存空间映射成功\n");
    }
    return 0;
}

int show_bmp_first(char *path)//通过指针传递路径参数
{
    //打开图片文件
    int bmp = open(path,O_RDWR);
    if(bmp == -1)
    {
        printf("打开文件失败！\n");
        return -1;
    }
    else
    {
        printf("打开文件成功！\n");
    }
   
    //将图片里面的像素点读取出来
    //补充：自动获取图片的长与宽
    int bmp_w,bmp_h;
    lseek(bmp,18,SEEK_SET);//将文件指针偏移到传入字节数处（文件头开始偏移)
    
    read(bmp,&bmp_w,4);
    read(bmp,&bmp_h,4);
    printf("图片的长：%d---宽：%d\n",bmp_w,bmp_h);

    lseek(bmp,54,SEEK_SET);//跳过基本信息域
    int data=bmp_w*bmp_h*3;//存放像素点
    char buf[data];

    int num;//存放每一行需要补的字节数
     //判断宽度是否能被4整除
    if(bmp_w*3%4 == 0)
    {
        num = 0;
    }
    else
    {
        num = 4-(bmp_w*3%4);
    }

    int n;
    for(n=0;n<bmp_h;n++)
    {
        read(bmp,buf+(n*bmp_w*3),bmp_w*3);//读取一行像素点
        //将读取的像素点写入到显存中
        lseek(bmp,num,SEEK_CUR);//将文件指针偏移到传入字节数处
    }

    int x,y;    
    for(y=0;y<bmp_h;y++)
    {
        for(x=0;x<bmp_w;x++)
        {
            if(x>=0 && x<=bmp_w && y>=0 && y<=bmp_h)
            {
                *(mmap_star+800*(479-y)+x) = (buf[3*(bmp_w*y+x)]<<0)+(buf[3*(bmp_w*y+x)+1]<<8)+(buf[3*(bmp_w*y+x)+2]<<16);    
            }
            
        }
    }
    //关闭图片文件
    close(bmp);
    return 0;
}

int get_touch_xy()
{
    //打印x,y坐标
    int x,y;
    while(1)
    {   
        read(ts, &touch, sizeof(touch));//读取触摸屏数据
        //注意：黑色板子的触摸屏比例是1024*600，蓝色板子的触摸屏比例是800*480;
        if(touch.type == EV_ABS && touch.code == ABS_X) x = touch.value*800/1024;   //触摸点x坐标
        if(touch.type == EV_ABS && touch.code == ABS_Y) y = touch.value*480/600;   //触摸点y坐标

        if(touch.type == EV_KEY && touch.code == BTN_TOUCH && touch.value == 0)//手指离开屏幕
        {
            touch_x=x;
            touch_y=y;
            printf("x:%d,y:%d\n",touch_x,touch_y);
            break;
        }
    }
    return 0;
}

int pic_first()
{
    bmp_path_first[0]="/zhu.bmp";
    show_bmp_first(bmp_path_first[0]);
    int n=0;
    //先获取触摸屏坐标
    while(1)
    {
        get_touch_xy();
        
        if(touch_x>=200 && touch_x<=350 && touch_y>=180 && touch_y<270)//相册位置
        {
            printf("打开相册\n");
            system("mplayer -slave -input file=/my_fifo -zoom -x 800 -y 480 /zc2.avi &"); 
            usleep(1800000);
            system("killall -SIGKILL mplayer");
            Page_change();
        }
        else if(touch_x>=200 && touch_x<=350 && touch_y>=320 && touch_y<400)//视频位置
        {
            printf("打开视频\n");
            system("mplayer -slave -input file=/my_fifo -zoom -x 800 -y 480 /zc4.avi &"); 
            usleep(2500000);
            system("killall -SIGKILL mplayer");
            
            video_show(); //显示视频界面背景图片
            video_touch(); //触摸播放视频     
        }
        else if(touch_x>=470 && touch_x<580 && touch_y>170 && touch_y<270)//音乐位置
        {
            printf("打开音乐\n");
            system("mplayer -slave -input file=/my_fifo -zoom -x 800 -y 480 /zc3.avi &"); 
            usleep(2000000);
            system("killall -SIGKILL mplayer");
            music_chose();
        }
        else if(touch_x>=470 && touch_x<580 && touch_y>=280 && touch_y<380)//游戏位置
        {
            printf("打开游戏\n");
            system("mplayer -slave -input file=/my_fifo -zoom -x 800 -y 480 /zc1.avi &"); 
            usleep(2500000);
            system("killall -SIGKILL mplayer");
            game_choose(); //游戏选择界面
        }
    }
    return 0;
}

int dev_free()
{
    //关闭设备，释放内存空间
    close(lcd);
    close(ts);
    munmap(mmap_star,800*480*4);
    return 0;
}

int video_init()
{
    //先判断即将使用的管道文件是否存在
    //如果不存在则要先去创建它
    if (access("/my_fifo",F_OK))
    {
        mkfifo("/my_fifo",0777);
    }
    //然后有管道文件之后，就要把管道文件打开
    fifo_fd = open("/my_fifo", O_RDWR);
    if (fifo_fd == -1)
    {
        perror("打开管道失败");
        return -1;
    }
    //打开设备文件event0
    ts = open("/dev/input/event0", O_RDWR);
    lcd=open("/dev/fb0",O_RDWR);
    if (ts==-1 || lcd == -1 )
    {
       perror("打开event0fb0失败\n");
       return -1;
    }
    else
    {
        printf("打开event0fb0成功\n");
    }    
    bmp_video = open("/video.bmp",O_RDWR);
    //进行内存映射的空间申请
    mmap_star = (int *)mmap(NULL,800*480*4,PROT_READ | PROT_WRITE,MAP_SHARED,lcd,0);
    if (mmap_star == (int *)-1)
    {
        printf("内存映射失败");
        return 0;
    }
    else
    {
        printf("内存映射成功");
    }

    return 0;
}

int video_show()
{
    //将图片的像素点读取出来
    lseek(bmp_video,54,SEEK_SET);

    char buf[800*480*3] = {0}; //数组名等同于数组的首地址 等同于数组地址
    int read_ret = read(bmp_video,buf,800*480*3);

    //像素点的改造和映射一气呵成
    int x,y;
    for (y=0;y<480;y++)
    {
        for (x=0;x<800;x++)
        {
            *(mmap_star+800*(479-y)+x) = (buf[3*(800*y+x)]<<0) + (buf[3*(800*y+x)+1]<<8) + (buf[3*(800*y+x)+2]<<16);
        }    
    }
    return 0;
}

int video_selcet()
{
    if (num_V>3)
    {
        num_V = 0;
    }
    if (num_V<0)
    {
        num_V = 3;
    }
    video_path[0]="/1.avi";
    video_path[1]="/2.avi";
    video_path[2]="/3.avi";
    video_path[3]="/4.avi";
    sprintf(str1_v,"mplayer -slave -input file=/my_fifo -zoom -x 800 -y 400 %s &",video_path[num_V]);
    return 0;
}

int video_touch()
{
    //循环读取设备文件里的数据
    struct input_event touch;
    int touch_Vx,touch_y; //记录触摸点的坐标
    while (1)
    {
        //拼接函数定义切换视频
        read(ts, &touch, sizeof(touch));
        //3.将读取到的数据进行if区分,得到想要的内容 并打印出来
        if(touch.type == EV_ABS && touch.code == ABS_X) touch_Vx = touch.value*800/1024;   //触摸点x坐标
        if(touch.type == EV_ABS && touch.code == ABS_Y) touch_y = touch.value*480/600;   //触摸点y坐标
        //介入压力值
        if (touch.type == EV_KEY && touch.code == BTN_TOUCH && touch.value == 0)  //触摸结束
        {
            printf("x = %d, y = %d\n", touch_Vx, touch_y);
            if (touch_Vx >=0 && touch_Vx <=400 && touch_y >=0 && touch_y <=400)  //上一个视频
            {
                system("killall -SIGKILL mplayer");
                printf("上一个视频\n");
                num_V--;
                video_selcet();
                system(str1_v);
            }
            if (touch_Vx >=400 && touch_Vx <=800 && touch_y >=0 && touch_y <=400)  //下一个视频
            {
                system("killall -SIGKILL mplayer");
                printf("下一个视频\n");
                num_V++;
                video_selcet();
                system(str1_v);
            }
            if (touch_Vx >=0 && touch_Vx <=120 && touch_y >=400 && touch_y <=480)  //退出播放
            {
                printf("退出视频\n");
                system("killall -SIGKILL mplayer");
                break;
            }
            if (touch_Vx >=120 && touch_Vx <=240 && touch_y >=400 && touch_y <=480)  //快退
            {
                printf("快退 5秒\n");
                write(fifo_fd, "seek -5\n", strlen("seek -5\n"));
            }
            if (touch_Vx >=240 && touch_Vx <=370 && touch_y >=400 && touch_y <=480)  //播放
            {
                printf("播放视频\n");
                video_selcet();
                system(str1_v);
            }
            if (touch_Vx >=370 && touch_Vx <=500 && touch_y >=400 && touch_y <=480)  //暂停/继续
            {
                printf("暂停或继续视频\n");
                write(fifo_fd, "pause\n", strlen("pause\n"));
            }
            if (touch_Vx >=500 && touch_Vx <=605 && touch_y >=400 && touch_y <=480)  //快进
            {
                printf("快进 5秒\n");
                write(fifo_fd, "seek +5\n", strlen("seek +5\n"));
            }
            if (touch_Vx >=605 && touch_Vx <=705 && touch_y >=400 && touch_y <=480)  //音量-
            {
                printf("音量—\n");
                write(fifo_fd,"volume -300\n",strlen("volume -600\n"));
            }
            if (touch_Vx >=705 && touch_Vx <=800 && touch_y >=400 && touch_y <=480) //音量+
            {
                printf("音量+\n");    
                write(fifo_fd,"volume +300\n",strlen("volume +600\n"));
            }
        }
    }
    pic_first();
}
//获取图片长宽
int get_bmp_size()
{
    //自动获取图片的长w和宽h
    lseek(bmp_music,18,SEEK_SET);//跳过bmp图片的前18个字节,才能获取到长和宽
    read(bmp_music,&w,4);/**/
    read(bmp_music,&h,4);/**/
    printf("图片的长：%d---宽：%d\n",w,h);

    lseek(bmp_music,54,SEEK_SET);//跳过前54个字节
    max=w*h*3;
    printf("图片的像素点：%d\n",max);
    
   //判断宽度是否能被4整除，计算需要补的字节数
    
    if (w*3%4==0)
    {
        music_num=0;
    }
    else
    {
        music_num=4-(w*3%4);
    }
}

//显示继续停止标识
int show_stop(char *path)
{
    //1.打开图片,传什么路径打开什么路径
    bmp_music = open(path,O_RDWR);
   //自动获取图片的长w和宽h
    get_bmp_size();
    char buf[max];
    for (n = 0; n < h; n++)
    {
      read(bmp_music,buf+(n*w*3),w*3);
      lseek(bmp_music,music_num,SEEK_CUR);
    }
    //像素点的改造和映射
    int x,y;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            if(pho==0)
            {
                *(mmap_star+800*(479-y)+x)=(buf[3*(w*y+x)]<<0)+(buf[3*(w*y+x)+1]<<8)+(buf[3*(w*y+x)+2]<<16);
            }
            else if (pho==1)
            {
                *(mmap_star+800*(350-y)+(x+300))=(buf[3*(w*y+x)]<<0)+(buf[3*(w*y+x)+1]<<8)+(buf[3*(w*y+x)+2]<<16);
            }
            else if (pho==2)
            {
                *(mmap_star+800*(293-y)+(x+160))=(buf[3*(w*y+x)]<<0)+(buf[3*(w*y+x)+1]<<8)+(buf[3*(w*y+x)+2]<<16);
            }
        }
    }
    //5.释放空间
    close(bmp_music);
    return 0;
}

int play_music()
{
    mus_path[0]="/tox.mp3";
    mus_path[1]="/egg.mp3";
    mus_path[2]="/bee.mp3";
    mus_path[3]="/fly.mp3";
    char str1[100]="madplay ";
    char str3[]=" &";
    if(mus == 0)
    {
        printf("播放音乐\n");
        sprintf(str1,"%s%s%s",str1,mus_path[b],str3);
        system(str1);//这里的&是后台运行的意思
    }
    if(mus == 1)
    {
        printf("暂停音乐\n");
        system("killall -SIGSTOP madplay");
    }
    if(mus == 2)
    {
        printf("继续音乐\n");
        system("killall -SIGCONT madplay");
    }
    if(mus == 3)
    {
        printf("退出\n");
        system("killall -SIGKILL madplay");
    }
    return 0;
}

int mpic()
{
    bmp_path[0]="/stop.bmp";
    bmp_path[1]="/bofang.bmp";
    bmp_path[2]="/tox.bmp";
    bmp_path[3]="/egg.bmp";
    bmp_path[4]="/bee.bmp";
    bmp_path[5]="/fly.bmp";
    bmp_path[6]="/back.bmp";
    int a=0;
    int stop=0;
   // 1.先获取触摸坐标
    while(1)
    {
        get_touch_xy();
    
        if (touch_x>=0 && touch_x<=160 && touch_y>=200 && touch_y<=300)//开始播放
        {
            printf("开始播放\n");
            mus=0;
            play_music();
        }
        if (touch_x>=200&&touch_x<=270&&touch_y>=200&&touch_y<=300)//暂停
        {
            printf("继续/停止\n");
            pho=2;
            if(a==0)
            {
                a=1;
            }
            else
            {
                a=0;
            }
            show_stop(bmp_path[a]);
            stop++;
            if(stop==1)
            {
                printf("暂停\n");
                mus=1;
            }
            else if(stop==2)
            {
                mus=2;
                stop=0;
            }
            play_music();
        }
        
        if (touch_x>=100&&touch_x<=200&&touch_y>=400&&touch_y<=480)//下一首
        {
            system("killall -SIGKILL madplay");
            printf("下一首\n");
            pho=1;
            if(c==5)
            {
                c=2;
            }
            else
            {
                c++;
            }
            show_stop(bmp_path[c]);
            printf("下一首\n");
            if(b==3)
            {
                b=0;
            }
            else
            {
                b++;
            }
            mus=0;
            play_music();
        }
        if (touch_x>=0&&touch_x<=80&&touch_y>=400&&touch_y<=480)//上一首
        {
            system("killall -SIGKILL madplay");
            printf("上一首\n");
            pho=1;
            if(c==2)
            {
                c=5;
            }
            else
            {
                c--;
            }
            show_stop(bmp_path[c]);
            printf("上一首\n");
            if(b==0)
            {
                b=3;
            }
            else
            {
                b--;
            }
            mus=0;
            play_music();
        }
        if (touch_x>=700&&touch_x<=800&&touch_y>=0&&touch_y<=100)//退出
        {
            printf("退出\n");
            system("killall -SIGKILL madplay");
            mus=3;
            play_music();
            break;
        }
    }
    pho=0;
    music_chose();

    return 0;
}

//实现歌曲选择目录界面
int music_chose()
{
    printf("music_chose\n");
    bmp_path[2]="/tox.bmp";
    bmp_path[3]="/egg.bmp";
    bmp_path[4]="/bee.bmp";
    bmp_path[5]="/fly.bmp";
    bmp_path[6]="/back.bmp";
    bmp_path[7]="/mulu.bmp";
    show_stop(bmp_path[7]);//显示目录

    while(1)
    {
        get_touch_xy();
        //2.再根据坐标位置，左下角上一张，右下角下一张，右上角退出
        if (touch_x>=420&&touch_x<=800&&touch_y>=110&&touch_y<=185)
        {
            pho=0;
            show_stop(bmp_path[6]);
            pho=1;
            c=2;
            show_stop(bmp_path[c]);
            b=0;
            mpic();
        }
        if (touch_x>=390&&touch_x<=800&&touch_y>=200&&touch_y<=270)
        {
            pho=0;
            show_stop(bmp_path[6]);
            pho=1;
            c=3;
            show_stop(bmp_path[c]);
            b=1;
            mpic();
        }
        if (touch_x>=360&&touch_x<=800&&touch_y>=290&&touch_y<=365)
        {
            pho=0;
            show_stop(bmp_path[6]);
            pho=1;
            c=4;
            show_stop(bmp_path[c]);
            b=2;
            mpic();
        }
        if (touch_x>=340&&touch_x<=800&&touch_y>=380&&touch_y<=450)
        {
            pho=0;
            show_stop(bmp_path[6]);
            pho=1;
            c=5;
            show_stop(bmp_path[c]);
            b=3;
            mpic();
        }
        if (touch_x>=700&&touch_x<=800&&touch_y>=0&&touch_y<=100)//退出
        {
            printf("退出\n");
            break;
        }
    }
    pic_first();
    return 0;
}

void My_2048(){
	int direction;
	init_matrix();

	while(1){
		get_touch_xy();
		if (touch_x>=680 && touch_x<800 && touch_y>=400 && touch_y<480)//退出
		{
			printf("退出\n");
			break;
		}
		lcd_draw_matrix();
		direction = get_finger_move_direction();
		switch (direction){
			case UP:
				handle_up();
				rand_digital();
				break;
			case DOWN:
				handle_down();
				rand_digital();
				break;
			case LEFT:
				handle_left();
				rand_digital();
				break;
			case RIGHT:
				handle_right();
				rand_digital();
				break;
		}
		
		// handle(direction);
		// rand_digital();

		if(False()){
			pfalse();
			return;
		}


		if(Success()){
			psuccess();
			return;
		}
		// rand_digital();
	}
    game_choose();
}

int m2048(int argc, char *argv[])
{
	int fd;
	
	fd = open("/dev/fb0", O_RDWR);
	//if (fd == -1)
		
	if (-1 == fd){
		printf("/dev/fb0 open error!\n");
		return 0;
	}
	
	plcd = mmap(NULL, 800*480*4,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					fd,
					0);
	if (MAP_FAILED == plcd){
		printf("mmap error !\n");
		return 0;
	}
	#if 0
	int i;
	for (i = 0; i < 800*480;i++)
		*(plcd + i) = 0x0000ff00;
	#endif
	
	#if 0
	int x, y, color;
	color = 0x000000ff;
	for (y = 0; y < 480; y++){
		for (x = 0; x < 800; x++)
			draw_point(x, y, color);
	}
	#endif
	//在坐标为（200,200）的位置，显示一个100*100的颜色矩形，颜色为红色
	#if 0
	for (y = 200; y < 200+100; y++){
		for (x = 200; x < 200 + 100; x++)
			draw_point(x, y, color);
	}
	#endif
	lcd_draw_rect(0, 0, 800, 480, 0x0);
	
    
	My_2048();
	
	
	return 0;
}

void displayImage(int lcd, char* imagePath)              //显示图片
{
    int bmp = open(imagePath, O_RDONLY);
    lseek(bmp,18,SEEK_SET);                             //从文件开头处光标偏移18个字节
    read(bmp,&bmp_w_qq,4);                                 //读取图片的宽度
    read(bmp,&bmp_h_qq,4);                                 //读取图片的高度
    printf("图片的长度：%d---宽：%d\n",bmp_w_qq,bmp_h_qq);

        int num;                                        //存放每一行需要补的字节数,判断宽度是否能被4整除
        if (bmp_w_qq*3%4 == 0)
        {
            num = 0;
        }
        else
        {
            num = 4-(bmp_w_qq*3%4);
        }
        int max = bmp_w_qq*bmp_h_qq*3 + num*bmp_h_qq;            //计算图片总大小
        lseek(bmp,54,SEEK_SET);                         //图片文件开头偏移54个字节
        char rgb[max];                                  //将图片文件的像素点读取出来 放到便利贴(800*480*3)
        int read_ret = read(bmp,rgb,max);

        //读取像素点
        int argb[800*480] = {0};
        int x,y,n;
        for (y=0,n=0;y<bmp_h_qq;y++)
        {
            for (x=0;x<bmp_w_qq;x++,n+=3)
            {
                argb[800*(479-y)+x] = (rgb[n]<<0) + (rgb[n+1]<<8) + (rgb[n+2]<<16);
            }
            //跳过系统补齐的字节数
            n+=num;
        }
        // 第三步：将便利贴里面的像素点，写进fb0文件里面
        int write_ret = write(lcd,argb,800*480*4);
        lseek(lcd,0,SEEK_SET);                         //将光标移到文件开头处

        close(bmp);
}

void get_touch()
{
    read(ts,&touch,sizeof(touch));
    
    if (touch.type == EV_ABS && touch.code == ABS_X) //x的坐标
        touch_bmpX=touch.value * 800/1024;
    if (touch.type == EV_ABS && touch.code == ABS_Y) //y的坐标
        touch_bmpY=touch.value *480/600;
}

int get_touch_event()
{
    int tmp_touch_x = 0;                                // 此变量用于记录起始坐标值
    int tmp_touch_y = 0;

    int event_flags = -1;                               // 触摸屏动作标志位（-1识别失败或未定义，0 单击，1左划， 2右划）

    while(1) 
    {
        get_touch(); // 获取触摸屏信息，以及X，Y决定坐标事件处理
        if( touch.type == EV_KEY && touch.code == BTN_TOUCH && touch.value > 0)
        {
            tmp_touch_x = touch_bmpX; // 保存起始坐标值
            tmp_touch_y = touch_bmpY;
        }

        if( touch.type == EV_KEY && touch.code == BTN_TOUCH && touch.value == 0) 
        // 手指离开触摸屏
        {
            //printf("手指松开触摸屏\n");
            event_flags = 0; // 单击动作
            if( tmp_touch_x - touch_bmpX > 50 ) event_flags = 1; // 左划动作
            if( touch_bmpX - tmp_touch_x > 50 ) event_flags = 2; // 右划动作

            return event_flags; // 返回触摸屏动作标志位
        }
    }
 }

struct xxx *createNode(char *path)
{
    // 分配内存空间给新节点
    struct xxx *newNode = (struct xxx *)malloc(sizeof(struct xxx));

     // 复制图片路径到节点中
    strcpy(newNode->bmp_path, path);
    //printf("&s", newNode->bmp_path);

    // 初始化新节点的指针
    newNode->next = NULL;
    newNode->prev = NULL;

    if (head == NULL) 
    {
        newNode->next = newNode;
        newNode->prev = newNode;
        head = newNode;
        tail = newNode;
    }
    else
    {
        newNode->next = head;
        newNode->prev = tail;
        tail->next = newNode;
        tail->prev = newNode;
        tail = newNode;
    }

    return newNode;
}

void insertNode(struct xxx **head, char *path) 
{
    // 创建新节点
    struct xxx *newNode = createNode(path);
    // 如果链表为空，新节点就是头节点
    if (*head == NULL) 
    {
        *head = newNode;
    } 
    else 
    {
        newNode->next = *head;
        newNode->prev = (*head)->prev;
        (*head)->prev->next = newNode;
        (*head)->prev = newNode;
    }
}

void freeLinkedList(struct xxx *head) 
{
    if(head != NULL)
    {
        struct xxx *current = head;
        struct xxx *next;

        do
        {
            next = current->next;
            free(current);
            current = next;
        }while(current != head);
    }
}

struct xxx *buildLinkedList() 
{
    struct xxx *head = NULL;
    int i;
    for (i = 0; i < sizeof(imagePathsA) / sizeof(imagePathsA[0]); i++) 
    {
        // 逐个将图片路径数组 A 中的路径插入链表
        insertNode(&head, imagePathsA[i]);
    }
    currentNode = head;
    return head;
}

void traverseLinkedList(struct xxx *head) 
{
    struct xxx *temp = head;
    do
    {       
        // 打印每个节点中的图片路径
        printf("Image path: %s\n", temp->bmp_path);
        temp = temp->next;
    } while (temp != head);
    
}

void nextImage(struct xxx *head)
{
    currentNode = currentNode->next;

    displayImage(lcd, currentNode->bmp_path);
    displayImage(lcd, currentNode->bmp_path);

}

void lastImage(struct xxx *head)
{
    currentNode = currentNode->prev;

    displayImage(lcd, currentNode->bmp_path);
    displayImage(lcd, currentNode->bmp_path);
}

void block_collision(int *mmap_star)
{
    int x = 0, y = 0;
    int z = 0;
    int m = 0;
    int flag = 0, fla = 0;


    int bmp1 = open("/qi.bmp", O_RDWR);
    int bmp2 = open("/en.bmp", O_RDWR);
    int bmp3 = open("/tutu1.bmp", O_RDWR);
    int bmp4 = open("/mu.bmp", O_RDWR);
    int bmp5 = open("/shui.bmp", O_RDWR);
    int bmp[5] = {bmp1, bmp2, bmp3, bmp4, bmp5};
    int i = 0;
    int photo;


    int peng = 0;


    char buf[100*100*4];


    // 生成一个0到100之间的随机数字
    int a = 0, b = 0;


    while (1) 
    {
        if (peng == 1) 
        {
            i++;
            peng = 0;
        }
        if (i > 4) 
        {
            i = 0;
        }
        photo = bmp[i];
        lseek(photo, 54, SEEK_SET);
        read(photo, buf, 100*100*3);


        for (y = 0; y < 480; y++) 
        {
            for (x = 0; x < 800; x++) 
            {
                if (x >= 0+z && x < 100+z && y >= 0+m && y < 100+m) 
                {
                    *(mmap_star+800*(99+2*m - y)+x)=(buf[(100*(y - m)+(x - z))*3]<<0)+(buf[(100*(y - m)+(x - z))*3+1]<<8)+(buf[(100*(y - m)+(x - z))*3+2]<<16);
                } 
                else 
                {
                    *(mmap_star+800*y+x)=0x00ffffff;
                }
            }
        }


        if (z >= 700) 
        {
            flag = 1;
            a = rand() % 10+1;
            peng = 1;
        } 
        else if (z <= 0) 
        {
            flag = 0;
            a = rand() % 10+1;
            peng = 1;
        }


        if (m >= 375) {
            fla = 1;
            b = rand() % 5+1;
            peng = 1;
        } else if (m <= 5) {
            fla = 0;
            b = rand() % 5+1;
            peng = 1;
        }


        if (flag == 0) {
            z = z+a;
        } else {
            z = z - a;
        }


        if (fla == 0) {
            m = m+b;
        } else {
            m = m - b;
        }


        usleep(500);

        get_touch();
        if (touch.type == EV_KEY && touch.code == BTN_TOUCH && touch.value == 0) //手指离开
        {
            printf("x:%d,y:%d\n",touch_bmpX,touch_bmpY);
            if(touch_bmpX >= 0 && touch_bmpX <= 100 && touch_bmpY >= 0 && touch_bmpY <= 100)//左上角：退出（100*100）
            {
                printf("退出\n");
                
                break;
            }
        }
    }
    printf("1111\n");
    Page = 0;
    Page_change();



    close(bmp1);
    close(bmp2);
    close(bmp3);
    close(bmp4);
    close(bmp5);
}

int Page_change()
{
    struct xxx *list = buildLinkedList();
    traverseLinkedList(list);
    if(list != NULL)
    {
        currentNode = list;
        displayImage(lcd,currentNode->bmp_path);
        displayImage(lcd,currentNode->bmp_path);
    }

    if(Page == 0)
    {
        displayImage(lcd,currentNode->bmp_path);
        displayImage(lcd,currentNode->bmp_path);
        while (1)   
        {   
                get_touch();
                flag_x_y = get_touch_event(); 
                if (touch.type == EV_KEY && touch.code == BTN_TOUCH && touch.value == 0) //手指离开
                {
                    printf("x:%d,y:%d\n",touch_bmpX,touch_bmpY);
                    if(touch_bmpX >= 0 && touch_bmpX <= 100 && touch_bmpY >= 0 && touch_bmpY <= 100)//左上角：退出（100*100）
                    {
                        printf("退出\n");
                        break;
                    }
                    if(touch_bmpX >= 0 && touch_bmpX <= 100 && touch_bmpY >= 380 && touch_bmpY <= 480)//左下角：目录（100*100）
                    {
                        printf("目录\n");
                        Page = 1;
                        block_collision(mmap_star);                                                     // 执行方块碰撞逻辑
  
                    }
                    if(touch_bmpX >= 0 && touch_bmpX <= 100 && touch_bmpY >= 100 && touch_bmpY <= 380)//左中角：上一张（100*280）
                    {
                        printf("上一张\n"); 
                        lastImage(currentNode);
                    }
                    if(touch_bmpX >= 700 && touch_bmpX <= 800 && touch_bmpY >= 0 && touch_bmpY <= 480)//右边：下一张（100*480）
                    {
                        printf("下一张\n");
                        nextImage(currentNode);
                    }
                    if(flag_x_y == 2)
                    {
                        printf("上一张\n"); 
                        lastImage(currentNode);
                    }
                    if(flag_x_y == 1)
                    {
                        printf("下一张\n");
                        nextImage(currentNode);
                    }
                }
        }
        pic_first();
    }
    if (Page == 1)
    {
        // // 申请映射空间
        // mmap_star=(int*)mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd,0);
        // if (mmap_star == MAP_FAILED)
        // {
        //     perror("mmap");
        //     return -1;
        // }

        // 执行方块碰撞逻辑
        block_collision(mmap_star);


        //释放mmap的内存
        if (munmap(mmap_star, 800*480*4) == -1)
        {
            perror("munmap");
        }
    }
    return 0;
}

int get_draw_xy()
{
    //打印x,y坐标
    int x,y;
    while(1)
    {   
        read(ts, &touch, sizeof(touch));//读取触摸屏数据
        //注意：黑色板子的触摸屏比例是1024*600，蓝色板子的触摸屏比例是800*480;
        if(touch.type == EV_ABS && touch.code == ABS_X) x = touch.value*800/1024;   //触摸点x坐标
        if(touch.type == EV_ABS && touch.code == ABS_Y) y = touch.value*480/600;   //触摸点y坐标
        draw_x=x;
        draw_y=y;
        break;
    }
    return 0;
}

int show_drawdelete()
{
    //打开图片文件
    int bmp = open("/delete.bmp",O_RDWR);
    if(bmp == -1)
    {
        printf("打开文件失败！\n");
        return -1;
    }
    else
    {
        printf("打开文件成功！\n");
    }
   
    //将图片里面的像素点读取出来
    //补充：自动获取图片的长与宽
    int bmp_w,bmp_h;
    lseek(bmp,18,SEEK_SET);//将文件指针偏移到传入字节数处（文件头开始偏移)
    
    read(bmp,&bmp_w,4);
    read(bmp,&bmp_h,4);
    printf("图片的长：%d---宽：%d\n",bmp_w,bmp_h);

    lseek(bmp,54,SEEK_SET);//跳过基本信息域
    int data=bmp_w*bmp_h*3;//存放像素点
    char buf[data];

    int num;//存放每一行需要补的字节数
     //判断宽度是否能被4整除
    if(bmp_w*3%4 == 0)
    {
        num = 0;
    }
    else
    {
        num = 4-(bmp_w*3%4);
    }

    int n;
    for(n=0;n<bmp_h;n++)
    {
        read(bmp,buf+(n*bmp_w*3),bmp_w*3);//读取一行像素点
        //将读取的像素点写入到显存中
        lseek(bmp,num,SEEK_CUR);//将文件指针偏移到传入字节数处
    }

    int x,y;    
    for(y=0;y<bmp_h;y++)
    {
        for(x=0;x<bmp_w;x++)
        {
            if(x>=0 && x<=bmp_w && y>=0 && y<=bmp_h)
            {
                *(mmap_star+800*(479-y)+x) = (buf[3*(bmp_w*y+x)]<<0)+(buf[3*(bmp_w*y+x)+1]<<8)+(buf[3*(bmp_w*y+x)+2]<<16);    
            }
            
        }
    }
    //关闭图片文件
    close(bmp);
    return 0;
}

int show_drawback()
{
    //打开图片文件
    int bmp = open("/drawtui.bmp",O_RDWR);
    if(bmp == -1)
    {
        printf("打开文件失败！\n");
        return -1;
    }
    else
    {
        printf("打开文件成功！\n");
    }
    int bmp_w,bmp_h;
    lseek(bmp,18,SEEK_SET);//将文件指针偏移到传入字节数处（文件头开始偏移)
    
    read(bmp,&bmp_w,4);
    read(bmp,&bmp_h,4);
    printf("图片的长：%d---宽：%d\n",bmp_w,bmp_h);

    lseek(bmp,54,SEEK_SET);//跳过基本信息域
    int data=bmp_w*bmp_h*3;//存放像素点
    char buf[data];

    int num;//存放每一行需要补的字节数
     //判断宽度是否能被4整除
    if(bmp_w*3%4 == 0)
    {
        num = 0;
    }
    else
    {
        num = 4-(bmp_w*3%4);
    }

    int n;
    for(n=0;n<bmp_h;n++)
    {
        read(bmp,buf+(n*bmp_w*3),bmp_w*3);//读取一行像素点
        //将读取的像素点写入到显存中
        lseek(bmp,num,SEEK_CUR);//将文件指针偏移到传入字节数处
    }

    int x,y;    
    for(y=0;y<bmp_h;y++)
    {
        for(x=0;x<bmp_w;x++)
        {
            if(x>=0 && x<=bmp_w && y>=0 && y<=bmp_h)
            {
                *(mmap_star+800*y+x) = (buf[3*(bmp_w*y+x)]<<0)+(buf[3*(bmp_w*y+x)+1]<<8)+(buf[3*(bmp_w*y+x)+2]<<16);    
            }
            
        }
    }
    //关闭图片文件
    close(bmp);
    return 0;
}

int draw_back()//绘制背景
{
    int a1=150,a2=250,a3=350,a4=450,a5=550,a6=650,a7=750;
    int b=430,r=35;
    int x,y;

    //每次刷新之前将屏幕清空
    for(y=0;y<480;y++)
    {
        for(x=0;x<800;x++)
        {
            *(mmap_star+800*y+x) = 0xffffffff;
        }
    }
    show_drawback();
    show_drawdelete();
    for(y=0;y<480;y++)
    {
        for(x=0;x<800;x++)
        { 
            if((x-a1)*(x-a1)+(y-b)*(y-b)<=r*r)//红色圆
            {
                *(mmap_star+800*y+x) = 0xce072f;   
            }
            if((x-a2)*(x-a2)+(y-b)*(y-b)<=r*r)//橙色圆
            {
                *(mmap_star+800*y+x) = 0xf16f2e;   
            }
            if((x-a3)*(x-a3)+(y-b)*(y-b)<=r*r)//黄色圆
            {
                *(mmap_star+800*y+x) = 0xf6e922;
            }
            if((x-a4)*(x-a4)+(y-b)*(y-b)<=r*r)//绿色圆
            {
                *(mmap_star+800*y+x) = 0x2f8f2e;
            }
            if((x-a5)*(x-a5)+(y-b)*(y-b)<=r*r)//青色圆
            {
                *(mmap_star+800*y+x) = 0x2ee0f1;
            }
            if((x-a6)*(x-a6)+(y-b)*(y-b)<=r*r)//黑色圆
            {
                *(mmap_star+800*y+x) = 0x000000;
            }
            if((x-a7)*(x-a7)+(y-b)*(y-b)<=r*r)//紫色圆
            {
                *(mmap_star+800*y+x) = 0x8f2ef1;
            }
        }
    }
    return 0;
}

void draw()
{
    int x,y;
    int w=2;
    draw_back();
    while (1)
    {
        get_draw_xy();
        for(y=0;y<390;y++)
        {
            for(x=0;x<800;x++)
            {
                if(x>=(draw_x-w) && x <=(draw_x+w) && y>=(draw_y-w) && y<=(draw_y+w))
                {
                   *(mmap_star + 800*y+x) = color;
                }
            }
        }
        if(draw_x>=0 && draw_x<100 && draw_y>=380 && draw_y<480)//刷新
        {
            draw_back();
        }
        if(draw_x>=0 && draw_x<100 && draw_y>=0 && draw_y<100)//退出
        {
            printf("退出画笔\n");
            break;
        }
        else if(draw_x>=110 && draw_x<180 && draw_y>=380 && draw_y<480)
        {
            color=0xce072f;
        }
        else if(draw_x>=210 && draw_x<280 && draw_y>=380 && draw_y<480)
        {
            color=0xf16f2e;
        }
        else if(draw_x>=310 && draw_x<380 && draw_y>=380 && draw_y<480)
        {
            color=0xf6e922;
        }
        else if(draw_x>=410 && draw_x<480 && draw_y>=380 && draw_y<480)
        {
            color=0x2f8f2e;
        }
        else if(draw_x>=510 && draw_x<580 && draw_y>=380 && draw_y<480)
        {
            color=0x2ee0f1;
        }
        else if(draw_x>=610 && draw_x<680 && draw_y>=380 && draw_y<480)
        {
            color=0x000000;
        }
        else if(draw_x>=710 && draw_x<780 && draw_y>=380 && draw_y<480)
        {
            color=0x8f2ef1;
        }
    }
    game_choose();
}

int show_gamemulu()
{
    //打开图片文件
    int bmp = open("/gamemulu.bmp",O_RDWR);
    if(bmp == -1)
    {
        printf("打开文件失败！\n");
        return -1;
    }
    else
    {
        printf("打开文件成功！\n");
    }
    int bmp_w,bmp_h;
    lseek(bmp,18,SEEK_SET);//将文件指针偏移到传入字节数处（文件头开始偏移)
    read(bmp,&bmp_w,4);
    read(bmp,&bmp_h,4);
    printf("图片的长：%d---宽：%d\n",bmp_w,bmp_h);
    lseek(bmp,54,SEEK_SET);//跳过基本信息域
    int data=bmp_w*bmp_h*3;//存放像素点
    char buf[data];
    int num;//存放每一行需要补的字节数
    if(bmp_w*3%4 == 0)
    {
        num = 0;
    }
    else
    {
        num = 4-(bmp_w*3%4);
    }
    int n;
    for(n=0;n<bmp_h;n++)
    {
        read(bmp,buf+(n*bmp_w*3),bmp_w*3);//读取一行像素点
        //将读取的像素点写入到显存中
        lseek(bmp,num,SEEK_CUR);//将文件指针偏移到传入字节数处
    }
    int x,y;    
    for(y=0;y<bmp_h;y++)
    {
        for(x=0;x<bmp_w;x++)
        {
            if(x>=0 && x<=bmp_w && y>=0 && y<=bmp_h)
            {
                *(mmap_star+800*(479-y)+x) = (buf[3*(bmp_w*y+x)]<<0)+(buf[3*(bmp_w*y+x)+1]<<8)+(buf[3*(bmp_w*y+x)+2]<<16);    
            }
            
        }
    }
    //关闭图片文件
    close(bmp);
    return 0;
}

int game_choose()
{
    show_gamemulu();
    while (1)
    {
        get_touch_xy();
        if(touch_x>=170 && touch_x<400 && touch_y>=110 && touch_y<260)//游戏一2048
        {
            m2048(0, NULL);
        }
        if(touch_x>=490 && touch_x<600 && touch_y>=110 && touch_y<260)//游戏二画笔
        {
            draw();
        }
        if(touch_x>=610 && touch_x<710 && touch_y>=340 && touch_y<430)//退出按钮
        {
            printf("退出游戏\n");
            break;
        }
    }
    pic_first();
    return 0;
}

int main()
{
    //总分总结构
    //第一步：所有硬件设备初始化
    Dev_Init();
    video_init();

    system("mplayer -slave -input file=/my_fifo -zoom -x 800 -y 480 /huanyin.avi &");   
    usleep(5000000);
    system("killall -SIGKILL mplayer");//播放完开场动画后，关闭mplayer

    //中间：具体功能的实现（由多个函数互相配合实现）
    pic_first();

    //最后一步，关闭所有文件，并释放内存空间
    dev_free();
    return 0;
}


