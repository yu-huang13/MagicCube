//
//  main.cpp
//  repos
//
//  Created by HY on 16/11/15.
//  Copyright © 2016年 HY. All rights reserved.
//

#include <windows.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <glut.h>

#include "vector3.h"

using namespace std;


//保存图片相关参数
int FRAME_WIDTH = 800;
int FRAME_HEIGHT = 800;
//unsigned char colorBuf[FRAME_WIDTH*FRAME_HEIGHT*3];
unsigned char *colorBuf = NULL;
int frameNum = 0;
int countInterval = 0;
const int INTERVAL = 10;
const int BMP_NUM = 1000;
char bmpFilename[50];
char numStr[5];
bool recording = false;

enum CubeColorType {WHITE=0, BLUE=1, ORANGE=2, YELLOW=3, RED=4, GREEN=5, BLACK=6};

GLubyte CubeColorVec[7][3] = {{255, 255, 255}, {0, 0, 255}, {255, 165, 0}, {255, 255, 0}, {255, 0, 0}, {0, 255, 0}, {0, 0, 0}};

enum Surface {FRONT=0, BACK=1, LEFT=2, RIGHT=3, UP=4, DOWN=5};
enum Status {RF, RB, RL, RR, RU, RD, STEADY};				//当前旋转动作
const Surface COLOR_ROTATE[3][2][4] = {
	{{UP, LEFT, DOWN, RIGHT}, {UP, RIGHT, DOWN, LEFT}},		//绕x轴
	{{UP, FRONT, DOWN, BACK}, {UP, BACK, DOWN, FRONT}},		//绕y轴
	{{BACK, LEFT, FRONT, RIGHT}, {BACK, RIGHT, FRONT, LEFT}} //绕z轴
};

const double EPS = 1e-6;
const int CUBE_NUM = 27;
const double CUBE_EDGE = 0.45;				//小方块边长
const double CUBE_DIS = 1;					//方块中心的间距

vector<int> rotatingCube;					//当前正在旋转的方块
Status status = STEADY;						//当前旋转状态
const double ANGLE_STEP = 1;				//一次idle旋转的角度
const int STEP_NUM = 90 / ANGLE_STEP;		//旋转至90度需要的步数
double angle_step = ANGLE_STEP;				//大于0为逆时针旋转，小于0为顺时针旋转
int stepCount = 0;

bool isZero(const double &x){
    return x < EPS && x > -EPS;
}

bool equal(const double &x, const double &y){
    return isZero(x - y);
}

int sign(const double &x){
	return x < 0 ? -1 : 1;
}

struct Position
{
    Position(Vector3 axis, GLdouble r, Vector3 free){
        axisMove = axis;
        rotate = r;
        freeMove = free;
    }
    Position() {}

    Vector3 axisMove;		//轴平移
    GLdouble rotate;		//绕axisMove的旋转角
    Vector3 freeMove;		//旋转后再平移
};

struct SingleCube{
    SingleCube(){
        for (int i = 0; i < 6; ++i)
            color[i] = BLACK;
    }

	void addRotate(GLdouble angle){
        pos.rotate += angle; 
		if (equal(fabs(pos.rotate), 90))
			clearRotate();
    }

	void clearRotate(){//旋转90度后将rotate清零，更新pos.freeMove与color
		if (!isZero(pos.axisMove[0])){
			int flag = sign(pos.rotate) * sign(pos.axisMove[0]);
			updateFree(1, 2, flag < 0? 1 : 2);
			updateColor(COLOR_ROTATE[0][flag > 0? 0 : 1], 4);
		}
		else if  (!isZero(pos.axisMove[1])){
			int flag = sign(pos.rotate) * sign(pos.axisMove[1]);
			updateFree(0, 2, flag < 0? 2 : 0);
			updateColor(COLOR_ROTATE[1][flag > 0? 0 : 1], 4);
		}
		else {
			int flag = sign(pos.rotate) * sign(pos.axisMove[2]);
			updateFree(0, 1, sign(pos.rotate) * pos.axisMove[2] < 0? 0 : 1);
			updateColor(COLOR_ROTATE[2][flag > 0? 0 : 1], 4);
		}	
		pos.rotate = 0;
	}

