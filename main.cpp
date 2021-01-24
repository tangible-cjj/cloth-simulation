#ifndef GLUT_DISABLE_ATEXIT_HACK  
#define GLUT_DISABLE_ATEXIT_HACK  
#endif  
#define GLEW_STATIC  
#include <GL/glew.h>  
#include <GL/wglew.h>  
#include <GL/freeglut.h>  
#include <vector>  
#include <cstring>  
#include <glm/glm.hpp>  
#include <glm/gtc/matrix_transform.hpp> //for matrices  
#include <glm/gtc/type_ptr.hpp>  
#include "GLTools.h"
#include "common\OpenGLSB.h"
#include "common\GLTools.h"
#include "common\LoadTGA.c"
#include "common\VectorMath.c"
#include <iostream>  
using namespace std;
#define WIDTH         600
#define HEIGHT        600

const int GRID_SIZE = 5;  //地板砖边长  
//判断悬挂点
bool isfix[1000];
bool lineandpoint = false;
int sign =   1;
int IsMove = 0;
int IsWind = 1;
//球体参数  
float radius =  1;

const int numX = 20, numY = 20; //一行有numx+1个点  
const int total_points = (numX + 1) * (numY + 1); //总点数  
int w_min = 1, w_max = 5;

//布料顶点位置 速度  
typedef struct
{
    glm::vec3 Pos[total_points];
    glm::vec3 Veloc[total_points];
    glm::vec3 force[total_points];
    glm::mat4 ellipsoid, invreseEllipsoid;
}Cloth;

Cloth cloth;

glm::vec3 Wind;
float Wind_coefficient = 0.05f;

