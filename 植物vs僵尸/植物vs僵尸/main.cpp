/*
1.素材导入
2.最开始的游戏场景
3.顶部工具栏
4.工具栏中的植物显示
*/
#include<stdio.h>
#include<graphics.h>//easyx图形库的头文件
#include<time.h>
#include "tools.h"
#include<math.h>
#include<mmsystem.h>//音效头文件
#include"vector2.h"//向日葵产太阳抛物
#pragma comment(lib,"winmm.lib")

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum { WAN_DOU_SHE_SHOU, XIANG_RI_KUI, DA_ZUI_HUA, ZHI_WU_COUNT };//技巧 ZHI_WU_COUNT利用enum来直接动态显示植物卡牌数量

IMAGE imgBg;//表示背景图片
IMAGE imgBar;//工具图片变量
IMAGE imgCards[ZHI_WU_COUNT];//卡槽
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];//植物跳舞
IMAGE imgZmStand[11];//僵尸站立

int curX, curY;//当前选中的植物，在移动过程中的位置
int curZhiWu=0;//拖动过程渲染的是哪个植物 0：没有选择 1：第一个植物
int sunshine;//阳光值

struct zhiwu {//种下来
	int type; //0：没有植物
	int frameIndex;//序列帧的序号

	bool catched;//是否被僵尸捕获
	int deadTime;//死亡倒计时

	int timer;//计时器产阳光
	int x, y;//植物的位置
};
struct zhiwu map[3][9];//x:250-1000 y:180-490  一个格子 x:83.3333 y:103.333333

enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT };//阳光状态  阳光下降  阳光着落  阳光收集  阳光生产

struct sunshineBall {//阳光球
	int x, y;//阳光飘落过程中的坐标位置垂直下落 x不变
	int frameIndex;//当前显示的土拍你帧的序号  目的让他旋转
	int destY;//飘落的目标位置的y坐标
	bool used;//判别在池子中是否在使用  false表示现无使用
	int timer;//定时器   停留的时间

	float xoff;
	float yoff;

	float t;//贝塞尔曲线的时间点0..1
	vector2 p1, p2, p3, p4;//起点 终点 两个控制点
	vector2 pCur;//当前时刻阳光球的位置
	float speed;//阳光球的速度
	int status;
};

struct zm {//僵尸
	int x, y;//直线走  y不变
	int frameIndex;
	bool used;
	int speed;
	int row;
	int blood;//血量
	bool dead;//僵尸死亡的状态
	bool eating;//正在吃植物的状态
};
IMAGE imgZM[22];
struct zm zms[10];
IMAGE imgZMDead[20];
IMAGE imgZMEat[21];

//子弹的数据结构
struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;//是否发射爆炸
	int frameIndex;//爆炸帧序号
};
struct bullet bullets[30];//子弹池
IMAGE imgBulletNormal;

IMAGE imgBullBlast[4];//爆炸图片帧

//池子技术  阳光池
struct sunshineBall balls[10];
IMAGE imgSunshineBall[29];//阳光球的图片帧

bool fileExist(const char *filename) {
	FILE* fp = fopen(filename, "r");
	if (fp) {
		fclose(fp);
		return true;
	}
	return false;
}

