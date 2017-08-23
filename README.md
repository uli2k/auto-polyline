# A nodejs module to Generate Poly Line for POD

## Building

```sh
npm run install
```

const autopolyline = require('./build/Release/autopolyline');

const obj = new autopolyline.AutoPolyLine(100, 40);

obj.InitMap();

//	重设地图尺寸,改变尺寸时调用
//	参数:
//		width, height:	地图尺寸

//	初始化地图,放置或修改障碍物矩形之前调用
obj.InitSize(200, 80);

obj.InitMap();

//	设置阻隔边框参数
//	参数:
//		width: 阻隔边框宽度
//		resistance: 边框阻力系数
obj.SetBorder(20, 2);

//	增加障碍物矩形
//	参数:
//		x, y: 矩形左上角位置
//		width, height: 矩形尺寸
//		resistance:	障碍物矩形阻力系数
obj.AddObstacleRect(20, 5, 20, 15, 0);

//	创建折线,并根据折线自动更新阻隔边框参数
//	参数:
//		x1, y1: 起始点
//		x2, y2: 结束点
//	返回值:
//		折线点的个数
var poly = obj.CreatePolyLine(5, 3, 95, 35);

## License
[MIT]