int Size = 4;
float hsize = Size / 2.0f;
const float frameTime = 1.0f / 60.0f;
const float mass = 1.0 / total_points;
const float globalDamp = 0.98;  //速度衰减参数  
const glm::vec3 g = glm::vec3(0, -9.8, 0);  //重力加速度  
const float Spring_K = 2.5;  //弹性系数  
const float len0 = 4.0 / numX; //单边长度  
float set;//球体运动  
glm::mat4 scene;
//关于键盘以及鼠标相应
//键盘
float lrangles, udangles, ioangles;
float leri_tran, updo_tran, inot_tran;
//鼠标
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraDistanceX;
float cameraDistanceY;
float cameraAngleX;
float cameraAngleY;
float times = 1;
GLfloat LightAmbient[] = { 0.2f,0.2f,0.2f,0.2f };                // 环境光参数        
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };                 // 漫射光参数            
GLfloat LightPosition[] = { 1.0f, 1.0f, 1.0f, 0.0f };
GLfloat LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
unsigned int texture1, texture2, texture0;
GLbyte* pBytes;
GLbyte* pBytes2;
GLint iWidth, iHeight, iComponents;
GLenum eFormat;
GLint iWidth2, iHeight2, iComponents2;
GLenum eFormat2;
GLbyte* pByte3;
GLint iWidth3, iHeight3, iComponents3;
GLenum eFormat3;
GLfloat red[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
GLfloat white[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat cloFRONT[4] = { 0.8f, 0.0f, 1.0f ,0.0f };
GLfloat cloBACK[4] = { 1.0f, 1.0f, 0.0f ,0.0f };
glm::mat4 SSS;
bool cut = false;
void Initvariable()
{
    lrangles = 0.0f; udangles = 0; ioangles = 0.0f;
    leri_tran = 0.0f; updo_tran = -4.0f, inot_tran = -8.0f;
    IsMove = 0; IsWind = 1;
    mouseX = 0.0f; mouseY = 0.0f;
    cameraDistanceX = 0.0f; cameraDistanceY = 0.0f;
    cameraAngleX = 0.0f; cameraAngleY = 0.0f;
    times = 1; sign = 1;
    w_min = 1; w_max = 5;
    lineandpoint = false;
}

void Inter()
{
    scene = glm::scale(SSS, glm::vec3(times));
    glTranslatef(cameraDistanceX, cameraDistanceY, 0);
    glRotatef(cameraAngleX, 1, 0, 0);
    glRotatef(cameraAngleY, 0, 1, 0);
    glRotatef(lrangles, 0.0, 1.0, 0.0);
    glRotatef(udangles, 1.0, 0.0, 0.0);
    glRotatef(ioangles, 0.0, 0.0, 1.0);
    glTranslatef(leri_tran, updo_tran, inot_tran);
}

int zero(int sign)
{
    if (sign == 1) return 0;
    return 1;
}

void initGL()
{

    //初始化顶点位置  
    memset(cloth.Pos  , 0, sizeof(cloth.  Pos));
    memset(cloth.Veloc, 0, sizeof(cloth.Veloc));
    memset(cloth.force, 0, sizeof(cloth.force));
    Initvariable();
    //fill in positions  
    int count1 = 0;
    for (int j = 0; j <= numY; j++) {
        for (int i = 0; i <= numX; i++) {
            cloth.Pos[count1++] = glm::vec3(((float(i) / (numX - 1)) * 2 - 1) * hsize, Size + 1, ((float(j) / (numY)) * Size));
        }
        //悬挂点为X[0] 和 X[numX]  
    }
    memset(isfix, 0, sizeof(isfix));
    //悬挂点设置
    isfix[0] = isfix[numX] = 1;
    isfix[total_points - 1] = 1;
    isfix[total_points - 1 - numY] = 1;
    set = 2;

    glLightfv(GL_LIGHT0, GL_POSITION, LightPosition); //指定第0号光源的位置 
    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient); //GL_AMBIENT表示各种光线照射到该材质上，
                                                          //经过很多次反射后最终遗留在环境中的光线强度（颜色）
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse); //漫反射后~~
    glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);//镜面反射后~~~

    glEnable(GL_LIGHT0); //使用第0号光照
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //**********************************创建纹理********************************************
    pBytes = gltLoadTGA("grass.tga", &iWidth, &iHeight, &iComponents, &eFormat);
    pBytes2 = gltLoadTGA("floor.tga", &iWidth2, &iHeight2, &iComponents2, &eFormat2);
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, iComponents2, iWidth2, iHeight2, 0, eFormat2, GL_UNSIGNED_BYTE, pBytes2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    pByte3 = gltLoadTGA("stone.tga", &iWidth3, &iHeight3, &iComponents3, &eFormat3);
    glGenTextures(1, &texture0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    glTexImage2D(GL_TEXTURE_2D, 0, iComponents3, iWidth3, iHeight3, 0, eFormat3, GL_UNSIGNED_BYTE, pByte3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

//画球体并设置球体移动
void DrawEllipsoid() {

    cloth.ellipsoid = glm::translate(glm::mat4(1), glm::vec3(0, 2.0f, set)); 

    set += 0.01 * sign * IsMove;
    if (set >= 4.0) sign *= -1;
    else if (set <= -2.0) sign *= -1;
    cloth.invreseEllipsoid = glm::inverse(cloth.ellipsoid);
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(cloth.ellipsoid));
    static GLUquadricObj* sphere = gluNewQuadric();
    glEnable(GL_LIGHTING);
    glMaterialfv(GL_FRONT, GL_AMBIENT, red);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
    glMaterialfv(GL_FRONT, GL_SPECULAR, white);
    glMaterialf(GL_FRONT, GL_SHININESS, 32.0f);
    glEnable(GL_CULL_FACE);
    gluSphere(sphere, radius, 48, 24);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glPopMatrix();
}

//画地板 
void DrawGrid()
{

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f((float)-GRID_SIZE, 0, (float)-GRID_SIZE);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f((float)GRID_SIZE, 0, (float)-GRID_SIZE);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f((float)GRID_SIZE, 0, (float)GRID_SIZE);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f((float)-GRID_SIZE, 0, (float)GRID_SIZE);
    glEnd();
    glFlush();
    glDisable(GL_TEXTURE_2D);//关闭二维纹理贴图
}

//画布料  
void DrawTextile() {
    int k = 0;
    int t = 0;
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glMaterialfv(GL_FRONT, GL_AMBIENT, cloBACK);	//set material
    glMaterialfv(GL_FRONT, GL_DIFFUSE, cloBACK);
    glBindTexture(GL_TEXTURE_2D, texture0);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i <= numX; i++) {
        for (int j = 0; j < numY; j++) {
            glTexCoord2f(0.0f + j / (numY * 1.0), 1.0f - i / (numX * 1.0));
            glVertex3f(cloth.Pos[t].x, cloth.Pos[t].y, cloth.Pos[t].z);
            glTexCoord2f(0.0f + (j + 1) / (numY * 1.0), 1.0f - i / (numX * 1.0));
            glVertex3f(cloth.Pos[t + 1].x, cloth.Pos[t + 1].y, cloth.Pos[t + 1].z);

            glTexCoord2f(0.0f + j / (numY * 1.0), 1.0f - (i + 1) / (numX * 1.0));
            glVertex3f(cloth.Pos[t + numX + 1].x, cloth.Pos[t + numX + 1].y, cloth.Pos[t + numX + 1].z);
            glTexCoord2f(0.0f + j / (numY * 1.0), 1.0f - (i + 1) / (numX * 1.0));
            glVertex3f(cloth.Pos[t + numX + 1].x, cloth.Pos[t + numX + 1].y, cloth.Pos[t + numX + 1].z);

            glTexCoord2f(0.0f + (j + 1) / (numY * 1.0), 1.0f - i / (numX * 1.0));
            glVertex3f(cloth.Pos[t + 1].x, cloth.Pos[t + 1].y, cloth.Pos[t + 1].z);

            glTexCoord2f(0.0f + (j + 1) / (numY * 1.0), 1.0f - (i + 1) / (numX * 1.0));
            glVertex3f(cloth.Pos[t + numX + 2].x, cloth.Pos[t + numX + 2].y, cloth.Pos[t + numX + 2].z);

            t++;
        }
        t = (numX + 1) * i;
    }
    glEnd();    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);//关闭二维纹理贴图
    if (lineandpoint) {
        glBegin(GL_LINES);
        glColor3f(0.8f, 0.0f, 1.0f);
        for (int i = 0; i <= numX; i++) {
            for (int j = 0; j <= numY; j++) {
                if (j != numX) {
                    glVertex3f(cloth.Pos[k].x, cloth.Pos[k].y, cloth.Pos[k].z);
                    glVertex3f(cloth.Pos[k + 1].x, cloth.Pos[k + 1].y, cloth.Pos[k + 1].z);
                }
                if (i != numY) {
                    glVertex3f(cloth.Pos[k].x, cloth.Pos[k].y, cloth.Pos[k].z);
                    glVertex3f(cloth.Pos[k + numX + 1].x, cloth.Pos[k + numX + 1].y, cloth.Pos[k + numX + 1].z);
                }
                if (i != numY && j != numX)
                {
                    glVertex3f(cloth.Pos[k].x, cloth.Pos[k].y, cloth.Pos[k].z);
                    glVertex3f(cloth.Pos[k + numX + 2].x, cloth.Pos[k + numX + 2].y, cloth.Pos[k + numX + 2].z);
                    glVertex3f(cloth.Pos[k + 1].x, cloth.Pos[k + 1].y, cloth.Pos[k + 1].z);
                    glVertex3f(cloth.Pos[k + numX + 1].x, cloth.Pos[k + numX + 1].y, cloth.Pos[k + numX + 1].z);
                }
                k++;
            }
        }
        glEnd();

        glPointSize(3);
        glBegin(GL_POINTS);
        glColor3f(255, 128, 0);
        for (size_t i = 0; i < total_points; i++) {
            glVertex3f(cloth.Pos[i].x, cloth.Pos[i].y, cloth.Pos[i].z);
        }
        glEnd();
    }
}