	void updateFree(int swapr1, int swapr2, int negr){
		pos.freeMove[negr] = -pos.freeMove[negr];
		swap(pos.freeMove[swapr1], pos.freeMove[swapr2]);
	}

	void updateColor(const Surface *order, int len){
		CubeColorType temp = color[order[len-1]];
		for (int i = len-1; i > 0; --i)
			color[order[i]] = color[order[i-1]];
		color[order[0]] = temp;
	}

    Position pos;
    CubeColorType color[6];
};

Vector3 calCoord(const Position &pos){
    return pos.axisMove + pos.freeMove;
}

/*
 俯视图：
 |   0   1   2   |   9   10  11  |   18  19  20  |
 |   3   4   5   |   12  13  14  |   21  22  23  |
 |   6   7   8   |   15  16  17  |   24  25  26  |
 底层             中层             上层
 */
SingleCube cube[CUBE_NUM];

/**
* 初始化魔方
*/
void initCube(){
    
    //底层
    cube[0].pos = Position(Vector3(0, 0, -CUBE_DIS), 0, Vector3(-CUBE_DIS, -CUBE_DIS, 0));
    cube[0].color[DOWN] = WHITE; cube[0].color[BACK] = BLUE; cube[0].color[LEFT] = ORANGE;
    cube[1].pos = Position(Vector3(0, 0, -CUBE_DIS), 0, Vector3(-CUBE_DIS, 0, 0));
    cube[1].color[DOWN] = WHITE; cube[1].color[BACK] = BLUE;
    cube[2].pos = Position(Vector3(0, 0, -CUBE_DIS), 0, Vector3(-CUBE_DIS, CUBE_DIS, 0));
    cube[2].color[DOWN] = WHITE; cube[2].color[BACK] = BLUE; cube[2].color[RIGHT] = RED;

    cube[3].pos = Position(Vector3(0, 0, -CUBE_DIS), 0, Vector3(0, -CUBE_DIS, 0));
    cube[3].color[DOWN] = WHITE; cube[3].color[LEFT] = ORANGE;
    cube[4].pos = Position(Vector3(0, 0, -CUBE_DIS), 0, Vector3(0, 0, 0));
    cube[4].color[DOWN] = WHITE;
    cube[5].pos = Position(Vector3(0, 0, -CUBE_DIS), 0, Vector3(0, CUBE_DIS, 0));
    cube[5].color[DOWN] = WHITE; cube[5].color[RIGHT] = RED;
    
    cube[6].pos = Position(Vector3(0, 0, -CUBE_DIS), 0, Vector3(CUBE_DIS, -CUBE_DIS, 0));
    cube[6].color[DOWN] = WHITE; cube[6].color[LEFT] = ORANGE; cube[6].color[FRONT] = GREEN;
    cube[7].pos = Position(Vector3(0, 0, -CUBE_DIS), 0, Vector3(CUBE_DIS, 0, 0));
    cube[7].color[DOWN] = WHITE; cube[7].color[FRONT] = GREEN;
    cube[8].pos = Position(Vector3(0, 0, -CUBE_DIS), 0, Vector3(CUBE_DIS, CUBE_DIS, 0));
    cube[8].color[DOWN] = WHITE; cube[8].color[RIGHT] = RED; cube[8].color[FRONT] = GREEN;
    
    //中层
    cube[9].pos = Position(Vector3(0, -CUBE_DIS, 0), 0, Vector3(-CUBE_DIS, 0, 0));
    cube[9].color[LEFT] = ORANGE; cube[9].color[BACK] = BLUE;
    cube[10].pos = Position(Vector3(-CUBE_DIS, 0, 0), 0, Vector3(0, 0, 0));
    cube[10].color[BACK] = BLUE;
    cube[11].pos = Position(Vector3(0, CUBE_DIS, 0), 0, Vector3(-CUBE_DIS, 0, 0));
    cube[11].color[BACK] = BLUE; cube[11].color[RIGHT] = RED;
    
    cube[12].pos = Position(Vector3(0, -CUBE_DIS, 0), 0, Vector3(0, 0, 0));
    cube[12].color[LEFT] = ORANGE;
    cube[13].pos = Position(Vector3(0, 0, 0), 0, Vector3(0, 0, 0));
    cube[14].pos = Position(Vector3(0, CUBE_DIS, 0), 0, Vector3(0, 0, 0));
    cube[14].color[RIGHT] = RED;
    
    cube[15].pos = Position(Vector3(0, -CUBE_DIS, 0), 0, Vector3(CUBE_DIS, 0, 0));
    cube[15].color[LEFT] = ORANGE; cube[15].color[FRONT] = GREEN;
    cube[16].pos = Position(Vector3(CUBE_DIS, 0, 0), 0, Vector3(0, 0, 0));
    cube[16].color[FRONT] = GREEN;
    cube[17].pos = Position(Vector3(0, CUBE_DIS, 0), 0, Vector3(CUBE_DIS, 0, 0));
    cube[17].color[RIGHT] = RED; cube[17].color[FRONT] = GREEN;
    
    //上层
    cube[18].pos = Position(Vector3(0, 0, CUBE_DIS), 0, Vector3(-CUBE_DIS, -CUBE_DIS, 0));
    cube[18].color[LEFT] = ORANGE; cube[18].color[BACK] = BLUE; cube[18].color[UP] = YELLOW;
    cube[19].pos = Position(Vector3(0, 0, CUBE_DIS), 0, Vector3(-CUBE_DIS, 0, 0));
    cube[19].color[BACK] = BLUE; cube[19].color[UP] = YELLOW;
    cube[20].pos = Position(Vector3(0, 0, CUBE_DIS), 0, Vector3(-CUBE_DIS, CUBE_DIS, 0));
    cube[20].color[RIGHT] = RED; cube[20].color[BACK] = BLUE; cube[20].color[UP] = YELLOW;
    
    cube[21].pos = Position(Vector3(0, 0, CUBE_DIS), 0, Vector3(0, -CUBE_DIS, 0));
    cube[21].color[LEFT] = ORANGE; cube[21].color[UP] = YELLOW;
    cube[22].pos = Position(Vector3(0, 0, CUBE_DIS), 0, Vector3(0, 0, 0));
    cube[22].color[UP] = YELLOW;
    cube[23].pos = Position(Vector3(0, 0, CUBE_DIS), 0, Vector3(0, CUBE_DIS, 0));
    cube[23].color[RIGHT] = RED; cube[23].color[UP] = YELLOW;
    
    cube[24].pos = Position(Vector3(0, 0, CUBE_DIS), 0, Vector3(CUBE_DIS, -CUBE_DIS, 0));
    cube[24].color[LEFT] = ORANGE; cube[24].color[FRONT] = GREEN; cube[24].color[UP] = YELLOW;
    cube[25].pos = Position(Vector3(0, 0, CUBE_DIS), 0, Vector3(CUBE_DIS, 0, 0));
    cube[25].color[FRONT] = GREEN; cube[25].color[UP] = YELLOW;
    cube[26].pos = Position(Vector3(0, 0, CUBE_DIS), 0, Vector3(CUBE_DIS, CUBE_DIS, 0));
    cube[26].color[RIGHT] = RED; cube[26].color[FRONT] = GREEN; cube[26].color[UP] = YELLOW;
    
}

