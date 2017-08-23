const autopolyline = require('./build/Release/autopolyline');

const obj = new autopolyline.AutoPolyLine(100, 40);

obj.InitMap();
obj.SetBorder(2, 2);
obj.AddObstacleRect(20, 5, 20, 15, 0);
obj.AddObstacleRect(70, 25, 10, 5, 0);

var poly = obj.CreatePolyLine(5, 3, 95, 35);

for (var i = 0; i < poly.length; i++)
    console.log(poly[i]);

poly = obj.CreatePolyLine(25, 3, 55, 39);

for (var i = 0; i < poly.length; i++)
    console.log(poly[i]);