void OnRender(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    //设置视角  
    Inter();
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(scene));
    //做出地面  
    DrawGrid();
    DrawTextile();
    DrawEllipsoid();
    glutSwapBuffers();
    glPopMatrix();
}

//求出b点对a点的作用力  
glm::vec3 SpringForce(int a, int b, int is) {
    float inilen;
    if (is == 1)inilen = len0;
    if (is == 2) inilen = len0 * 2;
    if (is == 3) inilen = len0 * 1.414213;
    glm::vec3 res = glm::vec3(0);
    glm::vec3 tmp = cloth.Pos[b] - cloth.Pos[a];
    float dis = glm::length(tmp);
    res = tmp / dis;
    res *= (dis - inilen);
    res *= Spring_K;
    return res;
}

void Calculate_Cloth()
{
    int i, j, k = 0;
    //结构弹簧 上下左右  
    for (i = 0; i <= numY; i++) {
        for (j = 0; j <= numX; j++) {
            if (i != 0) {//上  
                cloth.force[k] += SpringForce(k, k - numX - 1, 1);
            }
            if (j != numX) {//右  
                cloth.force[k] += SpringForce(k, k + 1, 1);
            }
            if (i != numY) {//下  
                cloth.force[k] += SpringForce(k, k + numX + 1, 1);
            }
            if (j != 0) {//左  
                cloth.force[k] += SpringForce(k, k - 1, 1);
            }
            k++;
        }
    }
    //柔性弹簧  
    k = 0;
    for (i = 0; i <= numY; i++) {
        for (j = 0; j <= numX; j++) {
            if (i > 1) {//上  
                cloth.force[k] += SpringForce(k, k - 2 * numX - 2, 2);
            }
            if (j < numX - 1) {//右  
                cloth.force[k] += SpringForce(k, k + 2, 2);
            }
            if (i < numY - 1) {//下  
                cloth.force[k] += SpringForce(k, k + 2 * numX + 2, 2);
            }
            if (j > 1) {//左  
                cloth.force[k] += SpringForce(k, k - 2, 2);
            }
            k++;
        }
    }
    //剪切弹簧  
    k = 0;
    for (i = 0; i <= numY; i++) {
        for (j = 0; j <= numX; j++) {
            if (i > 0 && j > 0) {  //左上  
                cloth.force[k] += SpringForce(k, k - numX - 2, 3);
            }
            if (i > 0 && j < numX) {  //右上  
                cloth.force[k] += SpringForce(k, k - numX, 3);
            }
            if (i < numY && j < numX) {  //右下  
                cloth.force[k] += SpringForce(k, k + numX + 2, 3);
            }
            if (i < numY && j>0) {   //坐下  
                cloth.force[k] += SpringForce(k, k + numX, 3);
            }

            k++;
        }
    }
}


