/***************************************************************
	###���Կ����ֻ�
	*���ߣ�cendeal
	*ʱ�䣺2017/10/10
	*�汾��1.2
	*ԭ��
		����adb��input,��ͼָ�ʹ��c����
	*��ʵ�ֹ��ܣ�
		1.������Ϊ�����¼���֧�ֳ��� ��֧�ֻ���������δʵ����ק
		2.����ҵ��Ϊ���أ�����Ϊpower���Ѻ�˯��
		3.����up���ϻ���������down���»���
		4.���ֵ���Ϊhome,�ɳ���home
		5.��ʵʱ������Ļ
		6.���Զ���ȡ��Ļ����������Ӧ������Ļ
	*����ȱ�㣺
		1.��ʱ��
		2.�������⣬Ŀǰ��ȫ֧��Android4.4.2����Ҫ���ҵ��ֻ�����û������������
		3.�������δ֪�������������߳����⣬���ʲ���
********************************************************************/
#include <graphics.h>
#include<windows.h>
#include <stdio.h>
#include<time.h>
#include<process.h>
#include<string.h>
#include<stdlib.h>
#include<vector>
#define FILESIZE 1000
#define OPENERROR 1
#define SUCEESS 0
std::vector<PIMAGE> v;
//��ȡ���Ը߶�
int win_height=GetSystemMetrics(SM_CYFULLSCREEN);
double screen_pi=2;
//���½���
int getImage(PIMAGE &image);

//Ϊ�����ش��ڵ���adb
void mySystem(const char* cmd, const char*  par);

//������Ի���ͼƬ
int deletLocalCachePic();

//����ֻ�����ͼƬ
void deletCachePicFromPhone();

//���»���
void updateCache();

//��׽ͼ
unsigned WINAPI Catpic(void *arg);

//˵��
void aboutApp(int width);

//��ȡphone��widthxheight
bool repareDeviceWidthAndHeight(int &width,int &height);

//��ʼ��
void init(int width,int height,const char *appName);

//��Ӧ����
void respongYourRequset(int height);

///////////////////////////��������ʼ/////////////////////////
int main()
{
	int width;
	int height;
	deletLocalCachePic();
	if(!repareDeviceWidthAndHeight(width,height)) {
		MessageBox(GetConsoleWindow(),"��ʧ�ܣ������������ȷ��usb�򿪣�","��ʾ!",0);
		int isok=MessageBox(GetConsoleWindow(),"������usb���Դ򿪰���","usb���԰�����",1);
		if(isok==IDOK)
			WinExec("explorer .\\res\\usbhelp.html",SW_HIDE);
		return OPENERROR;
	}
	init(width,height,"�ؿ��ҵ�phone");
	getch();
	PIMAGE resizeImage = newimage();
	getImage(resizeImage);
	putimage(0,0,resizeImage);

	HANDLE handle=(HANDLE)_beginthreadex(NULL,0,Catpic,(void*)resizeImage,0,NULL);

	respongYourRequset(height);

	WaitForSingleObject(handle, 1000);
	//closegraph();
	return SUCEESS;
}
//////////////////////////����������//////////////////////////

int getImage(PIMAGE &image)
{
	PIMAGE pimg=newimage();
	if(getimage(pimg,".\\res\\1.png")==0) {
		PIMAGE resizeImage = newimage(getwidth(pimg) / screen_pi, getheight(pimg) / screen_pi);
		putimage(resizeImage, 0, 0, getwidth(resizeImage), getheight(resizeImage), pimg, 0, 0, getwidth(pimg), getheight(pimg));
		delimage(pimg);
		image=resizeImage;
		return OPENERROR;
	}
	return SUCEESS;
}


void mySystem(const char* cmd, const char*  par)
{
	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = cmd;//���õĳ�����
	ShExecInfo.lpParameters = par;//���ó���������в���
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;//����״̬Ϊ����
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess,INFINITE);////�ȵ��ý��̽���
}

int deletLocalCachePic()
{
	return remove(".\\res\\1.png");
}

void deletCachePicFromPhone()
{
	mySystem(".\\tool\\adb"," shell rm /sdcard/android/1.png");
}

void updateCache()
{
	mySystem(".\\tool\\adb"," shell screencap -p /sdcard/android/1.png");
	mySystem(".\\tool\\adb"," pull /sdcard/android/1.png .\\res");

}

unsigned WINAPI Catpic(void *arg)
{
	while(is_run()) {

		updateCache();
		api_sleep(15);
		PIMAGE resizeImage=(PIMAGE)arg;
		getImage(resizeImage);
		putimage(0,0,resizeImage);
	}

	deletCachePicFromPhone();
	deletLocalCachePic();
	return SUCEESS;
}

void aboutApp(int width)
{
	FILE *about=fopen(".\\res\\tip.txt","r");
	char carr[FILESIZE];
	//setbkcolor(YELLOW);
	setfont(25,0,"����");

	//bar(width/10,width/5,width/10+50,width/5+50);
	for(int i=0; (fscanf(about,"%s",carr))!=EOF; i++) {
		outtextxy(width/10+10,width/5+i*30,carr);
	}

}