/**
* 获得指定颜色的RGB值
*/
GLubyte* getColorVec(CubeColorType type){
    return CubeColorVec[type];
}

/**
 * c: 存储正方体的位置和颜色信息
 * edge: 正方体边长
 */
void drawCube(SingleCube &c, double edge){
    
    glPushMatrix();
    glTranslated(c.pos.axisMove.x, c.pos.axisMove.y, c.pos.axisMove.z);
    glRotated(c.pos.rotate, c.pos.axisMove.x, c.pos.axisMove.y, c.pos.axisMove.z);
    glTranslated(c.pos.freeMove.x, c.pos.freeMove.y, c.pos.freeMove.z);
    
    //绘制前表面
    glColor3ubv(getColorVec(c.color[FRONT]));
    glBegin(GL_QUADS);
    glVertex3d(edge, -edge, -edge);
    glVertex3d(edge, edge, -edge);
    glVertex3d(edge, edge, edge);
    glVertex3d(edge, -edge, edge);
    glEnd();
    
    //绘制后表面
    glColor3ubv(getColorVec(c.color[BACK]));
    glBegin(GL_QUADS);
    glVertex3d(-edge, -edge, edge);
    glVertex3d(-edge, edge, edge);
    glVertex3d(-edge, edge, -edge);
    glVertex3d(-edge, -edge, -edge);
    glEnd();
    
    //绘制左表面
    glColor3ubv(getColorVec(c.color[LEFT]));
    glBegin(GL_QUADS);
    glVertex3d(-edge, -edge, -edge);
    glVertex3d(edge, -edge, -edge);
    glVertex3d(edge, -edge, edge);
    glVertex3d(-edge, -edge, edge);
    glEnd();
    
    //绘制右表面
    glColor3ubv(getColorVec(c.color[RIGHT]));
    glBegin(GL_QUADS);
    glVertex3d(-edge, edge, edge);
    glVertex3d(edge, edge, edge);
    glVertex3d(edge, edge, -edge);
    glVertex3d(-edge, edge, -edge);
    glEnd();
    
    //绘制上表面
    glColor3ubv(getColorVec(c.color[UP]));
    glBegin(GL_QUADS);
    glVertex3d(-edge, -edge, edge);
    glVertex3d(edge, -edge, edge);
    glVertex3d(edge, edge, edge);
    glVertex3d(-edge, edge, edge);
    glEnd();
    
    //绘制下表面
    glColor3ubv(getColorVec(c.color[DOWN]));
    glBegin(GL_QUADS);
    glVertex3d(-edge, edge, -edge);
    glVertex3d(edge, edge, -edge);
    glVertex3d(edge, -edge, -edge);
    glVertex3d(-edge, -edge, -edge);
    glEnd();
    
    glFlush();
    glPopMatrix();
}