//重力影响
void Calculate_Gravity() {
    //重力  
    for (size_t i = 0; i < total_points; i++) {
        cloth.force[i] = glm::vec3(0);
        if (!isfix[i])
            cloth.force[i] += mass * g;
    }
}

float tb_Rnd(int min, int max) {
    float number;
    number = (((abs(rand()) % (max - min + 1)) + min));
    if (number > max) {
        number = max;
    }

    if (number < min) {
        number = min;
    }

    return number / 100.0;
}

//风力影响
void Calculate_WindForce(int c)
{
    if (c == 0) {
        Wind = { 0,-tb_Rnd(0,1),tb_Rnd(1,5) };
        Wind = glm::normalize(Wind);
        Wind_coefficient = tb_Rnd(w_min, w_max);
        for (size_t i = total_points/2.5; i < total_points; i++) {
            cloth.force[i] = glm::vec3(0);
            
            if (!isfix[i]) {
                cloth.force[i] += Wind * Wind_coefficient;
            }
        }
    }
}

//检测与处理布料和椭球之间的碰撞  
void EllipsoidCollision() {

    for (size_t i = 0; i < total_points; i++) {
        glm::vec3 tPos(cloth.Pos[i]);
        glm::vec4 aaa = (cloth.invreseEllipsoid * glm::vec4(tPos, 1));
        glm::vec3 ddd = glm::vec3(aaa.x, aaa.y, aaa.z);
        float distance = glm::length(ddd)-0.05f;
        if (distance < 1.0f) {
            ddd = (radius - distance) * ddd / distance;
            glm::vec3 delta;
            glm::vec3 trans;
            trans = glm::vec3(cloth.ellipsoid[0].x, cloth.ellipsoid[1].x, cloth.ellipsoid[2].x);
            trans /= glm::dot(trans, trans);
            delta.x = glm::dot(ddd, trans);
            trans = glm::vec3(cloth.ellipsoid[0].y, cloth.ellipsoid[1].y, cloth.ellipsoid[2].y);
            trans /= glm::dot(trans, trans);
            delta.y = glm::dot(ddd, trans);
            trans = glm::vec3(cloth.ellipsoid[0].z, cloth.ellipsoid[1].z, cloth.ellipsoid[2].z);
            trans /= glm::dot(trans, trans);
            delta.z = glm::dot(ddd, trans);
            tPos += delta;
            cloth.Veloc[i] += (tPos - cloth.Pos[i]) / frameTime;
            cloth.Veloc[i] *= globalDamp;
            cloth.Pos[i] = tPos;
        }
    }
}

