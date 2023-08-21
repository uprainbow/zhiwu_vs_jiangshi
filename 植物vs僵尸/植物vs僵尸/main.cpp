/*
1.�زĵ���
2.�ʼ����Ϸ����
3.����������
4.�������е�ֲ����ʾ
*/
#include<stdio.h>
#include<graphics.h>//easyxͼ�ο��ͷ�ļ�
#include<time.h>
#include "tools.h"
#include<math.h>
#include<mmsystem.h>//��Чͷ�ļ�
#include"vector2.h"//���տ���̫������
#pragma comment(lib,"winmm.lib")

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum { WAN_DOU_SHE_SHOU, XIANG_RI_KUI, DA_ZUI_HUA, ZHI_WU_COUNT };//���� ZHI_WU_COUNT����enum��ֱ�Ӷ�̬��ʾֲ�￨������

IMAGE imgBg;//��ʾ����ͼƬ
IMAGE imgBar;//����ͼƬ����
IMAGE imgCards[ZHI_WU_COUNT];//����
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];//ֲ������
IMAGE imgZmStand[11];//��ʬվ��

int curX, curY;//��ǰѡ�е�ֲ����ƶ������е�λ��
int curZhiWu=0;//�϶�������Ⱦ�����ĸ�ֲ�� 0��û��ѡ�� 1����һ��ֲ��
int sunshine;//����ֵ

struct zhiwu {//������
	int type; //0��û��ֲ��
	int frameIndex;//����֡�����

	bool catched;//�Ƿ񱻽�ʬ����
	int deadTime;//��������ʱ

	int timer;//��ʱ��������
	int x, y;//ֲ���λ��
};
struct zhiwu map[3][9];//x:250-1000 y:180-490  һ������ x:83.3333 y:103.333333

enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT };//����״̬  �����½�  ��������  �����ռ�  ��������

struct sunshineBall {//������
	int x, y;//����Ʈ������е�����λ�ô�ֱ���� x����
	int frameIndex;//��ǰ��ʾ��������֡�����  Ŀ��������ת
	int destY;//Ʈ���Ŀ��λ�õ�y����
	bool used;//�б��ڳ������Ƿ���ʹ��  false��ʾ����ʹ��
	int timer;//��ʱ��   ͣ����ʱ��

	float xoff;
	float yoff;

	float t;//���������ߵ�ʱ���0..1
	vector2 p1, p2, p3, p4;//��� �յ� �������Ƶ�
	vector2 pCur;//��ǰʱ���������λ��
	float speed;//��������ٶ�
	int status;
};

struct zm {//��ʬ
	int x, y;//ֱ����  y����
	int frameIndex;
	bool used;
	int speed;
	int row;
	int blood;//Ѫ��
	bool dead;//��ʬ������״̬
	bool eating;//���ڳ�ֲ���״̬
};
IMAGE imgZM[22];
struct zm zms[10];
IMAGE imgZMDead[20];
IMAGE imgZMEat[21];

//�ӵ������ݽṹ
struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;//�Ƿ��䱬ը
	int frameIndex;//��ը֡���
};
struct bullet bullets[30];//�ӵ���
IMAGE imgBulletNormal;

IMAGE imgBullBlast[4];//��ըͼƬ֡

//���Ӽ���  �����
struct sunshineBall balls[10];
IMAGE imgSunshineBall[29];//�������ͼƬ֡

bool fileExist(const char *filename) {
	FILE* fp = fopen(filename, "r");
	if (fp) {
		fclose(fp);
		return true;
	}
	return false;
}

void gameInit() {//��Ϸ��ʼ��
	//���ر���ͼƬ
	//���ַ����޸�Ϊ"���ֽ�"
	loadimage(&imgBg,"res/bg.jpg");
	loadimage(&imgBar,"res/bar5.png");

	memset(imgZhiWu, 0, sizeof(imgZhiWu));
	memset(map, 0, sizeof(map));
	//��ʼ��ֲ�￨��
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		//����ֲ�￨�Ƶ��ļ���
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i,j+1);
			//���ж�����ļ��Ƿ����
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
		loadimage(&imgSunshineBall[i],name);//��������֡
	}

	srand(time(NULL));//�����������

	//������Ϸ��ͼ�δ���
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

	//��������
	LOGFONT font;
	gettextstyle(&font);
	font.lfHeight = 30;
	font.lfWidth = 15;
	strcpy(font.lfFaceName, "Segoe UI Black");
	font.lfQuality = ANTIALIASED_QUALITY;//�����Ч��
	settextstyle(&font);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);

	//��ʬ��ʼ��
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZM[i], name);//���뽩ʬ����֡
	}

	//�ӵ���ʼ��
	memset(bullets, 0, sizeof(bullets));
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");

	//�ӵ���ըͼƬ֡
	loadimage(&imgBullBlast[3],"res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++) {
		float k = (i + 1)*0.2;
		loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png", imgBullBlast[3].getwidth() * k, imgBullBlast[3].getheight() * k, true);
	}

	//��ʬ����ͼƬ֡��ʼ��
	for (int i = 0; i < 20; i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZMDead[i],name);
	}

	//��ʬ��ֲ��ͼƬ֡��ʼ��
	for (int i = 0; i < 21; i++) {
		sprintf_s(name, sizeof(name), "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZMEat[i], name);
	}

	//Ƭ����ʬվ��
	for (int i = 0; i < 11; i++) {
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);
		loadimage(&imgZmStand[i], name);
	}
}