void gameInit() {//游戏初始化
	//加载背景图片
	//把字符集修改为"多字节"
	loadimage(&imgBg,"res/bg.jpg");
	loadimage(&imgBar,"res/bar5.png");

	memset(imgZhiWu, 0, sizeof(imgZhiWu));
	memset(map, 0, sizeof(map));
	//初始化植物卡牌
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		//生成植物卡牌的文件名
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i,j+1);
			//先判断这个文件是否存在
			if (fileExist(name)) {
				imgZhiWu[i][j] = new IMAGE;
				loadimage(imgZhiWu[i][j], name);
			}
			else
				break;
		}
	}

	curZhiWu = 0;
	sunshine = 50;

	memset(balls,false,sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name,sizeof(name),"res/sunshine/%d.png",i+1);
		loadimage(&imgSunshineBall[i],name);//存入阳光帧
	}

	srand(time(NULL));//配置随机种子

	//创建游戏的图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

	//设置字体
	LOGFONT font;
	gettextstyle(&font);
	font.lfHeight = 30;
	font.lfWidth = 15;
	strcpy(font.lfFaceName, "Segoe UI Black");
	font.lfQuality = ANTIALIASED_QUALITY;//抗锯齿效果
	settextstyle(&font);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);

	//僵尸初始化
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZM[i], name);//存入僵尸数据帧
	}

	//子弹初始化
	memset(bullets, 0, sizeof(bullets));
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");

	//子弹爆炸图片帧
	loadimage(&imgBullBlast[3],"res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++) {
		float k = (i + 1)*0.2;
		loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png", imgBullBlast[3].getwidth() * k, imgBullBlast[3].getheight() * k, true);
	}

	//僵尸死亡图片帧初始化
	for (int i = 0; i < 20; i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZMDead[i],name);
	}

	//僵尸吃植物图片帧初始化
	for (int i = 0; i < 21; i++) {
		sprintf_s(name, sizeof(name), "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZMEat[i], name);
	}

	//片场僵尸站立
	for (int i = 0; i < 11; i++) {
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);
		loadimage(&imgZmStand[i], name);
	}
}

