/*----------------------------------------------------------------------------------------
 *	Copyright (c) Jinan Tony Robotics Co., Ltd. All rights reserved.
 *	Author: Sun Liang
 *	带权的二维地图路径搜索方法
 *--------------------------------------------------------------------------------------*/

#include "AutoPolyLine.h"

namespace autopolyline
{

using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

Persistent<Function> AutoPolyLine::constructor;

AutoPolyLine::AutoPolyLine(int width, int height)
{
	if (width <= 0 || height <= 0 || width > 45000 || height > 45000)
		return;
	int size = width * height;
	this->width = width;
	this->height = height;
	this->BorderWidth = 0;
	this->BorderResistance = 1;
	this->map = new char[size];
	this->travel = new char[size];
	this->BoundaryBuf = new int[(width + height) * 2][2];
	this->BoundaryStep = new char[(width + height) * 2][2];
}

AutoPolyLine::~AutoPolyLine()
{
	if (this->BoundaryStep)
		delete[] this->BoundaryStep;
	if (this->BoundaryBuf)
		delete[] this->BoundaryBuf;
	if (this->travel)
		delete[] this->travel;
	if (this->map)
		delete[] this->map;
}

void AutoPolyLine::InitSize(int width, int height)
{
	if (width <= 0 || height <= 0 || width > 45000 || height > 45000)
		return;
	int size = width * height;
	this->width = width;
	this->height = height;
	delete[] this->BoundaryStep;
	delete[] this->BoundaryBuf;
	delete[] this->travel;
	delete[] this->map;
	this->map = new char[size];
	this->travel = new char[size];
	this->BoundaryBuf = new int[(width + height) * 2][2];
	this->BoundaryStep = new char[(width + height) * 2][2];
}

void AutoPolyLine::InitMap()
{
	for (int i = 0; i < this->width * this->height; i++)	// 全部设置为可通过
		this->map[i] = 1;
}

void AutoPolyLine::SetBorder(int width, char resistance)
{
	if (width < 0)
		width = 0;
	this->BorderWidth = width;
	if (resistance < 1)
		resistance = 1;
	this->BorderResistance = resistance;
}

bool AutoPolyLine::FixRect(int *x, int *y, int *width, int *height)
{
	if (*x >= this->width || *x + *width <= 0 || *y >= this->height || *y + *height <= 0)
		return false;	// 位置在地图外
	if (*width <= 0 || *height <= 0)
		return false;	// 非法尺寸
	if (*x < 0)	// 超出部分裁剪
	{
		*width += *x;
		*x = 0;
	}
	if (*width > this->width - *x)
		*width = this->width - *x;
	if (*y < 0)
	{
		*height += *y;
		*y = 0;
	}
	if (*height > this->height - *y)
		*height = this->height - *y;
	return true;
}

void AutoPolyLine::FillRect(int x, int y, int width, int height, char val)
{
	char *p = this->map + x + y * this->width;
	while (height > 0)
	{
		x = width;
		while (x > 0)
		{
			if (*p)
				*p = val;
			p++;
			x--;
		}
		p += this->width - width;
		height--;
	}
}

void AutoPolyLine::AddObstacleRect(int x, int y, int width, int height, char resistance)
{
	if (!this->FixRect(&x, &y, &width, &height))
		return;
	this->FillRect(x, y, width, height, resistance);

	int bx, by, bw, bh;
	// 左侧边框
	bx = x - this->BorderWidth;
	by = y;
	bw = this->BorderWidth;
	bh = height;
	if (this->FixRect(&bx, &by, &bw, &bh))
		this->FillRect(bx, by, bw, bh, this->BorderResistance);
	// 右侧边框
	bx = x + width;
	by = y;
	bw = this->BorderWidth;
	bh = height;
	if (this->FixRect(&bx, &by, &bw, &bh))
		this->FillRect(bx, by, bw, bh, this->BorderResistance);
	// 上方边框
	bx = x - this->BorderWidth;
	by = y - this->BorderWidth;
	bw = width + this->BorderWidth * 2;
	bh = this->BorderWidth;
	if (this->FixRect(&bx, &by, &bw, &bh))
		this->FillRect(bx, by, bw, bh, this->BorderResistance);
	// 下方边框
	bx = x - this->BorderWidth;
	by = y + height;
	bw = width + this->BorderWidth * 2;
	bh = this->BorderWidth;
	if (this->FixRect(&bx, &by, &bw, &bh))
		this->FillRect(bx, by, bw, bh, this->BorderResistance);
}

void AutoPolyLine::AddPolyLineBorder(const int point[], int count)
{
	for (int i = 2; i < count * 2; i += 2)
	{
		int bx, by, bw, bh;
		// 计算线的方向
		if (point[i] == point[i - 2])	// 竖线
		{
			if (point[i + 1] < point[i - 1])
			{
				by = point[i + 1] - this->BorderWidth;
				bh = point[i - 1] - by + 1 + this->BorderWidth;
			}
			else
			{
				by = point[i - 1] - this->BorderWidth;
				bh = point[i + 1] - by + 1 + this->BorderWidth;
			}
			bx = point[i] - this->BorderWidth;
			bw = 1 + this->BorderWidth * 2;
		}
		else	// 横线
		{
			if (point[i] < point[i - 2])
			{
				bx = point[i] - this->BorderWidth;
				bw = point[i - 2] - bx + 1 + this->BorderWidth;
			}
			else
			{
				bx = point[i - 2] - this->BorderWidth;
				bw = point[i] - bx + 1 + this->BorderWidth;
			}
			by = point[i + 1] - this->BorderWidth;
			bh = 1 + this->BorderWidth * 2;
		}
		if (this->FixRect(&bx, &by, &bw, &bh))
			this->FillRect(bx, by, bw, bh, this->BorderResistance);
	}
}

int AutoPolyLine::CreatePolyLine(const int dirx[], const int diry[], int x1, int y1, int x2, int y2)
{
	if (x1 < 0 || x1 >= this->width || y1 < 0 || y1 >= this->height)	// 起始结束点不在区域内
		return 0;
	if (x2 < 0 || x2 >= this->width || y2 < 0 || y2 >= this->height)
		return 0;
	if (x1 == x2 && y1 == y2)	// 起始结束点重合
		return 0;
	if (this->map[x1 + y1 * this->width] == 0 || this->map[x2 + y2 * this->width] == 0)	// 起始点或结束点不可通过
		return 0;

	for (int i = 0; i < this->width * this->height; i++)	// 全部设置为未搜索
		this->travel[i] = -1;
	this->travel[x1 + y1 * this->width] = 4;	// 设置起始点,4可区分任何方向,表示端点
	this->BoundaryBuf[0][0] = x1 + y1 * this->width;
	this->BoundaryStep[0][0] = this->map[x1 + y1 * this->width];
	int BbCount[2] = {1};	// 边界点计数器
	int step = 0;	// 计步器

	for (;;)
	{
		for (int i = 0; i < BbCount[step % 2]; i++)	// 遍历已搜索到的边界点
			if (this->BoundaryStep[i][step % 2] == 0)	// 步数清空,可向四周继续搜索
				for (int dir = 0; dir < 4; dir++)	// 4个方向
				{
					int x = this->BoundaryBuf[i][step % 2] % this->width + dirx[dir];
					int y = this->BoundaryBuf[i][step % 2] / this->width + diry[dir];
					if (x >= 0 && x < width && y >= 0 && y < height)
					{
						int idx = x + y * this->width;
						if (this->map[idx] > 0 && this->travel[idx] < 0)	// 可以通过且尚未搜索过
						{
							this->travel[idx] = dir;	// 标记来时方向
							this->BoundaryBuf[BbCount[(step + 1) % 2]][(step + 1) % 2] = idx;	// 放入另一个边界点缓冲区中
							this->BoundaryStep[BbCount[(step + 1) % 2]][(step + 1) % 2] = this->map[idx] - 1;	// 设置步数
							BbCount[(step + 1) % 2]++;
							if (x == x2 && y == y2)	// 到达结束点,进行后续处理,回溯路径,标记线边框
							{
								this->BoundaryBuf[0][0] = x;	// BoundaryBuf改用于暂存路径
								this->BoundaryBuf[0][1] = y;	// 加入结束点
								BbCount[0] = 1;
								for (;;)
								{
									x -= dirx[dir];
									y -= diry[dir];
									int tmpdir = this->travel[x + y * this->width];
									if (tmpdir != dir)	// 方向不一致,出现拐点
									{
										// 加入路径拐点
										this->BoundaryBuf[BbCount[0]][0] = x;	// BoundaryBuf改用于暂存路径
										this->BoundaryBuf[BbCount[0]][1] = y;
										BbCount[0]++;
										dir = tmpdir;
									}
									if (dir == 4)	// 到达端点,返回折线点个数
										return BbCount[0];
								}
							}
						}
					}
				}
			else	// 步数不足
			{
				this->BoundaryBuf[BbCount[(step + 1) % 2]][(step + 1) % 2] = this->BoundaryBuf[i][step % 2];	// 放入另一个边界点缓冲区中
				this->BoundaryStep[BbCount[(step + 1) % 2]][(step + 1) % 2] = this->BoundaryStep[i][step % 2] - 1;	// 步数减1
				BbCount[(step + 1) % 2]++;
			}
		if (BbCount[step % 2] == 0)	// 没有边界点,没有可到达的路径
			return 0;
		BbCount[step % 2] = 0;	// 清空边界点缓冲
		step++;
	}
}

void AutoPolyLine::Init(Local<Object> exports)
{
	Isolate *isolate = exports->GetIsolate();

	// 准备构造函数模版
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "AutoPolyLine"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// 原型
	NODE_SET_PROTOTYPE_METHOD(tpl, "InitSize", InitSize);
	NODE_SET_PROTOTYPE_METHOD(tpl, "InitMap", InitMap);
	NODE_SET_PROTOTYPE_METHOD(tpl, "SetBorder", SetBorder);
	NODE_SET_PROTOTYPE_METHOD(tpl, "AddObstacleRect", AddObstacleRect);
	NODE_SET_PROTOTYPE_METHOD(tpl, "CreatePolyLine", CreatePolyLine);

	constructor.Reset(isolate, tpl->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "AutoPolyLine"), tpl->GetFunction());
}