//检测与处理布料和地面之间的碰撞
void SceneCollision()
{
    for (size_t i = 0; i < total_points; i++)
    {
        //在地板之外
        if (cloth.Pos[i].x< -GRID_SIZE || cloth.Pos[i].x>GRID_SIZE
            || cloth.Pos[i].z< -GRID_SIZE || cloth.Pos[i].z>GRID_SIZE);
        else if (cloth.Pos[i].y <= 0.1f)
        {
            cloth.Pos[i].y = 0.1f;
            cloth.Veloc[i].y = 0;
            cloth.force[i] -= mass * g;
        }
    }
}
//计算新的位置  
void CalcPos() { 
    glm::vec3 acc = glm::vec3(0);
    for (size_t i = 0; i < total_points; i++) {
        if (isfix[i])continue;
        acc = cloth.force[i] / mass;  //得到加速度向量  
        cloth.Veloc[i] = cloth.Veloc[i] + acc * frameTime;  //得到新的速度值  
        cloth.Veloc[i] *= globalDamp;
        cloth.Pos[i] = cloth.Pos[i] + cloth.Veloc[i] * frameTime;
    }
}

void StepPhysics() {
    
    Calculate_Gravity();
    Calculate_WindForce(IsWind);
    Calculate_Cloth();
    EllipsoidCollision();
    SceneCollision();
    CalcPos();
    glutPostRedisplay();
    Sleep(10);
}

void OnReshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (GLfloat)w / (GLfloat)h, 1, GRID_SIZE*100.0);
    glMatrixMode(GL_MODELVIEW);
}

//wadsec 控制视角移动
//7      控制球是否移动
//nmvb   将悬挂点解除
//0      初始化
//4862   控制视角旋转
//93     场景缩放
//1      开启风力
//+-     增减风力
//.      开启布料线条显示
//*/     更换地面纹理
void KeyboardCB(unsigned char Key,int x,int y)
{
    switch (Key) {
    case 27:
        exit(0);
        break;
    case 'w':
        updo_tran += 0.5f;
        break;
    case 's':
        updo_tran -= 0.5f;
        break;        
    case 'a':
        leri_tran += 0.5f;
        break;
    case 'd':
        leri_tran -= 0.5f;
        break;
    case 'e':
        inot_tran += 0.5f;
        break;
    case 'c':
        inot_tran -= 0.5f;
        break;
    case 'n':
        isfix[0] = 0;
        break;
    case 'm':
        isfix[numX] = 0;
        break;
    case 'v':
        isfix[total_points - 1 - numY] = 0;
        break;
    case 'b':
        isfix[total_points - 1] = 0;
        break;
    case '7':
        IsMove = zero(IsMove);
        break;
    case '1':
        IsWind = zero(IsWind);
        break;
    case '+':
        if (w_max < 10)
        {
            w_min++;
            w_max++;
        }
        break;
    case '-':
        if (w_min >= 0)
        {
            w_min--;
            w_max--;
        }
        break;
    case '8':
        udangles += 0.5f;
        break;
    case '2':
        udangles -= 0.5f;
        break;
    case '4':
        lrangles += 0.5f;
        break;
    case '6':
        lrangles -= 0.5f;
        break;
    case '9':
        times += 0.08f;
        break;
    case '3':
        times -= 0.08f;
        break;
    case '0':
    {
        lrangles = 0.0f; udangles = 0.0f; ioangles = 0.0f;
        leri_tran = 0.0f; updo_tran = -4.0f, inot_tran = -12.0f;
        IsMove = 1; IsWind = 1;
        mouseX = 0.0f; mouseY = 0.0f;
        cameraDistanceX = 0.0f;
        cameraDistanceY = 0.0f;
        cameraAngleX = 0.0f;
        cameraAngleY = 0.0f;
        times = 1;
        initGL();
        sign = 1;
        break;
    }
    case '/':
        glBindTexture(GL_TEXTURE_2D, texture1);
        glTexImage2D(GL_TEXTURE_2D, 0, iComponents2, iWidth2, iHeight2, 0, eFormat2, GL_UNSIGNED_BYTE, pBytes2);
        glutPostRedisplay();
        break;
    case '*':
        glBindTexture(GL_TEXTURE_2D, texture1);
        glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
        glutPostRedisplay();
        break;
    case '.':
        if(lineandpoint) lineandpoint=false;
        else lineandpoint = true;
        break;
    default:
        break;
    }
}
//鼠标左键控制视角移动
//鼠标右键控制视角旋转
void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if (state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)
    {
        
        if (state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if (state == GLUT_UP)
            mouseRightDown = false;
    }
}

