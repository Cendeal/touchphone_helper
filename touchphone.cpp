/***************************************************************
	###电脑控制手机
	*作者：cendeal
	*时间：2017/10/10
	*版本：1.2
	*原理：
		利用adb的input,截图指令，使用c语言
	*已实现功能：
		1.鼠标左击为触点事件，支持长按 ，支持滑动，但是未实现拖拽
		2.鼠标右点击为返回，长按为power唤醒和睡眠
		3.滚轮up向上滑动，滚轮down向下滑动
		4.滚轮单点为home,可长按home
		5.可实时更新屏幕
		6.可自动获取屏幕参数构建对应比例屏幕
	*存在缺点：
		1.延时长
		2.适配问题，目前完全支持Android4.4.2（主要是我的手机）还没测试其他机型
		3.存在软件未知崩溃，怀疑是线程问题，机率不大
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
//获取电脑高度
int win_height=GetSystemMetrics(SM_CYFULLSCREEN);
double screen_pi=2;
//更新界面
int getImage(PIMAGE &image);

//为了隐藏窗口调用adb
void mySystem(const char* cmd, const char*  par);

//清除电脑缓存图片
int deletLocalCachePic();

//清除手机缓存图片
void deletCachePicFromPhone();

//更新缓存
void updateCache();

//捕捉图
unsigned WINAPI Catpic(void *arg);

//说明
void aboutApp(int width);

//获取phone的widthxheight
bool repareDeviceWidthAndHeight(int &width,int &height);

//初始化
void init(int width,int height,const char *appName);

//响应处理
void respongYourRequset(int height);

///////////////////////////主函数开始/////////////////////////
int main()
{
	int width;
	int height;
	deletLocalCachePic();
	if(!repareDeviceWidthAndHeight(width,height)) {
		MessageBox(GetConsoleWindow(),"打开失败，请重启软件，确定usb打开！","提示!",0);
		int isok=MessageBox(GetConsoleWindow(),"即将打开usb调试打开帮助","usb调试帮助？",1);
		if(isok==IDOK)
			WinExec("explorer .\\res\\usbhelp.html",SW_HIDE);
		return OPENERROR;
	}
	init(width,height,"控控我的phone");
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
//////////////////////////主函数结束//////////////////////////

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
	ShExecInfo.lpFile = cmd;//调用的程序名
	ShExecInfo.lpParameters = par;//调用程序的命令行参数
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;//窗口状态为隐藏
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess,INFINITE);////等到该进程结束
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
	setfont(25,0,"宋体");

	//bar(width/10,width/5,width/10+50,width/5+50);
	for(int i=0; (fscanf(about,"%s",carr))!=EOF; i++) {
		outtextxy(width/10+10,width/5+i*30,carr);
	}

}

bool repareDeviceWidthAndHeight(int &width,int &height)
{
	//用于获取手机屏幕尺寸，一般为高通机型
	if(system(".\\tool\\adb shell wm size >.\\res\\phone.info")!=0)
		return false;
	FILE *f=fopen(".\\res\\phone.info","r");
	if(f==NULL) {
		return false;
	}
	fseek(f,0L,SEEK_SET);
	char c,w[5],h[5];
	bool flags=false;
	int index=0;//寻址索引
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

		//第一次无法获取屏幕参数的情况下的方式
		if(system(".\\tool\\adb shell \"dumpsys window | grep mUnrestrictedScreen\" >.\\res\\phone.info")==0) {
			f=fopen(".\\res\\phone.info","r");
			if(f==NULL) {
				return false;
			}
			fseek(f,0L,SEEK_SET);
			memset(w,'\0',sizeof(w));
			memset(h,'\0',sizeof(w));
			flags=false;
			bool canread=false; //过滤非数字
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

	//设置屏幕大小
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
		//左键
		if(msg.is_left()) {
			if(longpress&&!swipe) {

				//长按
				sprintf(str, "%s swipe %4f %4f %4f %4f %4d",commandinput, lx1*screen_pi, ly1*screen_pi,lx1*screen_pi, ly1*screen_pi,(long long)(duration*1000));
				WinExec(str,SW_HIDE);
			} else if(!longpress&&swipe) {
				sprintf(str, "%s swipe %4f %4f %4f %4f %4d",commandinput, lx1*screen_pi, ly1*screen_pi,lx2*screen_pi, ly2*screen_pi,130);
				WinExec(str,SW_HIDE);
			} else {
				//左击==click
				mousepos(&x, &y);
				sprintf(str, "%s tap %4f %4f",commandinput,  x*screen_pi, y*screen_pi);
				WinExec(str,SW_HIDE);
			}
		}

		//右击
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

		//滚轮==up and down
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

		//中==longpress home
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