void AutoPolyLine::New(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	// 检查传入的参数的个数
	if (args.Length() < 2)
	{
		// 抛出一个错误并传回到 JavaScript
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	// 检查参数的类型
	if (!args[0]->IsNumber() || !args[1]->IsNumber())
	{
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
		return;
	}

	if (args.IsConstructCall())
	{
		// 像构造函数一样调用：`new AutoPolyLine(...)`
		int width = args[0]->IsUndefined() ? 0 : args[0]->Int32Value();
		int height = args[1]->IsUndefined() ? 0 : args[1]->Int32Value();
		AutoPolyLine *obj = new AutoPolyLine(width, height);
		obj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}
	else
	{
		// 像普通方法 `AutoPolyLine(...)` 一样调用，转为构造调用。
		const int argc = 2;
		Local<Value> argv[argc] = { args[0], args[1] };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		Local<Object> result = cons->NewInstance(context, argc, argv).ToLocalChecked();
		args.GetReturnValue().Set(result);
	}
}

void AutoPolyLine::InitSize(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	// 检查传入的参数的个数
	if (args.Length() < 2)
	{
		// 抛出一个错误并传回到 JavaScript
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	// 检查参数的类型
	if (!args[0]->IsNumber() || !args[1]->IsNumber())
	{
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
		return;
	}

	AutoPolyLine *obj = ObjectWrap::Unwrap<AutoPolyLine>(args.Holder());

	int width = args[0]->IsUndefined() ? 0 : args[0]->Int32Value();
	int height = args[1]->IsUndefined() ? 0 : args[1]->Int32Value();
	obj->InitSize(width, height);
}

void AutoPolyLine::InitMap(const FunctionCallbackInfo<Value>& args)
{
	AutoPolyLine *obj = ObjectWrap::Unwrap<AutoPolyLine>(args.Holder());

	obj->InitMap();
}

void AutoPolyLine::SetBorder(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	// 检查传入的参数的个数
	if (args.Length() < 2)
	{
		// 抛出一个错误并传回到 JavaScript
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	// 检查参数的类型
	if (!args[0]->IsNumber() || !args[1]->IsNumber())
	{
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
		return;
	}

	AutoPolyLine *obj = ObjectWrap::Unwrap<AutoPolyLine>(args.Holder());

	int width = args[0]->IsUndefined() ? 0 : args[0]->Int32Value();
	char resistance = args[1]->IsUndefined() ? 0 : args[1]->Int32Value();
	obj->SetBorder(width, resistance);
}

void AutoPolyLine::AddObstacleRect(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	// 检查传入的参数的个数
	if (args.Length() < 5)
	{
		// 抛出一个错误并传回到 JavaScript
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	// 检查参数的类型
	if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber() || !args[4]->IsNumber())
	{
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
		return;
	}

	AutoPolyLine *obj = ObjectWrap::Unwrap<AutoPolyLine>(args.Holder());

	int x = args[0]->IsUndefined() ? 0 : args[0]->Int32Value();
	int y = args[1]->IsUndefined() ? 0 : args[1]->Int32Value();
	int width = args[2]->IsUndefined() ? 0 : args[2]->Int32Value();
	int height = args[3]->IsUndefined() ? 0 : args[3]->Int32Value();
	char resistance = args[4]->IsUndefined() ? 0 : args[4]->Int32Value();
	obj->AddObstacleRect(x, y, width, height, resistance);
}

void AutoPolyLine::CreatePolyLine(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	// 检查传入的参数的个数
	if (args.Length() < 4)
	{
		// 抛出一个错误并传回到 JavaScript
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	// 检查参数的类型
	if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber())
	{
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
		return;
	}

	AutoPolyLine *obj = ObjectWrap::Unwrap<AutoPolyLine>(args.Holder());

	int x1 = args[0]->IsUndefined() ? 0 : args[0]->Int32Value();
	int y1 = args[1]->IsUndefined() ? 0 : args[1]->Int32Value();
	int x2 = args[2]->IsUndefined() ? 0 : args[2]->Int32Value();
	int y2 = args[3]->IsUndefined() ? 0 : args[3]->Int32Value();
	Local<Array> arr;

	static const int dir1x[4] = {-1, 1, 0, 0};	// 左 右 上 下 4个方向
	static const int dir1y[4] = {0, 0, -1, 1};
	static const int dir2x[4] = {0, 0, -1, 1};	// 上 下 左 右 4个方向
	static const int dir2y[4] = {-1, 1, 0, 0};

	int pnum = obj->CreatePolyLine(dir1x, dir1y, x1, y1, x2, y2);	// 生成第一次点
	if (pnum == 0)	// 找不到折线
	{
		arr = Array::New(isolate, 4);	// 直连起始点
		arr->Set(0, Number::New(isolate, x1));
		arr->Set(1, Number::New(isolate, y1));
		arr->Set(2, Number::New(isolate, x2));
		arr->Set(3, Number::New(isolate, y2));
	}
	else
	{
		int tmpPoint[64];	// 暂存第一次点
		for (int i = 0; i < pnum; i++)
		{
			tmpPoint[i * 2 + 0] = obj->BoundaryBuf[i][0];
			tmpPoint[i * 2 + 1] = obj->BoundaryBuf[i][1];
		}

		int pnum2 = obj->CreatePolyLine(dir2x, dir2y, x1, y1, x2, y2);	// 生成第二次点
		if (pnum2 < pnum)	// 第二次生成的点比第一次少
		{
			pnum = pnum2;
			for (int i = 0; i < pnum; i++)
			{
				tmpPoint[i * 2 + 0] = obj->BoundaryBuf[i][0];
				tmpPoint[i * 2 + 1] = obj->BoundaryBuf[i][1];
			}
		}

		obj->AddPolyLineBorder(tmpPoint, pnum);	// 设置折线阻隔框
		arr = Array::New(isolate, pnum * 2);	// 连接折线点
		for (int i = 0; i < pnum * 2; i++)
			arr->Set(i, Number::New(isolate, tmpPoint[i]));
	}

	args.GetReturnValue().Set(arr);
}

void InitAll(Local<Object> exports) {
	AutoPolyLine::Init(exports);
}

NODE_MODULE(autopolyline, InitAll)

}