void mouseMotionCB(int x, int y)
{


    if (mouseRightDown)
    {
        cameraAngleY += (x - mouseX) * 0.1f;
        cameraAngleX += (y - mouseY) * 0.1f;
        mouseX = x;
        mouseY = y;
    }
    if (mouseLeftDown)
    {
        cameraDistanceX += (x - mouseX) * 0.015f;
        cameraDistanceY += -(y - mouseY) * 0.015f;
        mouseY = y;
        mouseX = x;
    }

    glutPostRedisplay();
}

void mysubmenu(int value)                                 //子菜单函数
{
    switch (value)
    {
    case 1:
        cout << "键盘功能：" << endl;
        cout << "0      初始化" << endl;
        cout << "wadsec 控制视角移动" << endl;
        cout << "7      控制球是否移动" << endl;
        cout << "vbnm   将左右悬挂点解除" << endl;
        cout << "4862   控制视角旋转" << endl;
        cout << "93     场景缩放" << endl;
        cout << "1      开关风力" << endl;
        cout << "+-     增减风力" << endl;
        cout << ".      开启布料线条显示"<<endl;
        cout << "*/     更换地面纹理" << endl;
        cout << "鼠标功能：" << endl;
        cout << "鼠标左键控制视角移动" << endl;
        cout << "鼠标右键控制视角旋转" << endl;
        cout << "单点鼠标滚轮键召唤菜单" << endl;
        break;
    case 2:
        initGL();
        break;
    case 4:
        exit(1);
        break;
    }
}

void mymenu(int value)
{
    switch (value)
    {
    case 1:
        IsWind = 0;
        break;
    case 2:
        IsWind = 1;
        break;
    }
}

void Createmenu()
{
    //创建子菜单
    int id1 = glutCreateMenu(mymenu);
    glutAddMenuEntry("OpenWind", 1);
    glutAddMenuEntry("CloseWind", 2);
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
    //创建顶级菜单
    int id = glutCreateMenu(mysubmenu);
    glutAddMenuEntry("Function introduction", 1);
    glutAddMenuEntry("Init", 2);
    glutAddSubMenu("Wind", id1);
    glutAddMenuEntry("exit", 4);
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(500, 200);
    glutCreateWindow("18281233_曹家骏");
    cout << "点击鼠标滚轮键可召唤出菜单" << endl;
    initGL();
    glutDisplayFunc(OnRender);
    Createmenu();
    glutReshapeFunc(OnReshape);
    glutIdleFunc(StepPhysics);
    glEnable(GL_DEPTH_TEST);
    glutKeyboardFunc(KeyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);
    glutMainLoop();
    return 0;
}