void updateWindow() {
	BeginBatchDraw();//开始缓冲

	//图片坐标
	putimage(0, 0, &imgBg);
	putimagePNG(300, 0, &imgBar);
	for (int i = 0,dx=384,dy=6; i < ZHI_WU_COUNT; i++,dx+=65) {
		putimage(dx, dy, &imgCards[i]);
	}

	//放置后跳舞的植物
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				//int x = 250 + 83.333*j, y = 180 + 103.333*i;
				putimagePNG(map[i][j].x, map[i][j].y,imgZhiWu[map[i][j].type-1][map[i][j].frameIndex]);
			}
		}
	}

	//渲染拖动过程中的植物
	if (curZhiWu > 0) {
		//拖动过程
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, imgZhiWu[curZhiWu - 1][0]);
	}

	//阳光球渲染
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y,&imgSunshineBall[balls[i].frameIndex]);
		}
	}

	//阳光数
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(332, 67, scoreText);//显示阳光值

	//僵尸渲染
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmMax; i++) {
		if (zms[i].used) {
			if (zms[i].dead)//死亡状态
				putimagePNG(zms[i].x, zms[i].y, &imgZMDead[zms[i].frameIndex]);
			else if (zms[i].eating)//吃植物状态
				putimagePNG(zms[i].x, zms[i].y, &imgZMEat[zms[i].frameIndex]);
			else//正常状态
				putimagePNG(zms[i].x, zms[i].y , &imgZM[zms[i].frameIndex]);
		}
	}

	//豌豆子弹渲染
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {//豌豆子弹爆炸渲染
				IMAGE* img = &imgBullBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else {//正常子弹渲染
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
	EndBatchDraw();//结束双缓冲
}

void collectSunshine(ExMessage *msg) {//收集阳光
	int count = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			//阳光的大小 x:15-55   y:20-55
			int x = balls[i].pCur.x, y = balls[i].pCur.y;
			if (msg->x >= x + 10 && msg->x <= x + 60 && msg->y >= y + 15 && msg->y <= y + 60) {
				
				balls[i].status = SUNSHINE_COLLECT;
				//sunshine += 25;
				PlaySound("res/sunshine.wav", NULL, SND_FILENAME | SND_ASYNC);//音效

				//设置阳光球偏移量
				//float desty = 0;
				//float destx = 262;
				//float angle = atan((balls[i].y - desty) / (balls[i].x - destx));//角度
				//balls[i].xoff = 15 * cos(angle);
				//balls[i].yoff = 15 * sin(angle);

				balls[i].p1 = balls[i].pCur;
				balls[i].p4 = vector2(262, 0);
				balls[i].t = 0;
				float distance = dis(balls[i].p1 - balls[i].p4);
				float off = 8;//越大越快
				balls[i].speed = 1.0 / (distance / off);
				break;
			}
		}
	}
}
void userClick() {
	ExMessage msg;
	static int status = 0;//状态变量
	if (peekmessage(&msg)) {
		if (msg.message== WM_LBUTTONDOWN) {//鼠标左键按下去
			if (msg.x > 384 && msg.x < 384 + 65 * ZHI_WU_COUNT && msg.y>6 && msg.y < 90) {
				int index = (msg.x - 384) / 65;//当前植物在卡牌槽中的位置
				status = 1;//拖动状态
				curZhiWu = index + 1;
			}
			else{//收集阳光
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1) {//鼠标移动
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP) {//鼠标抬起来
			//种下来 相当于创建一个植物
			if (msg.x >= 250 && msg.x <= 1000 && msg.y >= 180 && msg.y <= 490) {
				int column = (msg.x - 250) / 83.333, row = (msg.y - 180) / 103.333;
				if (map[row][column].type == 0) {

					map[row][column].type = curZhiWu;
					map[row][column].frameIndex = 0;
					map[row][column].deadTime = 0;
					map[row][column].catched = false;

					//int x = 250 + 83.333*j, y = 180 + 103.333*i;
					map[row][column].x = 250 + 83.333*column;
					map[row][column].y = 180 + 103.333*row;
				}
			}
			curZhiWu = 0;
		}
	}
}

void createSunshine() {
	static int count = 0;
	static int fre = 500;
	count++;
	if (count >= fre) {//每count帧创建一个阳光
		fre = 500 + rand() % 100;//200-400
		count = 0;
		//从阳光池中取一个可以使用的
		int ballMax = sizeof(balls) / sizeof(balls[0]);//多少个阳光
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax)return;
		balls[i].used = true;
		balls[i].frameIndex = 0;
		//balls[i].x = 270 + rand() % (850 - 260);//260-850
		//balls[i].y = 60;
		//balls[i].destY = 120 + rand() % (500 - 120);//120-500
		balls[i].timer = 0;
		//balls[i].xoff = 0;
		//balls[i].yoff = 0;

		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(260 + rand() % (900 - 260), 60);//起点
		balls[i].p4 = vector2(balls[i].p1.x, 120 + rand() % (500 - 120));//终点
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;
		balls[i].speed = 1.0 / (distance / off);
	}

	//向日葵生产阳光
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == XIANG_RI_KUI + 1) {
				map[i][j].timer++;
				if (map[i][j].timer > 700) {
					map[i][j].timer = 0;

					int k;
					for (k = 0; k < ballMax && balls[k].used; k++);
					if (k >= ballMax) return;//池子里的阳光都被用了

					balls[k].used = true;
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);
					int w = (100 + rand() % 50)*(rand() % 2 ? 1 : -1);
					balls[k].p4 = vector2(map[i][j].x + w, map[i][j].y + imgZhiWu[XIANG_RI_KUI][0]->getheight() - imgSunshineBall[0].getheight());
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);
					balls[k].status = SUNSHINE_PRODUCT;
					balls[k].speed = 0.05;
					balls[k].t = 0;
				}
			}
		}
	}
}

void updateSunshine() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			if (balls[i].status == SUNSHINE_DOWN) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t*(sun->p4 - sun->p1);
				if (sun->t >= 1) {//着陆了
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND) {
				balls[i].timer++;
				if (balls[i].timer > 100) {
					balls[i].used = false;
					balls[i].timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t*(sun->p4 - sun->p1);
				if (sun->t > 1) {//飞到终点了消失
					sun->used = false;
					sunshine += 25;
				}
			}
			else if (balls[i].status == SUNSHINE_PRODUCT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}

			//	balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			//	if (balls[i].timer == 0) {
			//		balls[i].y += 4;
			//	}
			//	if (balls[i].y >= balls[i].destY) {//落到目标位置
			//		balls[i].timer++;//定时器开始即时
			//		if (balls[i].timer++ > 200) {//当计时器到 时再让阳光消失
			//			balls[i].used = false;
			//		}
			//	}
			//}
			//else if (balls[i].xoff) {
			//	balls[i].x -= balls[i].xoff;
			//	balls[i].y -= balls[i].yoff;
			//	if (balls[i].y < 0 || balls[i].x < 262) {
			//		balls[i].xoff = 0;
			//		balls[i].yoff = 0;
			//		sunshine += 25;
			//	}
		}
	}
}