/*
* 换轴
*/
void changeAxis(Vector3 newAxis, Position &pos){
    pos.freeMove = pos.axisMove + pos.freeMove - newAxis;
    pos.axisMove = newAxis;
}

/*
* 旋转基函数
*/
void rotateBase(int rank, double value, Vector3 newAxis, Status newStatus, int sign){
    for (int i = 0; i < CUBE_NUM; ++i){
        if (equal(calCoord(cube[i].pos)[rank], value)){ //找到需要旋转的9个方块
            changeAxis(newAxis, cube[i].pos);
            rotatingCube.push_back(i);
        }
    }
    angle_step = sign < 0? -ANGLE_STEP : ANGLE_STEP;
    status = newStatus;
    stepCount = 0;
}

void rotateFront(int sign){
    rotateBase(0, CUBE_DIS, Vector3(1, 0, 0), RF, sign);
}

void rotateBack(int sign){
    rotateBase(0, -CUBE_DIS, Vector3(-1, 0, 0), RB, sign);
}

void rotateLeft(int sign){
    rotateBase(1, -CUBE_DIS, Vector3(0, -1, 0), RL, sign);
}

void rotateRight(int sign){
    rotateBase(1, CUBE_DIS, Vector3(0, 1, 0), RR, sign);
}

void rotateUp(int sign){
    rotateBase(2, CUBE_DIS, Vector3(0, 0, 1), RU, sign);
}

void rotateDown(int sign){
    rotateBase(2, -CUBE_DIS, Vector3(0, 0, -1), RD, sign);
}

boolean SaveDIB24(const char* lpszFileName, DWORD dwWidth, DWORD dwHeight, void* lpvBits)
{
	HANDLE hFile;

	BOOL bOK;
	DWORD dwNumWritten;
	DWORD dwWidthAlign;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;

	__try {

		hFile = CreateFile(
			lpszFileName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL
		);
		if (hFile==INVALID_HANDLE_VALUE) return 0;

		dwWidthAlign = ((dwWidth*sizeof(RGBTRIPLE)+3)/4)*4;

		// BITMAPFILEHEADDER
		bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
		bmfh.bfType = ('B'|'M'<<8);
		bmfh.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
		bmfh.bfSize = bmfh.bfOffBits + dwWidthAlign*dwHeight;

		bOK = WriteFile(
			hFile,
			&bmfh,
			sizeof(bmfh),
			&dwNumWritten,
			NULL
		);
		if (!bOK) return 0;


		bmih.biSize = sizeof(BITMAPINFOHEADER);
		bmih.biWidth = dwWidth;
		bmih.biHeight = dwHeight;
		bmih.biPlanes = 1;
		bmih.biBitCount = 24;
		bmih.biCompression = 0;
		bmih.biSizeImage = 0;
		bmih.biXPelsPerMeter = 0;
		bmih.biYPelsPerMeter = 0;
		bmih.biClrUsed = 0;
		bmih.biClrImportant = 0;

		bOK = WriteFile(
			hFile,
			&bmih,
			sizeof(bmih),
			&dwNumWritten,
			NULL
		);
		if (!bOK) return 0;

		bOK = WriteFile(
			hFile,
			lpvBits,
			dwWidthAlign*dwHeight,
			&dwNumWritten,
			NULL
		);
		if (!bOK) return 0;
	
	} __finally {

		CloseHandle(hFile);

	}
	
	return 1;
}

