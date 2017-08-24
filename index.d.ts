declare module 'autopolyline' {
	export class AutoPoliLine {
		constructor(width: number, height: number)
		InitSize(width: number, height: number): void;
		InitMap(): void;
		SetBorder(width: number, resistance: number): void;
		AddObstacleRect(x: number, y: number, width: number, height: number, resistance: number): void;
		CreatePolyLine(x1: number, y1: number, x2: number, y2: number): Array<number>;
	}
}