void createZm() {
	static  int zmFre=400;
	static int count = 0;
	count++;
	if (count > zmFre) {
		count = 0;
		zmFre = rand() % 400 + 350;
		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);
		for (i = 0; i < zmMax && zms[i].used; i++);
		if (i < zmMax) {
			zms[i].used = true;
			zms[i].dead = false;
			zms[i].x = WIN_WIDTH;
			zms[i].speed = 1;
			zms[i].row = rand() % 3;//0-2行
			zms[i].y = 130 + zms[i].row * ((490 - 180) / 3);//130-440
			zms[i].blood = 100;
			zms[i].eating = false;
		}
	}
}

void updateZm() {
	static int count1 = 0, count2 = 0;
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	//更新僵尸的位置
	count1++;
	if (count1 > 3) {
		count1 = 0;
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				if (zms[i].x <= 155) {
					//植物g
					MessageBox(NULL, "你的脑子被吃掉了！！！", "over", 0);
					exit(0);
				}
			}
		}
	}
	count2++;
	if (count2 > 4) {
		count2 = 0;
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				if (zms[i].dead) {//僵尸死亡的图片帧序号改变
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20) {
						zms[i].used = false;
						zms[i].dead = false;
					}
				}
				else if (zms[i].eating) {//僵尸正在吃植物
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
				}
				else {//僵尸行走状态的图片帧序号改变
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
				}
			}
		}
	}
}

void shoot() {
	int lines[3] = { 0 };//记录3行是否有僵尸
	int zmCount = sizeof(zms) / sizeof(zms[0]);//僵尸个数
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);//子弹个数
	int dangerX = WIN_WIDTH - imgZM[0].getwidth()/2;//屏幕宽度减一个僵尸身位才开始射击
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used && zms[i].x < dangerX) {
			lines[zms[i].row] = 1;
		}
	}

	for (int i = 0; i < 3; i++) {//3行9列
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == WAN_DOU_SHE_SHOU + 1 && lines[i]) {//可以发射了
				static int count_shoot = 0;
				count_shoot++;
				if (count_shoot > 160) {//每 帧发射
					count_shoot = 0;
					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);//找到可以使用的子弹
					if (k < bulletMax) {
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 3;

						bullets[k].blast = false;
						bullets[k].frameIndex = 0;

						//int x = 250 + 83.333*j, y = 180 + 103.333*i;
						int zwX = 250 + 83.333*j;
						int zwY= 180 + 103.333*i;
						bullets[k].x = zwX + imgZhiWu[map[i][j].type - 1][0]->getwidth() - 10;
						bullets[k].y = zwY+5;
					}
				}
			}
		}
	}
}

void updateBullets() {
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;

			if (bullets[i].x > WIN_WIDTH) {//豌豆子弹回收
				bullets[i].used = false;
			}

			if (bullets[i].blast) {
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4) {
					bullets[i].used = false;
				}
			}
		}
	}
}

void checkBullet_Zm() {
	int bCount = sizeof(bullets) / sizeof(bullets[0]);
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < bCount; i++) {
		if (bullets[i].used == false || bullets[i].blast)continue;//如果子弹没用或者已经爆炸了就不用检测了

		for (int k = 0; k < zmCount; k++) {
			if (zms[k].used == false || zms[k].dead) continue;//没用这个僵尸或者已经死了就不用检测碰撞了
			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			int x = bullets[i].x;
			if (bullets[i].row == zms[k].row && x > x1 && x < x2) {
				zms[k].blood -= 10;
				bullets[i].blast = true;
				bullets[i].speed = 0;

				if (zms[k].blood <= 0) {
					zms[k].dead = true;
					zms[k].speed = 0;
					zms[k].frameIndex = 0;
					//此时僵尸已经死了 无需让这个僵尸和别的子弹检测了
					break;
				}

			}
		}
	}
}