void savePicture(){
	if (++countInterval % INTERVAL)
		return;
	countInterval = 0;
	frameNum++;
	if (frameNum < BMP_NUM)
	{
		cout << "draw frame " << frameNum << endl;
		glReadBuffer(GL_BACK);
		glReadPixels(0, 0, FRAME_WIDTH, FRAME_HEIGHT, GL_BGR_EXT, GL_UNSIGNED_BYTE, colorBuf);
   
		sprintf(numStr,"%03d",frameNum);
		numStr[4] = '\0';
		sprintf(bmpFilename,"%s%s.bmp","../result/",numStr);
		SaveDIB24(bmpFilename,FRAME_WIDTH,FRAME_HEIGHT,colorBuf);
   }
}

void quit(){
	if (colorBuf != NULL)
		delete []colorBuf;
	exit(0);
}

/**
* 键盘事件处理函数
* 按下大写字母为顺时针旋转，小写字母逆时针旋转
*/
void keyboard(unsigned char key, int x, int y){
    if (status != STEADY)
        return;
    switch(key){
        case 'F': rotateFront(-1); break;
        case 'f': rotateFront(1); break;
        case 'B': rotateBack(-1); break;
        case 'b': rotateBack(1); break;
        case 'L': rotateLeft(-1); break;
        case 'l': rotateLeft(1); break;
        case 'R': rotateRight(-1); break;
        case 'r': rotateRight(1); break;
        case 'U': rotateUp(-1); break;
        case 'u': rotateUp(1); break;
        case 'D': rotateDown(-1); break;
        case 'd': rotateDown(1); break;
		case ' ': recording = true; break;
		case 'S':
		case 's': recording = false; break;
		case 27 : quit(); break;		//esc
        default: break;
    }
}

void setPictureSize(int width, int height){
	FRAME_WIDTH = width;
	FRAME_HEIGHT = height;
	if (colorBuf != NULL)
		delete colorBuf;
	colorBuf = new unsigned char[FRAME_WIDTH*FRAME_HEIGHT*3];
}


void reshape(int width, int height)
{
	setPictureSize(width, height);
	glViewport(0,0,width,height);						// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);        //设置透视投影矩阵
    glLoadIdentity();                   //初始化为单位阵
    gluPerspective(40, (GLfloat)width/(GLfloat)height, 1, 20);       //可视角，w/h，zNear，zFar
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(10, 10, 10, 0, 0, 0, -1, -1, 1);        //eye，center，up
}

/**
* 绘制事件处理函数
*/
void drawMagicCube(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for(int i = 0; i < CUBE_NUM; ++i)
        drawCube(cube[i], CUBE_EDGE);
     
    glFlush();
}

/**
* 空闲处理函数
*/
void magicCubeIdle(){
    if (status != STEADY){
        for (vector<int>::iterator it = rotatingCube.begin(); it != rotatingCube.end(); ++it)
            cube[*it].addRotate(angle_step);
        if (++stepCount == STEP_NUM){
            status = STEADY;
            rotatingCube.clear();
        }
    }
    drawMagicCube();
	if (recording)
		savePicture();
	glutSwapBuffers();
}


int main(int argc, char * argv[]) {
    initCube();
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(0, 0);
	glutInitWindowSize(FRAME_WIDTH, FRAME_HEIGHT);
    glutCreateWindow("Magic Cube");
	glutFullScreen();
    
    glEnable(GL_DEPTH_TEST);				// Enables Depth Testing
    glEnable(GL_CULL_FACE);
    
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);   //red, green, blue, alpha
    
	glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(drawMagicCube);
    glutIdleFunc(magicCubeIdle);

    glutMainLoop();
    quit();
}