void updateWindow() {
	BeginBatchDraw();//��ʼ����

	//ͼƬ����
	putimage(0, 0, &imgBg);
	putimagePNG(300, 0, &imgBar);
	for (int i = 0,dx=384,dy=6; i < ZHI_WU_COUNT; i++,dx+=65) {
		putimage(dx, dy, &imgCards[i]);
	}

	//���ú������ֲ��
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				//int x = 250 + 83.333*j, y = 180 + 103.333*i;
				putimagePNG(map[i][j].x, map[i][j].y,imgZhiWu[map[i][j].type-1][map[i][j].frameIndex]);
			}
		}
	}

	//��Ⱦ�϶������е�ֲ��
	if (curZhiWu > 0) {
		//�϶�����
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, imgZhiWu[curZhiWu - 1][0]);
	}

	//��������Ⱦ
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y,&imgSunshineBall[balls[i].frameIndex]);
		}
	}

	//������
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(332, 67, scoreText);//��ʾ����ֵ

	//��ʬ��Ⱦ
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmMax; i++) {
		if (zms[i].used) {
			if (zms[i].dead)//����״̬
				putimagePNG(zms[i].x, zms[i].y, &imgZMDead[zms[i].frameIndex]);
			else if (zms[i].eating)//��ֲ��״̬
				putimagePNG(zms[i].x, zms[i].y, &imgZMEat[zms[i].frameIndex]);
			else//����״̬
				putimagePNG(zms[i].x, zms[i].y , &imgZM[zms[i].frameIndex]);
		}
	}

	//�㶹�ӵ���Ⱦ
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {//�㶹�ӵ���ը��Ⱦ
				IMAGE* img = &imgBullBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else {//�����ӵ���Ⱦ
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
	EndBatchDraw();//����˫����
}

void collectSunshine(ExMessage *msg) {//�ռ�����
	int count = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			//����Ĵ�С x:15-55   y:20-55
			int x = balls[i].pCur.x, y = balls[i].pCur.y;
			if (msg->x >= x + 10 && msg->x <= x + 60 && msg->y >= y + 15 && msg->y <= y + 60) {
				
				balls[i].status = SUNSHINE_COLLECT;
				//sunshine += 25;
				PlaySound("res/sunshine.wav", NULL, SND_FILENAME | SND_ASYNC);//��Ч

				//����������ƫ����
				//float desty = 0;
				//float destx = 262;
				//float angle = atan((balls[i].y - desty) / (balls[i].x - destx));//�Ƕ�
				//balls[i].xoff = 15 * cos(angle);
				//balls[i].yoff = 15 * sin(angle);

				balls[i].p1 = balls[i].pCur;
				balls[i].p4 = vector2(262, 0);
				balls[i].t = 0;
				float distance = dis(balls[i].p1 - balls[i].p4);
				float off = 8;//Խ��Խ��
				balls[i].speed = 1.0 / (distance / off);
				break;
			}
		}
	}
}
void userClick() {
	ExMessage msg;
	static int status = 0;//״̬����
	if (peekmessage(&msg)) {
		if (msg.message== WM_LBUTTONDOWN) {//����������ȥ
			if (msg.x > 384 && msg.x < 384 + 65 * ZHI_WU_COUNT && msg.y>6 && msg.y < 90) {
				int index = (msg.x - 384) / 65;//��ǰֲ���ڿ��Ʋ��е�λ��
				status = 1;//�϶�״̬
				curZhiWu = index + 1;
			}
			else{//�ռ�����
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1) {//����ƶ�
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP) {//���̧����
			//������ �൱�ڴ���һ��ֲ��
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
	if (count >= fre) {//ÿcount֡����һ������
		fre = 500 + rand() % 100;//200-400
		count = 0;
		//���������ȡһ������ʹ�õ�
		int ballMax = sizeof(balls) / sizeof(balls[0]);//���ٸ�����
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
		balls[i].p1 = vector2(260 + rand() % (900 - 260), 60);//���
		balls[i].p4 = vector2(balls[i].p1.x, 120 + rand() % (500 - 120));//�յ�
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;
		balls[i].speed = 1.0 / (distance / off);
	}

	//���տ���������
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == XIANG_RI_KUI + 1) {
				map[i][j].timer++;
				if (map[i][j].timer > 700) {
					map[i][j].timer = 0;

					int k;
					for (k = 0; k < ballMax && balls[k].used; k++);
					if (k >= ballMax) return;//����������ⶼ������

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
				if (sun->t >= 1) {//��½��
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
				if (sun->t > 1) {//�ɵ��յ�����ʧ
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
			//	if (balls[i].y >= balls[i].destY) {//�䵽Ŀ��λ��
			//		balls[i].timer++;//��ʱ����ʼ��ʱ
			//		if (balls[i].timer++ > 200) {//����ʱ���� ʱ����������ʧ
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
			zms[i].row = rand() % 3;//0-2��
			zms[i].y = 130 + zms[i].row * ((490 - 180) / 3);//130-440
			zms[i].blood = 100;
			zms[i].eating = false;
		}
	}
}

void updateZm() {
	static int count1 = 0, count2 = 0;
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	//���½�ʬ��λ��
	count1++;
	if (count1 > 3) {
		count1 = 0;
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				if (zms[i].x <= 155) {
					//ֲ��g
					MessageBox(NULL, "������ӱ��Ե��ˣ�����", "over", 0);
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
				if (zms[i].dead) {//��ʬ������ͼƬ֡��Ÿı�
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20) {
						zms[i].used = false;
						zms[i].dead = false;
					}
				}
				else if (zms[i].eating) {//��ʬ���ڳ�ֲ��
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
				}
				else {//��ʬ����״̬��ͼƬ֡��Ÿı�
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
				}
			}
		}
	}
}

void shoot() {
	int lines[3] = { 0 };//��¼3���Ƿ��н�ʬ
	int zmCount = sizeof(zms) / sizeof(zms[0]);//��ʬ����
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);//�ӵ�����
	int dangerX = WIN_WIDTH - imgZM[0].getwidth()/2;//��Ļ��ȼ�һ����ʬ��λ�ſ�ʼ���
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used && zms[i].x < dangerX) {
			lines[zms[i].row] = 1;
		}
	}

	for (int i = 0; i < 3; i++) {//3��9��
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == WAN_DOU_SHE_SHOU + 1 && lines[i]) {//���Է�����
				static int count_shoot = 0;
				count_shoot++;
				if (count_shoot > 160) {//ÿ ֡����
					count_shoot = 0;
					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);//�ҵ�����ʹ�õ��ӵ�
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

			if (bullets[i].x > WIN_WIDTH) {//�㶹�ӵ�����
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
		if (bullets[i].used == false || bullets[i].blast)continue;//����ӵ�û�û����Ѿ���ը�˾Ͳ��ü����

		for (int k = 0; k < zmCount; k++) {
			if (zms[k].used == false || zms[k].dead) continue;//û�������ʬ�����Ѿ����˾Ͳ��ü����ײ��
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
					//��ʱ��ʬ�Ѿ����� �����������ʬ�ͱ���ӵ������
					break;
				}

			}
		}
	}
}