bool repareDeviceWidthAndHeight(int &width,int &height)
{
	//���ڻ�ȡ�ֻ���Ļ�ߴ磬һ��Ϊ��ͨ����
	if(system(".\\tool\\adb shell wm size >.\\res\\phone.info")!=0)
		return false;
	FILE *f=fopen(".\\res\\phone.info","r");
	if(f==NULL) {
		return false;
	}
	fseek(f,0L,SEEK_SET);
	char c,w[5],h[5];
	bool flags=false;
	int index=0;//Ѱַ����
	while((c=fgetc(f))!=EOF) {
		if(c>='0'&&c<='9') {
			if(!flags) {
				w[index++]=c;
			} else {
				h[index++]=c;
			}
		}
		if(c=='x') {
			flags=true;
			index=0;
		}
	}
	width=atoi(w);
	height=atoi(h);
	fclose(f);
	if(width==0||height==0) {

		//��һ���޷���ȡ��Ļ����������µķ�ʽ
		if(system(".\\tool\\adb shell \"dumpsys window | grep mUnrestrictedScreen\" >.\\res\\phone.info")==0) {
			f=fopen(".\\res\\phone.info","r");
			if(f==NULL) {
				return false;
			}
			fseek(f,0L,SEEK_SET);
			memset(w,'\0',sizeof(w));
			memset(h,'\0',sizeof(w));
			flags=false;
			bool canread=false; //���˷�����
			index=0;
			while((c=fgetc(f))!=EOF) {
				if(c==')') {
					canread=true;
				}
				if(!canread)
					continue;
				if(c>='0'&&c<='9') {
					if(!flags) {
						w[index++]=c;
					} else {
						h[index++]=c;
					}
					printf("%c",c);
				}
				if(c=='x') {
					flags=true;
					index=0;
				}
			}
			width=atoi(w);
			height=atoi(h);
			fclose(f);
		} else {
			return false;
		}

	}

	//������Ļ��С
	if(height>win_height) {
		screen_pi=(double)height/win_height;
	}
	return true;
}
void respongYourRequset(const int height)
{
	const char commandinput[30]=".\\tool\\adb shell input";
	bool action=false;
	clock_t start, finish;
	bool longpress=false,swipe=false;
	double duration=0;
	int lx1,ly1,lx2,ly2;

	for ( ; is_run(); ) {
		flushmouse();
		mouse_msg msg= {0};
		char str[100];
		int x, y;
		msg = getmouse();
		if(msg.is_down()) {
			start=clock();

			setcolor(EGERGB(0xb1, 0xb2, 0xb8));
			setfillcolor(EGERGB(0xb1, 0xb2, 0xb8));
			mousepos(&lx1,&ly1);
			circle(lx1,ly1,height/100);
			floodfill(lx1, ly1, EGERGB(0xb1, 0xb2, 0xb8));

			action=false;
			longpress=false;
			swipe=false;
		}
		if(msg.is_up()) {
			finish=clock();
			setcolor(EGERGB(0xb1, 0xb2, 0xb8));
			setfillcolor(EGERGB(0xb1, 0xb2, 0xb8));
			mousepos(&lx2,&ly2);
			line(lx1, ly1,lx2,ly2);
			circle(lx2,ly2,height/100);
			floodfill(lx2, ly2, EGERGB(0xb1, 0xb2, 0xb8));

			duration=(double)(finish-start)/1000;
			if(duration>1) {
				longpress=true;
			} else {
				longpress=false;
			}
			int dx=lx2-lx1;
			int dy=ly2-ly1;
			if(dx>0||dy>0) {
				swipe=true;
			} else {
				swipe=false;
			}
			action=true;
		}
		if(!action) {
			continue;
		}
		//���
		if(msg.is_left()) {
			if(longpress&&!swipe) {

				//����
				sprintf(str, "%s swipe %4f %4f %4f %4f %4d",commandinput, lx1*screen_pi, ly1*screen_pi,lx1*screen_pi, ly1*screen_pi,(long long)(duration*1000));
				WinExec(str,SW_HIDE);
			} else if(!longpress&&swipe) {
				sprintf(str, "%s swipe %4f %4f %4f %4f %4d",commandinput, lx1*screen_pi, ly1*screen_pi,lx2*screen_pi, ly2*screen_pi,130);
				WinExec(str,SW_HIDE);
			} else {
				//���==click
				mousepos(&x, &y);
				sprintf(str, "%s tap %4f %4f",commandinput,  x*screen_pi, y*screen_pi);
				WinExec(str,SW_HIDE);
			}
		}

		//�һ�
		if(msg.is_right()) {
			if(longpress&&!swipe) {
				sprintf(str, "%s keyevent  26",commandinput);
				WinExec(str,SW_HIDE);
			} else {
				//back
				sprintf(str, "%s keyevent 4",commandinput);
				WinExec(str,SW_HIDE);
			}
		}

		//����==up and down
		if(msg.is_wheel()) {
			if(msg.wheel==120) {

				//down
				mousepos(&x, &y);
				sprintf(str, "%s swipe %4f %4f %4f %4f",commandinput, x*screen_pi, y*screen_pi,x*screen_pi, y*screen_pi+height/screen_pi);
				WinExec(str,SW_HIDE);
			}
			if(msg.wheel==-120) {

				//up
				mousepos(&x, &y);
				sprintf(str, "%s swipe %4f %4f %4f %4f",commandinput, x*screen_pi, y*screen_pi,x*screen_pi, y*screen_pi-height/screen_pi);
				WinExec(str,SW_HIDE);
			}
		}

		//��==longpress home
		if(msg.is_mid()) {
			if(longpress) {
				sprintf(str, "%s  keyevent --longpress 3",commandinput );
				WinExec(str,SW_HIDE);
			} else {
				sprintf(str, "%s  keyevent  3",commandinput);
				WinExec(str,SW_HIDE);
				getmouse();
			}
		}
	}

}

void init(int width,int height,const char *appName)
{
	initgraph(width/screen_pi, height/screen_pi,INIT_NOFORCEEXIT);
	LPCSTR lpStr = appName;
	setcaption(lpStr);
	aboutApp(width);
	updateCache();
}