void checkZm_zhiwu() {
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zCount; i++) {
		if (zms[i].used == false || zms[i].dead == true) continue;//僵尸死了或没有这个僵尸

		int row = zms[i].row;//检查僵尸同行的植物
		for (int j = 0; j < 9; j++) {//9列
			if (map[row][j].type == 0)continue;//没有植物跳过
			int ZhiWuX = 250 + 83.333*j, ZhiWuY = 180 + 103.333*i;
			int x1 = ZhiWuX + 10, x2 = ZhiWuX + 60, x3 = zms[i].x + 80;
			if (x3 > x1 && x3 < x2) {//x1---x2是能植物能被吃的范围
				if (map[row][j].catched) {//正在被吃
					map[row][j].deadTime++;
					if (map[row][j].deadTime > 130) {
						map[row][j].type = 0;//植物消失
						zms[i].eating = false;
						zms[i].frameIndex = 0;
						zms[i].speed = 1;//
					}
				}
				else {//开吃
					map[row][j].catched = true;
					map[row][j].deadTime = 0;
					zms[i].eating = true;
					zms[i].speed = 0;
					zms[i].frameIndex = 0;
				}
			}
		}
	}
}

void collisionCheck() {
	checkBullet_Zm();//僵尸与子弹的碰撞
	checkZm_zhiwu();//僵尸与植物的碰撞
}
void updateGame() {
	static int count_zhiwu = 0;
	count_zhiwu++;
	if (count_zhiwu > 2) {
		count_zhiwu = 0;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 9; j++) {
				if (map[i][j].type > 0) {
					map[i][j].frameIndex++;
					int ZhiWuType = map[i][j].type - 1;
					int index = map[i][j].frameIndex;
					if (imgZhiWu[ZhiWuType][index] == NULL) {
						map[i][j].frameIndex = 0;
					}
				}
			}
		}
	}
	createSunshine();//创建阳光
	updateSunshine();//更新阳光状态

	createZm();//创建僵尸
	updateZm();//更新僵尸状态

	shoot();//发射豌豆子弹
	updateBullets();//更新子弹位置

	collisionCheck();//碰撞检测
}

void startUI() {
	IMAGE imgBg,imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");

	int flag = 0;

	while (1) {
		BeginBatchDraw();
		putimage(0,0,&imgBg);
		putimagePNG(470, 70, flag?&imgMenu1:&imgMenu2);

		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN && msg.x > 470 && msg.x < 470 + 300 && msg.y>70 && msg.y < 70 + 140) {
				flag = 1;//转换图片
			}
			else if (msg.message == WM_LBUTTONUP && flag==1) {
				EndBatchDraw();
				break;//抬起鼠标返回并将缓冲区的数据一并发出
			}
		}
		EndBatchDraw();
	}
}

void viewScence() {
	int xMin = WIN_WIDTH - imgBg.getwidth();
	vector2 points[9] = { {550,412},{570,512},{530,512},{670,521},{650,561},{690,312},{610,142},{560,24},{550,53} };
	for (int x = 0; x >= -500; x -= 2) {
		BeginBatchDraw();
		putimage(x, 0, &imgBg);
		EndBatchDraw();
		Sleep(5);
	}
}

int main(void) {
	gameInit();//数据初始化
	startUI();//游戏开始界面
	//viewScence();//游戏巡场
	int timer = 0;
	bool flag = true;
	//窗口
	while (1) {
		userClick();
		timer += getDelay();
		if (timer > 10) {
			flag = true;
			timer = 0;
		}
		if (flag) {
			flag = false;
			updateWindow();
			updateGame();
		}
	}
	system("pause");
	return 0;
}