void checkZm_zhiwu() {
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zCount; i++) {
		if (zms[i].used == false || zms[i].dead == true) continue;//��ʬ���˻�û�������ʬ

		int row = zms[i].row;//��齩ʬͬ�е�ֲ��
		for (int j = 0; j < 9; j++) {//9��
			if (map[row][j].type == 0)continue;//û��ֲ������
			int ZhiWuX = 250 + 83.333*j, ZhiWuY = 180 + 103.333*i;
			int x1 = ZhiWuX + 10, x2 = ZhiWuX + 60, x3 = zms[i].x + 80;
			if (x3 > x1 && x3 < x2) {//x1---x2����ֲ���ܱ��Եķ�Χ
				if (map[row][j].catched) {//���ڱ���
					map[row][j].deadTime++;
					if (map[row][j].deadTime > 130) {
						map[row][j].type = 0;//ֲ����ʧ
						zms[i].eating = false;
						zms[i].frameIndex = 0;
						zms[i].speed = 1;//
					}
				}
				else {//����
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
	checkBullet_Zm();//��ʬ���ӵ�����ײ
	checkZm_zhiwu();//��ʬ��ֲ�����ײ
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
	createSunshine();//��������
	updateSunshine();//��������״̬

	createZm();//������ʬ
	updateZm();//���½�ʬ״̬

	shoot();//�����㶹�ӵ�
	updateBullets();//�����ӵ�λ��

	collisionCheck();//��ײ���
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
				flag = 1;//ת��ͼƬ
			}
			else if (msg.message == WM_LBUTTONUP && flag==1) {
				EndBatchDraw();
				break;//̧����귵�ز���������������һ������
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
	gameInit();//���ݳ�ʼ��
	startUI();//��Ϸ��ʼ����
	//viewScence();//��ϷѲ��
	int timer = 0;
	bool flag = true;
	//����
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