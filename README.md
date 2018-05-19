
---
### 编译环境
>devc++（需要下载）
ege图形库(需要自己配置到devc++)
adb指令(已提供在tool文件夹下)
手机端要打开手机调试

### 主要原理
>利用adb的input指令，使用c语言
### 演示图
![演示.gif](https://upload-images.jianshu.io/upload_images/4413333-6def472eb945cfb0.gif?imageMogr2/auto-orient/strip)


### 基本功能
```
已实现功能：
- 1.鼠标左击为触点事件，
     支持长按 ，支持滑动，
     但是未实现拖拽 
- 2.鼠标右点击为返回，
     长按唤醒和睡眠，
- 3.滚轮up向上滑动，
      滚轮down向下滑动
- 4.滚轮单点为home,
      可长按home
- 5.可实时更新屏幕
- 6.可自动获取屏幕参数构建
      对应比例屏幕 
存在缺点：
- 1.延时长
```
### 操作提示
```
1.鼠标左击为触点事件
2.支持长按,滑动
3.但是未实现拖拽 
4.鼠标右点击为返回，
5.右键长按为唤醒和睡眠，
6.滚轮up向上滑动，
7.滚轮down向下滑动
8.滚轮单点为home,
9.可长按home
任意键继续...
```

### 部分核心源码解析
1.适配屏幕大小，先获取手机屏幕信息，然后和电脑的高度进行对比，决定缩放倍数(默认是2倍)
```
//获取电脑高度，一般电脑的高度是比较小(短板效应）,所以这里考虑高
int win_height=GetSystemMetrics(SM_CYFULLSCREEN);
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
	//设置在电脑显示屏幕大小的倍数
	if(height>win_height) {
		screen_pi=(double)height/win_height;
	}
	return true;
}
```
2.鼠标事件响应
```
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
				//}
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
```
### 项目地址
1.[项目地址](https://github.com/Cendeal/touchphone_helper)
2.[可执行文件压缩包](https://raw.githubusercontent.com/Cendeal/touchphone_helper/master/release/touchphone_helper.zip)



