/*----------------------------------------------------------------------------------------
 *	Copyright (c) Jinan Tony Robotics Co., Ltd. All rights reserved.
 *	Author: Sun Liang
 *	自动水平竖直折线生成模块
 *--------------------------------------------------------------------------------------*/

#ifndef AUTOPOLYLINE_H_
#define AUTOPOLYLINE_H_

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>

namespace autopolyline
{

using namespace v8;

class AutoPolyLine : public node::ObjectWrap
{
private:
	//	构造函数
	//	参数:
	//		width, height:	地图尺寸
	explicit AutoPolyLine(int width, int height);

	//	析构函数
	~AutoPolyLine();

	//	重设地图尺寸,改变尺寸时调用
	//	参数:
	//		width, height:	地图尺寸
	void InitSize(int width, int height);

	//	初始化地图,放置或修改障碍物矩形之前调用
	void InitMap();

	//	设置阻隔边框参数
	//	参数:
	//		width: 阻隔边框宽度
	//		resistance: 边框阻力系数
	void SetBorder(int width, char resistance);

	//	增加障碍物矩形
	//	参数:
	//		x, y: 矩形左上角位置
	//		width, height: 矩形尺寸
	//		resistance:	障碍物矩形阻力系数
	void AddObstacleRect(int x, int y, int width, int height, char resistance);

	//	创建折线,并根据折线自动更新阻隔边框参数
	//	参数:
	//		dirx, diry: 方向顺序数组
	//		x1, y1: 起始点
	//		x2, y2: 结束点
	//	返回值:
	//		折线点的个数
	int CreatePolyLine(const int dirx[], const int diry[], int x1, int y1, int x2, int y2);

private:
	int width, height;	// 地图尺寸
	int BorderWidth;	// 阻隔边框宽度
	char BorderResistance;	// 阻隔边框阻力系数
	char *map;	// 阻力地图,0:不可通过,大于0:通过所需步数
	char *travel;	// 行进标记数组,-1:尚未搜索,0,1,2,3:左右上下
	int (*BoundaryBuf)[2];	// 记录搜索区域边界,双缓冲
	char (*BoundaryStep)[2];	// 记录搜索区域边界剩余步数

	// 修正越界矩形, 可修正时返回true
	bool FixRect(int *x, int *y, int *width, int *height);

	// 填充矩形
	void FillRect(int x, int y, int width, int height, char val);

	// 添加折线阻隔边框
	void AddPolyLineBorder(const int point[], int count);	

// nodejs接口
public:
	static void Init(Local<Object> exports);

private:
	static void New(const FunctionCallbackInfo<Value>& args);
	static void InitSize(const FunctionCallbackInfo<Value>& args);
	static void InitMap(const FunctionCallbackInfo<Value>& args);
	static void SetBorder(const FunctionCallbackInfo<Value>& args);
	static void AddObstacleRect(const FunctionCallbackInfo<Value>& args);
	static void CreatePolyLine(const FunctionCallbackInfo<Value>& args);
	static Persistent<Function> constructor;
};

}

#endif  // AUTOPOLYLINE_H_