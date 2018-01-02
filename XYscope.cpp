#include "XYscope.h"

#include <DueTimer.h>

uint8_t TimerBlinkState = 0;

//#include "XYscopeVectorFont.cpp"

XYscope::XYscope() {
	//Initialize variables used in these routines

	XYlistEnd = 0;
	pinMode(crtBlankingPin, OUTPUT);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Vector Graphics Package
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//

void XYscope::setGraphicsIntensity(short GraphBright) {
	//	Routine to set graphics "intensity" (aka: brightness) for all graphics operations
	//
	//	Calling parameters:
	//
	//	GraphBright	This variable sets brightness of the graphics figure (in percent)
	//				While valid range is 0-255, Useful values are 25-200%.
	//
	//				Note 1: Brighter (larger) value will plot brighter as more "dots per inch"
	//				will be plotted.  This will "consume" more space in XY_list array.
	//
	//				Note 2: Depending upon size & focus of CRT, gaps between contiguous points of a line,
	//				circle, ellipse, etc. may be become visible for values less than 50%.
	//
	//				At startup, GraphBright is initialized to 100.
	//	Returns:
	//				Nothing.
	//
	//	20170811 Ver 0.0	E.Andrews	First cut

	long InputMin, InputMax, OutputMin, OutputMax;
	//Use linear mapping functions for brightness conversion
	//using two breakpoints...One for 0-100%, and the other for 100-255%;

	const int BreakPoint = 100;

	if (GraphBright <= 255 && GraphBright >= 0) {
		if (GraphBright >= BreakPoint) {//Use this mapping function parameters for Inputs from 100-255%

			InputMin = 100;		//Define output when input=100%
			OutputMin = 10;

			InputMax = 255;		//Define output when input=255%
			OutputMax = 1;
		} else {//Use this mapping function parameters for Inputs from 0 to 100%

			InputMin = 25;		//Define output when input=25%
			OutputMin = 40;

			InputMax = 100;		//Define output when input=100%
			OutputMax = 10;
		}
		//Save updated GraphBright to class variable
		_graphBrightness = GraphBright;
		//Calculate new graphDensity based on 'GraphBright' value
		_graphDensity = (GraphBright - InputMin) * (OutputMax - OutputMin)
				/ (InputMax - InputMin) + OutputMin;

	}//If user passes us a NEGATIVE or OVER_RANGE value, just go to the graphics density setting

}

short XYscope::getGraphicsIntensity() {
	//	Routine to set graphics "intensity" (aka: brightness) for all graphics operations
	//
	//	Calling parameters: NONE
	//
	//	Returns:
	//	GraphBright	This variable represents the current brightness of the graphics figure (in percent)
	//				While valid range is 0-255, Useful values are 25-200%.
	//
	//				At startup, GraphBright is initialized to 100.
	//
	//	20170811 Ver 0.0	E.Andrews	First cut

	return _graphBrightness;
}

void XYscope::setTextIntensity(short TextBright) {
	//	Routine to set graphics "intensity" (aka: brightness) for all graphics operations
	//
	//	Calling parameters:
	//
	//	GraphBright	This variable sets brightness of the text (in percent)
	//				While valid range is 0-255, Useful values are 25-200%.
	//
	//				Note 1: Brighter (larger) value will plot brighter as more "dots per inch"
	//				will be plotted.  This will "consume" more space in XY_list array.
	//
	//				Note 2: Depending upon size & focus of CRT, gaps between contiguous points of
	//				character may be become visible for values less than 50%.
	//
	//				At startup, TextBright is initialized to 100.
	//	Returns:
	//				Nothing.
	//
	//	20170811 Ver 0.0	E.Andrews	First cut

	long InputMin, InputMax, OutputMin, OutputMax;
	//Use linear mapping functions for brightness conversion
	//using two breakpoints...One for 0-100%, and the other for 100-255%;

	const int BreakPoint = 100;

	if (TextBright < 255 && TextBright >= 0) {//Only update brightness if passed parameter is IN RANGE
		if (TextBright >= BreakPoint) {	//Use this mapping function parameters for Inputs from 100-255%

			InputMin = 100;		//Define output when input=100%
			OutputMin = 10;

			InputMax = 255;		//Define output when input=255%
			OutputMax = 1;
		} else {//Use this mapping function parameters for Inputs from 0 to 100%

			InputMin = 25;		//Define output when input=25%
			OutputMin = 40;

			InputMax = 100;		//Define output when input=100%
			OutputMax = 10;
		}
		//Save updated TextBright to class variable
		_textBrightness = TextBright;
		//Calculate new density based on 'GraphBright' value
		_textDensity = (TextBright - InputMin) * (OutputMax - OutputMin)
				/ (InputMax - InputMin) + OutputMin;
	}

}

short XYscope::getTextIntensity() {
	//	Routine to set graphics "intensity" (aka: brightness) for all graphics operations
	//
	//	Calling parameters: NONE
	//
	//	Returns:
	//	TextBright	This variable represents the current brightness of the graphics figure (in percent)
	//				While valid range is 0-255, Useful values are 25-200%.
	//
	//				At startup, GraphBright is initialized to 100.
	//
	//	20170811 Ver 0.0	E.Andrews	First cut

	return _textBrightness;
}

void XYscope::plotClear(void) {
	//	This CLEARS OUT the current plot buffer
	//
	//	Returns: NOTHING (Sets global variable, XYlistEnd = 0)
	//
	//	20170705 Ver 2.0	E.Andrews	Simplified call by eliminating all passed parameters

	//	Just reset the array pointers and display goes blank
	XYlistEnd = 0;
	//Now initialize the first entry into the XYlist array using the plotStart() function
	plotStart();
	plotEnd();
	//AutoSetRefreshTime();
	return;
}

void XYscope::plotEnd() {
	//	Routine Repeats the contents of the last entry just pass the end-of-buffer pointer.
	//	This compensates for END OF LIST blanking uncertainty & ensures the very last point in the buffer
	//	will be unblanked and seen on the.	Call this routine anytime you think the display list is complete.
	//
	//	Calling parameters: NONE
	//	Returns: NOTHING
	//
	//	20170705 Ver 0.1	E.Andrews	First cut of simplified routine (no passed parameters)
	short LastX = X_flag, LastY = Y_flag;

	if (XYlistEnd > 0) {

		XY_List[XYlistEnd].X = XY_List[XYlistEnd - 1].X;
		XY_List[XYlistEnd].Y = XY_List[XYlistEnd - 1].X;
	} else {
		XY_List[XYlistEnd].X = X_flag;
		XY_List[XYlistEnd].Y = Y_flag;
	}

	//AutoSetRefreshTime();
	return;
}

void XYscope::plotStart() {
	//	Routine Loads the following three(X,Y) points into the start of the XYlist() buffer.
	//		(0,0), (4095,0),(0,0) - This creates a full-scale pulse on the X-channel, occuring during BLANKING period,
	//		that is used by the AGE hardware to properly synchronize the point clock to the DMA clock.
	//	Calling parameters: NONE
	//
	//	Returns: NOTHING
	//
	//	Global Variables: Resets pointers and Loads points into XY_List, updates XYlistEnd pointer
	//
	//	20170320 Ver 0.0	E.Andrews	First cut
	//  20170526 ver 0.1	E.Andrews	Fine tune number of start up pixels..
	//	20170617 Ver 0.2	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
	//
	//
	plotErr = 0;

	XYlistEnd = 0;
	//  We need to load a full scale pulse into the XYlist array for sync-up pouposes
	XY_List[XYlistEnd].X = 0 | X_flag;				//Load X Value
	XY_List[XYlistEnd].Y = 0 | Y_flag;
	XYlistEnd++;	//Load Y value and increment pointer

	//Write the SYNC PULSE into the front of the buffer...

	XY_List[XYlistEnd].X = 0xfff | X_flag;//Load full scale value into X to create a pulse
	XY_List[XYlistEnd].Y = 0 | Y_flag;
	XYlistEnd++;	//Load Y value

	XY_List[XYlistEnd].X = 0xfff | X_flag;//Load full scale value into X to create a pulse
	XY_List[XYlistEnd].Y = 0 | Y_flag;
	XYlistEnd++;	//Load Y value and increment pointer

	XY_List[XYlistEnd].X = 0 | X_flag;				//Load X Value
	XY_List[XYlistEnd].Y = 0 | Y_flag;
	XYlistEnd++;	//Load Y value and increment pointer

	//  Now add  a few more dummy points to give us time to come out of blanking...

	XY_List[XYlistEnd].X = 0 | X_flag;			    //Load X Value
	XY_List[XYlistEnd].Y = 0 | Y_flag;
	XYlistEnd++;	//Load Y value
	//AutoSetRefreshTime();
	return;
}

void XYscope::plotPoint(int x0, int y0) {
	//	Routine for POINT plotting
	//	Calling parameters:
	//
	//		x0, y0	Coordinate of point to be plotted.  
	//				Valid Range:  0<= x0 <= 4095, 0<= y0 <= 4095
	//	Returns: NOTHING
	//
	//	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
	//
	//	20170320 Ver 0.0	E.Andrews	First cut
	//	20170427 Ver 0.1	E.Andrews	Add plotErr & near end-of-buffer limit logic and check
	//	20170627 Ver 0.2	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
	//
	if (_screenOnTime_ms != 0)
		_crtOffTOD_ms = millis() + _screenOnTime_ms;//Update ScreenOff time of day (ms)
	if (XYlistEnd > MaxBuffSize - 3) {
		plotErr = 0;//Set plotErr and skip writing point into buffer if we are about to hit the end-of-buffer.
	} else {

		XY_List[XYlistEnd].X = (x0 & 0xfff) | X_flag;//Load X Value into EVEN array term
		XY_List[XYlistEnd].Y = (y0 & 0xfff) | Y_flag;//Load Y value into ODD array term

		XYlistEnd++;							//Increment List Pointer  value 

	}
	return;
}

void XYscope::plotLine(int x0, int y0, int x1, int y1) {
	//	BRESSHAM Algorithm for LINE drawing
	//		Algorithm implementation/starting code base from: http://members.chello.at/~easyfilter/bresenham.html
	//	Calling parameters:
	//
	//		x0, y0	Coordinate of starting point
	//		x1, y1	Coordinate of ending point
	//
	//		  		Density is a COMMON variable that the using sets with setDensity() routine.
	//				Higher values of Density increases the distance between ajacent plotted points of graphics figures.
	//				Smaller values of Density reduces the distand between adjacent plotted points of graphics figures.
	//				Valid numbers are 0 to 100.  Note: depending upon size & focus of CRT, values more ~50 shows
	//				cause lines of circles and vectors to begin to appear as dotted lines.
	//	Returns: NOTHING
	//
	//	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
	//
	//	20170321 Ver 0.1	E.Andrews	First cut
	//	20170617 Ver 0.2	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
	//
	plotErr = 0;
	int lastPointX, lastPointY;
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	int SkipCount = 0;
	for (;;) {
		if (SkipCount <= 0)	//|| (x0 == x1 && y0 == y1) )	//should we skip actualling putting point to the screen?
				{
			plotPoint(x0, y0);
			lastPointX = x0;		//Save and hold the last point plotted
			lastPointY = y0;
			SkipCount = _graphDensity;
		} else {
			SkipCount--;//The point is to be skipped so we don't need to actually send the point to the screen buffer
			//Just decrement Skip Point Counter till it gets to Zero, then plot the point
		}

		if (x0 == x1 && y0 == y1) {
			//Force a plot at the endpoint to 'finish the line' if the end point was not previously plotted
			if (lastPointX != x1 && lastPointY != y1)
				plotPoint(x1, y1);
			break;
		}
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}

	return;
}

void XYscope::plotRectangle(int x0, int y0, int x1, int y1) {
	//	Plots a RECTANGLE LINE drawing
	//	
	//	Calling parameters:
	//
	//		x0, y0	Coordinate of starting point
	//		x1, y1	Coordinate of ending point
	//
	//		  		Density is a COMMON variable that the using sets with setDensity() routine.
	//				Higher values of Density increases the distance between ajacent plotted points of graphics figures.
	//				Smaller values of Density reduces the distand between adjacent plotted points of graphics figures.
	//				Valid numbers are 0 to 100.  Note: depending upon size & focus of CRT, values more ~50 shows
	//				cause lines of circles and vectors to begin to appear as dotted lines.
	//
	//	Returns: NOTHING
	//
	//	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
	//
	//	20170424 Ver 0.0	E.Andrews	First cut
	//	20170617 Ver 0.2	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
	//	

	plotErr = 0;
	plotLine(x0, y0, x1, y0);	//Top horizontal line
	plotLine(x1, y0, x1, y1);	//Right verticle line
	plotLine(x1, y1, x0, y1);	//Bottom horizontal line
	plotLine(x0, y1, x0, y0);	//Left verticle line

	return;
}

void XYscope::plotCircle(int xc, int yc, int r) {
	//	Routine for CIRCLE plotting.  Basic algorithm implementation/starting code base from: 
	//	https://sites.google.com/site/ruslancray/lab/projects/bresenhamscircleellipsedrawingalgorithm/bresenham-s-circle-ellipse-drawing-algorithm
	//
	//	Calling parameters:
	//		ix		Array pointer into aaAudio.dacBuffer16() where X & Y point will be put.
	//
	//		xc, yc	Coordinate of point center of circle to be plotted.  
	//				Valid Range:  0<= xc <= 4095, 0<= yc <= 4095
	//
	//		r		Radius of circle   
	//
	//	CAUTIONS:	Caller should take care so that center + radius values do not exceed 4095.
	//				Caller should take care so that center - radius values do not exceed 4095.
	//				Negative or coordinate values >4095 will be masked to 12 bits and will
	//				fold-over the edges of the valid plot area (x,y) = (0 to 4095, 0 to 4095).
	//
	//				NOTE: Routine will set plotErr >0 & skip plotting if passed parameter errors are detected.
	//	Returns: NOTHING
	//
	//	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
	//
	//	20170322 Ver 0.0	E.Andrews	First cut
	//	20170427 Ver 1.0	E.Andrews	Rework to improve shape
	//	20170617 Ver 2.0	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
	//	20170619 Ver 2.1	E.Andrews	Make this routine run in four passes to improve plot quality at high DMA clock speeds
	//

	//plotCircle(xc, yc, r,255);
	int arcSegment = 255;

	plotErr = 0;
	int SkipCount = 0;
	uint8_t pass = 0;

	//==========Check to be sure that the figure is fully on-screen
	if ((xc + r > 4095) || (xc - r < 0))
		plotErr = 1;
	if ((yc + r > 4095) || (yc - r < 0))
		plotErr = 1;
	//==========Skip drawing if figure is faulted
	if (plotErr == 0) {

		int x, y, delta, error;
		for (uint8_t loopCnt = 0; loopCnt < 4; loopCnt++) {	//Although this is 4X slower, it orders the points in a way that will improve plot quality

			x = 0;
			y = r;	//radius
			delta = 2 - 2 * r;
			error = 0;
			int xHalfWayLimit = int(float(r) * .707106);//xHalfWayLimit = xr * sin(45Deg)
			int yHalfWayLimit = int(float(r) * .707106);//yHalfWayLimit = xr * cos(45Deg)

			while (y >= 0) {
				if (SkipCount == 0 || y <= 0) {	//Actually plot a point whenever (SkipCount=0) OR (we're at the last point to be plotted)
					int xp, yp;
					switch (loopCnt) {

					case 0: {
						xp = xc - x;
						yp = yc + y;	//Segment 0, 1
						//Serial.print("S01 (");Serial.print(xp);Serial.print(",");Serial.print(yp);Serial.println(")");						
						//plotPoint(xc - x, yc + y);		//Segment 0, 1
						if (arcSegment & arc0 && x >= xHalfWayLimit)
							plotPoint(xp, yp);		//Segment 0
						if (arcSegment & arc1 && x < xHalfWayLimit)
							plotPoint(xp, yp);		//Segment 1

						break;

					}
					case 1: {
						xp = xc + x;
						yp = yc + y;	//Segment 2, 3
						//Serial.print("S23 (");Serial.print(xp);Serial.print(",");Serial.print(yp);Serial.println(")");					
						//plotPoint(xc + x, yc + y);		//Segment 2, 3
						if (arcSegment & arc2 && x < xHalfWayLimit)
							plotPoint(xp, yp);		//Segment 2
						if (arcSegment & arc3 && y <= yHalfWayLimit)
							plotPoint(xp, yp);		//Segment 3
						break;
					}
					case 2: {
						xp = xc + x;
						yp = yc - y;
						//plotPoint(xc + x, yc - y);		//Segment 4, 5
						if (arcSegment & arc4 && x < xHalfWayLimit)
							plotPoint(xp, yp);		//Segment 4
						if (arcSegment & arc5 && y <= yHalfWayLimit)
							plotPoint(xp, yp);		//Segment 5
						break;
					}
					case 3: {
						xp = xc - x;
						yp = yc - y;
						//plotPoint(xc - x, yc - y);		//Segment 6. 7
						if (arcSegment & arc6 && x < xHalfWayLimit)
							plotPoint(xp, yp);		//Segment 6
						if (arcSegment & arc7 && y <= yHalfWayLimit)
							plotPoint(xp, yp);		//Segment 7
						break;
					}

					}
					//if (arcSegment & arc0 && x >= xHalfWayLimit) plotPoint(xc - x, yc + y);	//Segment 0
					//if (arcSegment & arc1 && x < xHalfWayLimit) plotPoint(xc - x, yc + y);	//Segment 1
					//if (arcSegment & arc2 && x < xHalfWayLimit) plotPoint(xc + x, yc + y);	//Segment 2
					//if (arcSegment & arc3 && y <= yHalfWayLimit ) plotPoint(xc + x, yc + y);	//Segment 3
					//if (arcSegment & arc4 && y < yHalfWayLimit) plotPoint(xc + x, yc - y);		//Segment 4
					//if (arcSegment & arc5 && x <= xHalfWayLimit) plotPoint(xc + x, yc - y);		//Segment 5
					//if (arcSegment & arc6 && x <= xHalfWayLimit) plotPoint(xc - x, yc - y);		//Segment 6
					//if (arcSegment & arc7 && x > xHalfWayLimit) plotPoint(xc - x, yc - y);		//Segment 7
					SkipCount = _graphDensity;
				} else
					SkipCount--;

				error = 2 * (delta + y) - 1;

				if ((delta < 0) && (error <= 0)) {
					++x;
					delta += 2 * x + 1;
					continue;
				}

				error = 2 * (delta - x) - 1;

				if (delta > 0 && error > 0) {
					--y;
					delta += 1 - 2 * y;
					continue;
				}
				++x;
				delta += 2 * (x - y);
				--y;
			}
		}
	}
	return;
}
/*

 void XYscope::plotCircle(int xc, int yc, int r){
 //	Routine for CIRCLE plotting.  Basic algorithm implementation/starting code base from:
 //	https://sites.google.com/site/ruslancray/lab/projects/bresenhamscircleellipsedrawingalgorithm/bresenham-s-circle-ellipse-drawing-algorithm
 //
 //	Calling parameters:
 //
 //		xc, yc	Coordinate of point center of circle to be plotted.
 //				Valid Range:  0<= xc <= 4095, 0<= yc <= 4095
 //
 //		r		Radius of circle
 //
 //		arcSegment	8-bit code corresponding to the arc segments that are to be drawn.
 //					Bit = 1 means Draw arc segment, Bit = 0 means do NOT draw arc segment (Skip it!)
 //

 1  |	 2
 \	  |   /
 0  \  |  /  3
 -------+-------
 7  /  |  \   4
 /   |   \
						   6  |  5

 //
 //	CAUTIONS:	Caller should take care so that center + radius values do not exceed 4095.
 //				Caller should take care so that center - radius values do not exceed 4095.
 //				Negative or coordinate values >4095 will be masked to 12 bits and will
 //				fold-over the edges of the valid plot area (x,y) = (0 to 4095, 0 to 4095).
 //
 //				NOTE: Routine will set plotErr >0 & skip plotting if passed parameter errors are detected.
 //
 //	Returns: NOTHING
 //
 //	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
 //
 //	20170322 Ver 0.0	E.Andrews	First cut
 //	20170427 Ver 1.0	E.Andrews	Rework to improve shape
 //	20170617 Ver 0.2	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
 //	20170714 Ver 0.3	E.Andrews	Overlay function when user wants a full circle
 //
 plotCircle(xc, yc, r,255);
 }


 */

void XYscope::plotCircle(int xc, int yc, int r, uint8_t arcSegment) {
	//	Routine for CIRCLE plotting.  Basic algorithm implementation/starting code base from: 
	//	https://sites.google.com/site/ruslancray/lab/projects/bresenhamscircleellipsedrawingalgorithm/bresenham-s-circle-ellipse-drawing-algorithm
	//
	//	Calling parameters:
	//
	//		xc, yc	Coordinate of point center of circle to be plotted.  
	//				Valid Range:  0<= xc <= 4095, 0<= yc <= 4095
	//
	//		r		Radius of circle   
	//
	//		arcSegment	8-bit code corresponding to the arc segments that are to be drawn.
	//					Bit = 1 means Draw arc segment, Bit = 0 means do NOT draw arc segment (Skip it!)
	//
	/*
	 1  |	 2
	 \	  |   /
	 0  \  |  /  3
	 -------+-------
	 7  /  |  \   4
	 /   |   \
						   6  |  5
	 */
	//
	//	CAUTIONS:	Caller should take care so that center + radius values do not exceed 4095.
	//				Caller should take care so that center - radius values do not exceed 4095.
	//				Negative or coordinate values >4095 will be masked to 12 bits and will
	//				fold-over the edges of the valid plot area (x,y) = (0 to 4095, 0 to 4095).
	//
	//				NOTE: Routine will set plotErr >0 & skip plotting if passed parameter errors are detected.
	//
	//	Returns: NOTHING
	//
	//	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
	//
	//	20170322 Ver 0.0	E.Andrews	First cut
	//	20170427 Ver 1.0	E.Andrews	Rework to improve shape
	//	20170617 Ver 0.2	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
	//	20170907 Ver 2.0 	E.Andrews	Abandon Bresenham algorithm for slower floating point approach
	//									because new approach plots adjacent points and does not take advantage
	//									of figure symetry.This Allows AGI to run at a higher DMA_CLK freq.
	//
	const float pi = atan(1) * 4;
	const float arcAng0 = 0;
	const float arcAng1 = pi * .25;		// .25 * pi
	const float arcAng2 = pi * .5;		// .50 * pi
	const float arcAng3 = pi * .75;		// .75 * pi
	const float arcAng4 = pi;			//1.00 * pi
	const float arcAng5 = pi * 1.25;	//1.25 * pi
	const float arcAng6 = pi * 1.50;	//1.50 * pi
	const float arcAng7 = pi * 1.75;	//1.75 * pi
	const float FullCircle = 2 * pi;	//2.00 * pi
	float circumf;
	short xr, yr;
	xr = yr = r;

	//calculate circle circumference
	circumf = FullCircle * r;

	//Figure out how many points we will actually be plotting
	//based on the current graphicsIntensity (_graphicsDensity) setting
	float numOfPoints = circumf / _graphDensity;
	//Calc the deltaAng between points
	float deltaAng = FullCircle / (numOfPoints);

	float angle;
	short X, Y;

	for (angle = 0; angle < FullCircle; angle = angle + deltaAng) {
		X = int(xc - cos(angle) * xr);
		Y = int(yc + sin(angle) * yr);

		//Only plot those segments that are enabled by passed "arcSegment" variable
		if (arcSegment & arc0 && angle <= arcAng1)
			plotPoint(X, Y);					//Segment 0
		if (arcSegment & arc1 && angle > arcAng1 && angle <= arcAng2)
			plotPoint(X, Y);	//Segment 1
		if (arcSegment & arc2 && angle > arcAng2 && angle <= arcAng3)
			plotPoint(X, Y);	//Segment 2
		if (arcSegment & arc3 && angle > arcAng3 && angle <= arcAng4)
			plotPoint(X, Y);	//Segment 3
		if (arcSegment & arc4 && angle > arcAng4 && angle <= arcAng5)
			plotPoint(X, Y);	//Segment 4
		if (arcSegment & arc5 && angle > arcAng5 && angle <= arcAng6)
			plotPoint(X, Y);	//Segment 5
		if (arcSegment & arc6 && angle > arcAng6 && angle <= arcAng7)
			plotPoint(X, Y);	//Segment 6
		if (arcSegment & arc7 && angle > arcAng7)
			plotPoint(X, Y);						//Segment 7
	}

	return;
}

void XYscope::plotCircleBres(int xc, int yc, int r, uint8_t arcSegment) {
	//	Routine for CIRCLE plotting.  Basic algorithm implementation/starting code base from:
	//	https://sites.google.com/site/ruslancray/lab/projects/bresenhamscircleellipsedrawingalgorithm/bresenham-s-circle-ellipse-drawing-algorithm
	//
	//	Calling parameters:
	//
	//		xc, yc	Coordinate of point center of circle to be plotted.
	//				Valid Range:  0<= xc <= 4095, 0<= yc <= 4095
	//
	//		r		Radius of circle
	//
	//		arcSegment	8-bit code corresponding to the arc segments that are to be drawn.
	//					Bit = 1 means Draw arc segment, Bit = 0 means do NOT draw arc segment (Skip it!)
	//
	//
	//					   1  |	 2
	//					  \	  |   /
	//					0  \  |  /  3
	//				   -------+-------
	//					7  /  |  \   4
	//					  /   |   \
	//					   6  |  5
	//
	//
	//	CAUTIONS:	Caller should take care so that center + radius values do not exceed 4095.
	//				Caller should take care so that center - radius values do not exceed 4095.
	//				Negative or coordinate values >4095 will be masked to 12 bits and will
	//				fold-over the edges of the valid plot area (x,y) = (0 to 4095, 0 to 4095).
	//
	//				NOTE: Routine will set plotErr >0 & skip plotting if passed parameter errors are detected.
	//
	//	Returns: NOTHING
	//
	//	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
	//
	//	20170322 Ver 0.0	E.Andrews	First cut
	//	20170427 Ver 1.0	E.Andrews	Rework to improve shape
	//	20170617 Ver 0.2	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
	//
	plotErr = 0;
	int SkipCount = 0;
	//==========Check to be sure that the figure is fully on-screen
	if ((xc + r > 4095) || (xc - r < 0))
		plotErr = 1;
	if ((yc + r > 4095) || (yc - r < 0))
		plotErr = 1;
	//==========Skip drawing if figure is faulted
	if (plotErr == 0) {

		int x = 0;
		int y = r;	//radius
		int delta = 2 - 2 * r;
		int error = 0;

		int xHalfWayLimit = int(float(r) * .707106);//xHalfWayLimit = xr * sin(45Deg)
		int yHalfWayLimit = int(float(r) * .707106);//yHalfWayLimit = xr * cos(45Deg)
		while (y >= 0) {
			//if (SkipCount<=0 || y <= 0){
			if (SkipCount <= 0 || y <= 0) {
				if (arcSegment & arc0 && x >= xHalfWayLimit)
					plotPoint(xc - x, yc + y);		//Segment 0
				if (arcSegment & arc1 && x < xHalfWayLimit)
					plotPoint(xc - x, yc + y);		//Segment 1
				if (arcSegment & arc2 && x < xHalfWayLimit)
					plotPoint(xc + x, yc + y);		//Segment 2 
				if (arcSegment & arc3 && y <= yHalfWayLimit)
					plotPoint(xc + x, yc + y);	//Segment 3
				if (arcSegment & arc4 && y < yHalfWayLimit)
					plotPoint(xc + x, yc - y);		//Segment 4
				if (arcSegment & arc5 && x <= xHalfWayLimit)
					plotPoint(xc + x, yc - y);		//Segment 5
				if (arcSegment & arc6 && x <= xHalfWayLimit)
					plotPoint(xc - x, yc - y);		//Segment 6
				if (arcSegment & arc7 && x > xHalfWayLimit)
					plotPoint(xc - x, yc - y);		//Segment 7

				SkipCount = _graphDensity;
			} else
				SkipCount--;

			error = 2 * (delta + y) - 1;
			if ((delta < 0) && (error <= 0)) {
				++x;
				delta += 2 * x + 1;
				continue;
			}
			error = 2 * (delta - x) - 1;

			if (delta > 0 && error > 0) {
				--y;
				delta += 1 - 2 * y;
				continue;
			}
			++x;
			delta += 2 * (x - y);
			--y;
		}
	}
	return;
}

void XYscope::plotEllipse(int xc, int yc, int xr, int yr) {
	//	Routine for horizontal ELLIPSE plotting.
	//		Algorithm implementation/starting code base from: 
	//			https://sites.google.com/site/ruslancray/lab/projects/bresenhamscircleellipsedrawingalgorithm/bresenham-s-circle-ellipse-drawing-algorithm
	//			Also see: http://members.chello.at/~easyfilter/bresenham.html 
	//	Calling parameters:
	//		xc, yc	Coordinate of point center of ellipse to be plotted.  
	//				Valid Range:  0<= x0 <= 4095, 0<= y0 <= 4095
	//
	//		xr, yr	X-Radius of ellipse, Y-radius of ellipse   
	//				Valid Range:  0<= xr <= 4095, 0<= yr <= 4095	//
	//
	//	Returns: NOTHING
	//
	//	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
	//
	//	CAUTIONS:	Caller should take care so that center + radius values do not exceed 4095.
	//				Negative coordinates or coordinate values >4095 will be masked to 12 bits and will
	//				be folded-over the edges of the valid plot area which is (x,y) = (0 to 4095, 0 to 4095).	
	//
	//				NOTE: Routine will set plotErr to non-zero value and skip plotting if error in passed parameters are detected.
	//
	//	20170424	Ver 0.0	E.Andrews	First cut, irratic operation
	//	20170427	Ver 1.0	E.Andrews	This version uses some DOUBLE variables to correct ocasional mishapen ellipses!)
	//	20170617 	Ver 1.1	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer

	//Adjust passed coordinates to algorithm needs 

	plotEllipse(xc, yc, xr, yr, 255);//Call standard plotEllipse function with all segments=ON (255)
	return;
}

void XYscope::plotEllipseBres(int xc, int yc, int xr, int yr,
		uint8_t arcSegment)
//TODO REMOVE routine
		{
	//	Routine for horizontal ELLIPSE plotting.
	//		Breseham Algorithm implementation/starting code base from:
	//			https://sites.google.com/site/ruslancray/lab/projects/bresenhamscircleellipsedrawingalgorithm/bresenham-s-circle-ellipse-drawing-algorithm
	//			Also see: http://members.chello.at/~easyfilter/bresenham.html
	//	Calling parameters:
	//		xc, yc	Coordinate of point center of ellipse to be plotted.
	//				Valid Range:  0<= x0 <= 4095, 0<= y0 <= 4095
	//
	//		xr, yr	X-Radius of ellipse, Y-radius of ellipse
	//				Valid Range:  0<= xr <= 4095, 0<= yr <= 4095
	//
	//		arcSegment	8-bit code corresponding to the arc segments that are to be drawn.
	//					Bit = 1 means Draw arc segment, Bit = 0 means do NOT draw arc segment (Skip it!)
	//					Any combination from 1 to 8 segments may be spec'd.
	/*
	 1  |	 2
	 \	  |   /
	 0  \  |  /  3
	 -------+-------
	 7  /  |  \   4
	 /   |   \
						   6  |  5
	 */
	//
	//	Returns: NOTHING
	//
	//	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
	//
	//	CAUTIONS:	Caller should take care so that center + radius values do not exceed 4095.
	//				Negative coordinates or coordinate values >4095 will be masked to 12 bits and will
	//				be folded-over the edges of the valid plot area which is (x,y) = (0 to 4095, 0 to 4095).
	//
	//				NOTE: Routine will set plotErr to non-zero value and skip plotting if error in passed parameters are detected.
	//
	//	20170424	Ver 0.0	E.Andrews	First cut, Works but had erratic operation
	//	20170427	Ver 1.0	E.Andrews	This version uses some DOUBLE variables to correct occasional mis-shaoed ellipses!
	//	20170617 	Ver 1.1	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
	plotErr = 0;		//Initialize plotErr to FALSE (No errors)...
	//Check to see if passed coordinates fully fit into the display range
	int x0 = xc - xr;//x0,y0 = lower left hand corner of a rectangle bounding the totality of the Ellipse
	int y0 = yc - yr;
	int x1 = xc + xr;//x1,y1 = upper right hand corner of a rectangle bounding the totality of the Ellipse
	int y1 = yc + yr;
	short px, py;
	//==========Check to be sure that the figure is fully on-screen; Set PlotErr flag if NO
	if ((x0 > 4095) || (x0 < 0) || (x1 > 4095) || (x1 < 0))
		plotErr = 1;
	if ((y0 > 4095) || (y0 < 0) || (y1 > 4095) || (y1 < 0))
		plotErr = 1;

	//==========Skip drawing if passed parameters are faulted
	if (plotErr == 0) {
		int SkipCount = 0;	//Initialize the point-skip counter...
		double width = 1 * xr;
		int height = 1 * yr;
		//  The following variables must be DOUBLE or ellipse shape can
		//	be plotted badly deformed due to value over-flows
		//  Note: On Arduino DUE, sizeof(double)=8 bytes
		double a2 = width * width;
		double b2 = height * height;
		double fa2 = 4 * a2, fb2 = 4 * b2;
		double x, y, sigma;
		//Calculate segment 'HalfWayLimits'
		int xHalfWayLimit = xr * .707106;	//xHalfWayLimit = xr * sin(45Deg)
		int yHalfWayLimit = yr * .707106;	//yHalfWayLimit = xr * cos(45Deg)
		/* first half */
		for (x = 0, y = height, sigma = 2 * b2 + a2 * (1 - 2 * height);
				b2 * x <= a2 * y; x++) {
			if (SkipCount <= 0) {//Only plot the point if the arcsegment flag bit=1, otherwise, skip the point

				if (arcSegment & arc0 && x > xHalfWayLimit) {
					//plotPoint(xc - x, yc + y);		//Segment 0
					px = xc - x;
					py = yc + y;
					plotPoint(px, py);
					//Serial.print ("0, 0, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc1 && x <= xHalfWayLimit) {
					//plotPoint(xc - x, yc + y);		//Segment 1
					px = xc - x;
					py = yc + y;
					plotPoint(px, py);
					//Serial.print ("0, 1, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc2 && x < xHalfWayLimit) {
					//plotPoint(xc + x, yc + y);		//Segment 2
					px = xc + x;
					py = yc + y;
					plotPoint(px, py);
					//Serial.print ("0, 2, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc3 && y <= yHalfWayLimit) {
					//plotPoint(xc + x, yc + y);		//Segment 3					px=xc+x; py=yc+y;
					px = xc + x;
					py = yc + y;
					plotPoint(px, py);
					//Serial.print ("0, 3, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc4 && y < yHalfWayLimit) {
					//plotPoint(xc + x, yc - y);		//Segment 4
					px = xc + x;
					py = yc - y;
					plotPoint(px, py);
					//Serial.print ("0, 4, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc5 && x <= xHalfWayLimit) {
					//plotPoint(xc + x, yc - y);		//Segment 5
					px = xc + x;
					py = yc - y;
					plotPoint(px, py);
					//Serial.print ("0, 5, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc6 && x < xHalfWayLimit) {
					//plotPoint(xc - x, yc - y);		//Segment 6
					px = xc - x;
					py = yc - y;
					plotPoint(px, py);
					//Serial.print ("0, 6, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc7 && x >= xHalfWayLimit) {
					//plotPoint(xc - x, yc - y);		//Segment 7
					px = xc - x;
					py = yc - y;
					plotPoint(px, py);
					//Serial.print ("0, 7, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				SkipCount = _graphDensity;
				;
			} else
				SkipCount--;
			if (sigma >= 0) {
				sigma += fa2 * (1 - y);
				y--;
			}
			sigma += b2 * ((4 * x) + 6);
		}

		/* second half */
		//SkipCount=0;
		//int X, Y;
		for (x = width, y = 0, sigma = 2 * a2 + b2 * (1 - 2 * width);
				a2 * y <= b2 * x; y++) {
			if (SkipCount <= 0) {

				if (arcSegment & arc0 && x > xHalfWayLimit) {
					//plotPoint(xc - x, yc + y);		//Segment 0
					px = xc - x;
					py = yc + y;
					plotPoint(px, py);
					//Serial.print ("1, 0, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc1 && x <= xHalfWayLimit) {
					//plotPoint(xc - x, yc + y);		//Segment 1
					px = xc - x;
					py = yc + y;
					plotPoint(px, py);
					//Serial.print ("1, 1, ");Serial.print(px); Serial.print(", "); Serial.println(py);
				}

				if (arcSegment & arc2 && x < xHalfWayLimit) {
					//plotPoint(xc + x, yc + y);		//Segment 2
					px = xc + x;
					py = yc + y;
					plotPoint(px, py);
					//Serial.print ("1, 2, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc3 && y <= yHalfWayLimit) {
					//plotPoint(xc + x, yc + y);		//Segment 3
					px = xc + x;
					py = yc + y;
					plotPoint(px, py);
					//Serial.print ("1, 3, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc4 && y < yHalfWayLimit) {
					//plotPoint(xc + x, yc - y);		//Segment 4
					px = xc + x;
					py = yc - y;
					plotPoint(px, py);
					//Serial.print ("1, 4, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc5 && x <= xHalfWayLimit) {
					//plotPoint(xc + x, yc - y);		//Segment 5
					px = xc + x;
					py = yc - y;
					plotPoint(px, py);
					//Serial.print ("1, 5, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc6 && x < xHalfWayLimit) {
					//plotPoint(xc - x, yc - y);		//Segment 6
					px = xc - x;
					py = yc - y;
					plotPoint(px, py);
					//Serial.print ("1, 6, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				if (arcSegment & arc7 && x >= xHalfWayLimit) {
					//plotPoint(xc - x, yc - y);		//Segment 7
					px = xc - x;
					py = yc - y;
					plotPoint(px, py);
					//Serial.print ("1, 7, ");Serial.print(px);Serial.print(", ");	Serial.println(py);
				}

				SkipCount = _graphDensity;
				;
			} else
				SkipCount--;

			if (sigma >= 0) {
				sigma += fb2 * (1 - x);
				x--;
			}
			sigma += a2 * ((4 * y) + 6);
		}
	}
	return;
}

void XYscope::plotEllipse(int xc, int yc, int xr, int yr, uint8_t arcSegment) {
	//	Routine for non-rotated ELLIPSE plotting.
	//		Algorithm implementation uses standard geometry (not speed optized)
	//
	//	Calling parameters:
	//		xc, yc	Coordinate of point center of ellipse to be plotted.
	//				Valid Range:  0<= x0 <= 4095, 0<= y0 <= 4095
	//
	//		xr, yr	X-Radius of ellipse, Y-radius of ellipse
	//				Valid Range:  0<= xr <= 4095, 0<= yr <= 4095
	//
	//		arcSegment	8-bit code corresponding to the arc segments that are to be drawn.
	//					Bit = 1 means Draw arc segment, Bit = 0 means do NOT draw arc segment (Skip it!)
	//					Any combination from 1 to 8 segments may be spec'd.
	/*
	 1  |	 2
	 \	  |   /
	 0  \  |  /  3
	 -------+-------
	 7  /  |  \   4
	 /   |   \
						   6  |  5
	 */
	//
	//	Returns: NOTHING
	//
	//	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
	//
	//	CAUTIONS:	Caller should take care so that center + radius values do not exceed 4095.
	//				Negative coordinates or coordinate values >4095 will be masked to 12 bits and will
	//				be folded-over the edges of the valid plot area which is (x,y) = (0 to 4095, 0 to 4095).
	//
	//				NOTE: Routine will set plotErr to non-zero value and skip plotting if error in passed parameters are detected.
	//
	//	20170424	Ver 0.0	E.Andrews	First cut, Works but had erratic operation
	//	20170427	Ver 1.0	E.Andrews	This version uses some DOUBLE variables to correct occasional mis-shaoed ellipses!
	//	20170617 	Ver 1.1	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
	//	20170907	Ver 2.0 E.Andrews	Abandon Bresenham algorithm for slower floating point approach
	//									because new approach plots adjacent points and does not take advantage
	//									of figure symetry.This Allows AGI to run at a higher DMA_CLK freq.
	const float pi = atan(1) * 4;
	const float arcAng0 = 0;
	const float arcAng1 = pi * .25;		// .25 * pi
	const float arcAng2 = pi * .5;		// .50 * pi
	const float arcAng3 = pi * .75;		// .75 * pi
	const float arcAng4 = pi;			//1.00 * pi
	const float arcAng5 = pi * 1.25;	//1.25 * pi
	const float arcAng6 = pi * 1.50;	//1.50 * pi
	const float arcAng7 = pi * 1.75;	//1.75 * pi
	const float FullCircle = 2 * pi;	//2.00 * pi
	float circumf;
	//calculate ellipse circumference

	circumf = FullCircle * sqrt((pow(xr, 2) + pow(yr, 2)) / 2.);
	//Figure out how many points we will actually be plotting
	//based on the current graphicsIntensity (_graphicsDensity) setting
	float numOfPoints = circumf / _graphDensity;
	//Calc the deltaAng between points
	float deltaAng = FullCircle / (numOfPoints);

	float angle;
	short X, Y;

	for (angle = 0; angle < FullCircle; angle = angle + deltaAng) {
		X = int(xc - cos(angle) * xr);
		Y = int(yc + sin(angle) * yr);

		//Only plot those segments that are enabled by passed "arcSegment" variable
		if (arcSegment & arc0 && angle <= arcAng1)
			plotPoint(X, Y);					//Segment 0
		if (arcSegment & arc1 && angle > arcAng1 && angle <= arcAng2)
			plotPoint(X, Y);	//Segment 1
		if (arcSegment & arc2 && angle > arcAng2 && angle <= arcAng3)
			plotPoint(X, Y);	//Segment 2
		if (arcSegment & arc3 && angle > arcAng3 && angle <= arcAng4)
			plotPoint(X, Y);	//Segment 3
		if (arcSegment & arc4 && angle > arcAng4 && angle <= arcAng5)
			plotPoint(X, Y);	//Segment 4
		if (arcSegment & arc5 && angle > arcAng5 && angle <= arcAng6)
			plotPoint(X, Y);	//Segment 5
		if (arcSegment & arc6 && angle > arcAng6 && angle <= arcAng7)
			plotPoint(X, Y);	//Segment 6
		if (arcSegment & arc7 && angle > arcAng7)
			plotPoint(X, Y);						//Segment 7
	}
}
/*

 void XYscope::plotEllipse_BAK(int xc, int yc, int xr, int yr,uint8_t arcSegment=255)
 //TODO Rework this routine
 {
 //	Routine for horizontal ELLIPSE plotting.
 //		Algorithm implementation/starting code base from: 
 //			https://sites.google.com/site/ruslancray/lab/projects/bresenhamscircleellipsedrawingalgorithm/bresenham-s-circle-ellipse-drawing-algorithm
 //			Also see: http://members.chello.at/~easyfilter/bresenham.html 
 //	Calling parameters:
 //		xc, yc	Coordinate of point center of ellipse to be plotted.  
 //				Valid Range:  0<= x0 <= 4095, 0<= y0 <= 4095
 //
 //		xr, yr	X-Radius of ellipse, Y-radius of ellipse   
 //				Valid Range:  0<= xr <= 4095, 0<= yr <= 4095
 //	
 //		arcSegment	8-bit code corresponding to the arc segments that are to be drawn.
 //					Bit = 1 means Draw arc segment, Bit = 0 means do NOT draw arc segment (Skip it!)
 //					Any combination from 1 to 8 segments may be spec'd.

 1  |	 2
 \	  |   /
 0  \  |  /  3
 -------+-------
 7  /  |  \   4
 /   |   \
						   6  |  5

 //
 //	Returns: NOTHING
 //
 //	Global Variables: Loads points int XY_List, updates XYlistEnd pointer
 //
 //	CAUTIONS:	Caller should take care so that center + radius values do not exceed 4095.
 //				Negative coordinates or coordinate values >4095 will be masked to 12 bits and will
 //				be folded-over the edges of the valid plot area which is (x,y) = (0 to 4095, 0 to 4095).	
 //
 //				NOTE: Routine will set plotErr to non-zero value and skip plotting if error in passed parameters are detected.
 //
 //	20170424	Ver 0.0	E.Andrews	First cut, Works but had erratic operation
 //	20170427	Ver 1.0	E.Andrews	This version uses some DOUBLE variables to correct occasional mis-shaoed ellipses!
 //	20170617 	Ver 1.1	E.Andrews	Simplify Routine Call by eliminating need to pass the index pointer
 
 plotErr=0;		//Initialize plotErr to FALSE (No errors)...
 //Check to see if passed coordinates fully fit into the display range
 int x0 = xc - xr;	//x0,y0 = lower left hand corner of a rectangle bounding the totality of the Ellipse
 int y0 = yc - yr;
 int x1 = xc + xr;	//x1,y1 = upper right hand corner of a rectangle bounding the totality of the Ellipse
 int y1 = yc + yr;
 //==========Check to be sure that the figure is fully on-screen; Set PlotErr flag if NO
 if ((x0>4095 )||(x0<0)||(x1>4095 )||(x1<0)) plotErr=1;
 if ((y0>4095 )||(y0<0)||(y1>4095 )||(y1<0)) plotErr=1;

 //==========Skip drawing if passed parameters are faulted
 if (plotErr==0){
 int SkipCount = 0;	//Initialize the point-skip counter...
 double width=1*xr;
 int height=1*yr;		
 //  The following variables must be DOUBLE or ellipse shape can
 //	be plotted badly deformed due to value over-flows
 //  Note: On Arduino DUE, sizeof(double)=8 bytes
 double a2 = width * width;			
 double b2 = height * height;
 double fa2 = 4 * a2, fb2 = 4 * b2;
 double x, y, sigma;
 //Calculate segment 'HalfWayLimits'
 int xHalfWayLimit=xr*.707106;	//xHalfWayLimit = xr * sin(45Deg)
 int yHalfWayLimit=yr*.707106;	//yHalfWayLimit = xr * cos(45Deg)
 first half
 for (x = 0, y = height, sigma = 2*b2+a2*(1-2*height); b2*x <= a2*y; x++)
 {
 if(SkipCount<=0){	//Only plot the point if the arcsegment flag bit=1, otherwise, skip the point

 if (arcSegment & arc0 && x >  xHalfWayLimit) plotPoint(xc - x, yc + y);		//Segment 0

 if (arcSegment & arc1 && x <= xHalfWayLimit) plotPoint(xc - x, yc + y);		//Segment 1

 if (arcSegment & arc2 && x < xHalfWayLimit)  plotPoint(xc + x, yc + y);		//Segment 2 

 if (arcSegment & arc3 && y <= yHalfWayLimit) plotPoint(xc + x, yc + y);		//Segment 3

 if (arcSegment & arc4 && y <  yHalfWayLimit) plotPoint(xc + x, yc - y);		//Segment 4

 if (arcSegment & arc5 && x <= xHalfWayLimit) plotPoint(xc + x, yc - y);		//Segment 5

 if (arcSegment & arc6 && x <  xHalfWayLimit) plotPoint(xc - x, yc - y);		//Segment 6

 if (arcSegment & arc7 && x >= xHalfWayLimit) plotPoint(xc - x, yc - y);		//Segment 7
 
 SkipCount=_graphDensity;;
 } else SkipCount--;
 if (sigma >= 0)
 {
 sigma += fa2 * (1 - y);
 y--;
 }
 sigma += b2 * ((4 * x) + 6);
 }

 second half
 //SkipCount=0;
 for (x = width, y = 0, sigma = 2*a2+b2*(1-2*width); a2*y <= b2*x; y++)
 {
 if(SkipCount<=0){

 if (arcSegment & arc0 && x >  xHalfWayLimit) plotPoint(xc - x, yc + y);		//Segment 0

 if (arcSegment & arc1 && x <= xHalfWayLimit) plotPoint(xc - x, yc + y);		//Segment 1

 if (arcSegment & arc2 && x < xHalfWayLimit)  plotPoint(xc + x, yc + y);		//Segment 2 

 if (arcSegment & arc3 && y <= yHalfWayLimit) plotPoint(xc + x, yc + y);		//Segment 3

 if (arcSegment & arc4 && y <  yHalfWayLimit) plotPoint(xc + x, yc - y);		//Segment 4

 if (arcSegment & arc5 && x <= xHalfWayLimit) plotPoint(xc + x, yc - y);		//Segment 5

 if (arcSegment & arc6 && x <  xHalfWayLimit) plotPoint(xc - x, yc - y);		//Segment 6

 if (arcSegment & arc7 && x >= xHalfWayLimit) plotPoint(xc - x, yc - y);		//Segment 7
 
 SkipCount=_graphDensity;;
 } else SkipCount--;
 
 if (sigma >= 0)
 {
 sigma += fb2 * (1 - x);
 x--;
 }
 sigma += a2 * ((4 * y) + 6);
 }
 }	
 return;
 }

 */

void XYscope::plotArduinoLogo(int& charX, int& charY, int& charHt) {
	//	Plots the horizontal, figure-8 Arduino LOGO to the screen,  Displays "ARDUINO DUE"
	//	text below the figure-8 logo.
	//
	//	Calling parameters:
	//
	//		x0, y0	Coordinate of lower left hand corner of logo and as passed by REFERENCE and will be updated by
	//				this routine; they will be incremented (and returned) pointing to the next character position to the right.
	//				(x0,y0) values will	wrap to right most character position in the next (lower) row when end-of-line is reached.
	//
	//		CharHt	Logo_height in pixels, Range: 16<=CharHt<=4095
	//				Note: Logo_width is 1.6 X Logo_height!
	//
	//-----COMMON VARIABLES----- (Not passed during procedure call)
	//		 Density is a COMMON variable that is changed by calling the setDensity() routine.
	//				Higher values of Density increases the distance between ajacent plotted points of graphics figures.
	//				Smaller values of Density reduces the distand between adjacent plotted points of graphics figures.
	//				Valid numbers are 0 to 100.  Note: depending upon size & focus of CRT, values more ~50 shows
	//				cause lines of circles and vectors to begin to appear as dotted lines.
	//
	//	Returns: charX & charY are updated.
	//
	//	201703719 Ver 0.0	E.Andrews	First cut
	//

	plotErr = 0;
	//Make CharScale Factor
	float Char_ScaleFactor = float(charHt) / 16;

	//Plot the two logo characters, 13 spaces apart

	int Tx = charX + 13, Ty = charY;
	plotChar(128, charX, charY, charHt);

	//Move charX to the right by 13*CharHt places...
	charX = Tx + 13 * Char_ScaleFactor;	//Fixed, non-proportional Font...

	charY = Ty;
	plotChar(129, charX, charY, charHt);
	Ty = Ty - charHt / 16;
	Tx = Tx + charHt / 8;//Starting location for "Ardu.." text for PROPORTIONAL FONT
	if (getFontSpacing() != mono)
		Tx = Tx + ((charHt / 8) * .75);	//Starting location for "Ardu.." text for PROPORTIONAL FONT
	printSetup(Tx, Ty, charHt / 5);
	print("ARDUINO DUE");

	return;
}

const short prop = 0, monoTight = 8, mono = 10, monoNorm = 10, monoWide = 12;//Define FontSpacing  Constants

void XYscope::setFontSpacing(short spacingMode) {
	//Set Font Proportional or mono spaced mode...
	//
	//	Calling parameters:
	//
	//		spacingMode  Valid range 0-15.  Normal values are: prop=0, monoTight=8, mono=10, monoNorm=10, monoWide=12;
	//					representing Proportional Spacing, tight Mono Spacing, Normal Mono Spacing, Wide Mono Spacing
	//
	//-----COMMON VARIABLES-----
	//		fontSpacing
	//
	//	Returns: Nothing.
	//
	//	20170906 Ver 0.0	E.Andrews	First cut
	//
	if (spacingMode >= 0 && spacingMode < 16)
		_fontSpacing = spacingMode;
	//TODO Serial.print ("SpacingMode=");Serial.print(spacingMode); Serial.print(", _fontSpacing=");Serial.println(_fontSpacing);
	return;
}

short XYscope::getFontSpacing() {
	//Retrieve current font spacing value.
	//
	//	Calling parameters:  NONE
	//
	//-----COMMON VARIABLES-----
	//		fontSpacing
	//
	//	Returns: Current fontSpacing Value.  Normal values are: prop=0, monoTight=8, mono=10, monoNorm=10, monoWide=12;
	//			 representing Proportional Spacing, tight Mono Spacing, Normal Mono Spacing, Wide Mono Spacing
	//
	//	20170906 Ver 0.0	E.Andrews	First cut
	//

	return _fontSpacing;
}
void XYscope::plotChar(char c, int& charX, int& charY, int& charHt) {
	//	Writes a single CHARACTER to screen

	//	Calling parameters:
	//
	//		c		Ascii Charactor to be plotted, Range: 0<= digit <= 255
	//				Note: Invalid or undefined characters will be plotted as "~"
	//		
	//		x0, y0	Coordinate of lower left hand cornder of character cell and as passed by REFERENCE and will be updated by
	//				this routine; they will be incremented (and returned) pointing to the next character position to the right.
	//				(x0,y0) values will	wrap to right most character position in the next (lower) row when end-of-line is reached.
	//
	//		CharHt	Character height in pixels, Range: 16<=CharHt<=4095
	//
	//-----COMMON VARIABLES----- (Not passed during procedure call)
	//		 _textDensity is a COMMON variable that sets the plot brightness during plotChar operaitons.
	//				Higher values of Density increases the distance between ajacent plotted points of graphics figures.
	//				Smaller values of Density reduces the distand between adjacent plotted points of graphics figures.
	//
	//	Returns: x0 & y0 are updated.
	//
	//	20170609 Ver 0.0	E.Andrews	First cut
	//	20170714 Ver 1.0	E.Andrews	Simplified calling sequence
	//	20170714 Ver 1.1	E.Andrews	Updated to use _textDensity value durin plotChar operations
	//	20170905 Ver 2.0	E.Andrews	Added Proportional Font spacing into ROM & Ascii2Font arrays
	//
	boolean printDump = false; //Diagnostics flag used during code development (set to false to prevent debug message output)
	plotErr = 0;

	//Make CharScale Factor
	float Char_ScaleFactor = float(charHt) / 16;

	int EndOfPlot = 0;

	int PlotCount = 10;

	//Get pointer into the fontROM
	int charRomIndex = int((Ascii2Font[int(c)] & 0x1ff)) - 1;//limit return to low order 9 bits
	//Get Width of character (if it is present)
	int charWidth;
	charWidth = _fontSpacing;
	//TODO Serial.print(" plotChar -");
	if (_fontSpacing == 0) {
		charWidth = int((Ascii2Font[int(c)]) >> 9);	//extract char width from b10-b13 (if available)

		charWidth = charWidth & 0x0f;				//mask value to 0-15 range.
		//TODO Serial.print(c); Serial.print(", width = ");Serial.print(charWidth);
	}

	//TODO Serial.print(", Final_width = ");Serial.println(charWidth);

	//For Character Plotting, temporarily set  _density value to _textDensity
	short tempDensity = _graphDensity;
	_graphDensity = _textDensity;

	while (EndOfPlot == 0) {
		PlotCount--;
		charRomIndex++;
		//Retrieve OpCode
		uint8_t OC = 0, EOC = 0, P1a = 0, P1b = 0, P2a = 0, P2b = 0, P3 = 0;

		//Unpack OPCODE, EndOfChar Flag, P1,P2,P3
		OC = DigitFont[charRomIndex].Opcode;	//Retrieve OPCODE from FontROM
		if ((OC & 0x80) != 0)
			EndOfPlot++;//Test to see if End_of_Charaacter (EOC) is set; end processing of so
		OC = OC & 0x0f;						//Strip down to just lower 4 bits
		P1b = DigitFont[charRomIndex].P1;//Retrieve P1 from FontROM and unpack 
		P1a = P1b >> 4;
		P1b = P1b & 0x0f;
		P2b = DigitFont[charRomIndex].P2;//Retrieve P2 from FontROM and unpack 
		P2a = P2b >> 4;
		P2b = P2b & 0x0f;
		P3 = DigitFont[charRomIndex].P3;			//Retrieve P3 from FontROM

		//Process each graphics element code
		uint8_t const NOP = 0x0, PNT = 0x1, LIN = 0x2, REC = 0x3, CIR = 0x4,
				ELP = 0x5;//NOP = No Operaton, PNT = plot a single POINT, LIN = Plot a LINE
		//REC = Plot a Rectangle, CIR = Plot a Circle or a partial circular segment 
		//ELP = Plot an Ellipse or a partial ellipical segment.
		//translate to plotting location and scale ROM data by CharHeight value
		int x0 = P1a * Char_ScaleFactor + charX;//Start X for lines, points, rectangles...Center point for Circles & Ellipses												
		int y0 = P1b * Char_ScaleFactor + charY;//Start Y for lines, points, rectangles...Center point for Circles & Ellipses
		int x1 = P2a * Char_ScaleFactor + charX;//End X for lines and Rectangles												
		int y1 = P2b * Char_ScaleFactor + charY;//End Y for Lines and Rectangles
		int xCP = x0;					//X Center Point for Circle & Ellipse
		int yCP = y0;					//Y Center Point for Circle & Ellipse
		int r1 = P2a * Char_ScaleFactor;//Radius for Circle, RadiusX for Ellipse
		int r2 = P2b * Char_ScaleFactor;				//RadiusY for Ellipse
		uint8_t SegmentFlag = P3;		//Segment flag for Circles and Ellipse

		if (printDump) {
			//lookup char in ROM and start processing

			Serial.println("|----------plotChar Data Dump -------------|");
			Serial.print("charIndex = ");
			Serial.print(charRomIndex);
			Serial.print("  (charX,charY)= ");
			Serial.print(charX);
			Serial.print(",");
			Serial.print(charY);
			Serial.print("   Char_ScaleFactor = ");
			Serial.println(Char_ScaleFactor, 2);
			Serial.print("  OpCode,P1,P2,P3 = ");
			Serial.print(OC, HEX);
			Serial.print(", ");
			Serial.print(DigitFont[charRomIndex].P1, HEX);
			Serial.print(", ");
			Serial.print(DigitFont[charRomIndex].P2, HEX);
			Serial.print(", ");
			Serial.print(DigitFont[charRomIndex].P3, HEX);
			Serial.print("   .... P1a,P1b,P2a,P2b,P3 = ");
			Serial.print(P1a);
			Serial.print(", ");
			Serial.print(P1b);
			Serial.print(", ");
			Serial.print(P2a);
			Serial.print(", ");
			Serial.print(P2b);
			Serial.print(", ");
			Serial.print(P3, HEX);
			Serial.println();
			Serial.print("   (x0,y0) = ");
			Serial.print(x0);
			Serial.print(",");
			Serial.print(y0);
			Serial.print("   (x1,y1) = ");
			Serial.print(x1);
			Serial.print(",");
			Serial.print(y1);
			Serial.print("   (xCP,yCP) = ");
			Serial.print(xCP);
			Serial.print(",");
			Serial.print(yCP);
			Serial.print("   (r1,r2) = ");
			Serial.print(r1);
			Serial.print(",");
			Serial.print(r2);
			Serial.print("   SegmentFlag = ");
			Serial.print(SegmentFlag, HEX);

		}

		int ix = XYlistEnd;
		switch (OC) {
		case NOP:	//Do nothing
			if (printDump)
				Serial.print("    OpCode= NOP");
			break;
		case PNT:	//Plot a point 		P1=(x0,y0), P2 Not Used, P3 Not Used
			if (printDump)
				Serial.print("    OpCode= PNT");
			plotPoint(x0, y0);
			break;
		case LIN:	//plot a line		P1=(x0,y0), P2=(x1,y1), P3 Not Used
			if (printDump)
				Serial.print("    OpCode= LIN");
			plotLine(x0, y0, x1, y1);
			break;
		case REC:	//plot a rectangle	P1=(x0,y0), P2=(x1,y1), P3 Not Used
			if (printDump)
				Serial.print("    OpCode= REC");
			plotRectangle(x0, y0, x1, y1);
			break;
		case CIR://plot a circle		P1=(xCenterPoint,yCenterPoint), P2=(radiusX,not used), P3=Segment Flags (0-7)
			if (printDump)
				Serial.print("    OpCode= CIR");
			plotCircle(xCP, yCP, r1, SegmentFlag);
			break;
		case ELP://plot an ellipse	P1=(xCenterPoint,yCenterPoint), P2=(radiusX,radiusY), P3=Segment Flags (0-7)
			if (printDump)
				Serial.print("    OpCode= ELP");
			plotEllipse(xCP, yCP, r1, r2, SegmentFlag);
			break;
		default:	//Do Nothing
			break;
		}

		if ((OC & 0x80) != 0)
			EndOfPlot++;	//Does this instruction have the EndOfChar flag set?
		if (PlotCount == 10)
			EndOfPlot++;//Backup check to prevent more than MaxCount of graphic primatives to be processed (prevents run-ons...)
		//if (EndOfPlot!=0) Serial.println (" <<-END OF CHARACTER"); else Serial.println("");

	}
	//Done with Character Plotting, reset  _graphDensity value to teh temp value...
	_graphDensity = tempDensity;

	//Move charX to the right by one character size...
	charX = charX + charWidth * Char_ScaleFactor;//Move to the right one place...
	if (charX + charHt > 4095) {//Perform NEW LINE function if we will spill over...
		charX = 0;
		charY = charY - 15 * Char_ScaleFactor;
	}

	return;
}

void XYscope::plotCharUL(char c, int& charX, int& charY, int& charHt) {
	//	Writes a single CHARACTER to screen with UNDERLINE
	//
	//	Calling Paramerters:
	//
	//		c		Ascii Charactor to be plotted, Range: 0<= digit <= 255
	//				Note: Invalid or undefined characters will be plotted as "~"
	//				Character will be printed with UNDERLINE beneath it!
	//
	//		x0, y0	Coordinate of lower left hand corner of character cell and as passed by REFERENCE and will be updated by
	//				this routine; they will be incremented (and returned) pointing to the next character position to the right.
	//				(x0,y0) values will	wrap to right most character position in the next (lower) row when end-of-line is reached.
	//
	//		CharHt	Character height in pixels, Range: 16<=CharHt<=4095
	//
	//-----COMMON VARIABLES----- (Not passed during procedure call)
	//		 TextIntensity is a COMMON variable that is changed by calling the setTextIntensity() routine.
	//				Higher values of Density increases the distance between adjacent plotted points of graphics figures.
	//				Smaller values of Density reduces the distand between adjacent plotted points of graphics figures.
	//				Valid numbers are 0 to 100.  Note: depending upon size & focus of CRT, values more ~50 shows
	//				cause lines of circles and vectors to begin to appear as dotted lines.
	//
	//	Returns: x0 & y0 are updated.
	//
	//	20170718 Ver 0.0	E.Andrews	First cut
	//	20170905 Ver 2.0	E.Andrews	Reordered things so Proportional Font spacing will work for UL characcters
	//
	int charX_Temp = charX;	//Remember the original passed Coordinates
	int charY_Temp = charY;
	int c_Temp = c;
	c = '_';
	plotChar(c, charX_Temp, charY_Temp, charHt);	//Now print the UNDERLINE

	plotChar(c_Temp, charX, charY, charHt);			//Print the base character

	return;
}

void XYscope::printSetup(short textX, short textY) {
	//	Sets X and Y location for subsequent calls to XYscope.print(...)
	//
	//	Calling parameters:
	//	
	//		x0, y0	Coordinate of lower left hand corner of character cell.  Range: 0 to 4095.
	//
	//-----COMMON VARIABLES----- (Not passed during procedure call)
	//		 Density is a COMMON variable that is changed by calling the setDensity() routine.
	//				Higher values of Density increases the distance between ajacent plotted points of graphics figures.
	//				Smaller values of Density reduces the distand between adjacent plotted points of graphics figures.
	//				Valid numbers are 0 to 100.  Note: depending upon size & focus of CRT, values more ~50 shows
	//				cause lines of circles and vectors to begin to appear as dotted lines.
	//	Returns:  NOTHING
	//
	//	201703610 Ver 0.0	E.Andrews	First cut
	//
	charX = textX;
	charY = textY;
}

void XYscope::printSetup(short textX, short textY, short textSize = -1) {
	//	Sets X and Y location for subsequent calls to XYscope.print(...)
	//
	//	Calling parameters:
	//	
	//		x0, y0	Coordinate of lower left hand corner of character cell.  Range: 0 to 4095.
	//
	//		textSize	Character height in pixels, Range: 16<=CharHt<=4095
	//
	//-----COMMON VARIABLES----- (Not passed during procedure call)
	//		 Density is a COMMON variable that is changed by calling the setDensity() routine.
	//				Higher values of Density increases the distance between ajacent plotted points of graphics figures.
	//				Smaller values of Density reduces the distand between adjacent plotted points of graphics figures.
	//				Valid numbers are 0 to 100.  Note: depending upon size & focus of CRT, values more ~50 shows
	//				cause lines of circles and vectors to begin to appear as dotted lines.
	//	Returns:  NOTHING
	//
	//	201703610 Ver 0.0	E.Andrews	First cut
	//
	charX = textX;
	charY = textY;
	if (textSize > 0)
		charSize = textSize;
}

void XYscope::printSetup(short textX, short textY, short textSize = -1,
		short textBright = -1) {
	//	Sets X and Y location for subsequint calls to XYscope.print(...) 
	//
	//	Calling parameters:
	//	
	//		x0, y0	Coordinate of lower left hand cornder of character cell.  Range: 0 to 4095.
	//
	//		textSize	Character height in pixels, Range: 16<=CharHt<=4095
	//
	//		textBright	Sets text brightness (%). Range: 1 to approx 255, Larger=BRIGHTER.
	//				Values less than 25% will be less bright and or lower text quality.
	//
	//-----OTHER NOTES
	//	Use int GetTextIntensity() to read current text setting.
	//	SetTextIntensity(value) is an alternate way to set text brightness value
	//	Returns:  NOTHING
	//
	//	201703610 Ver 0.0	E.Andrews	First cut
	//
	charX = textX;
	charY = textY;
	if (textSize > 0)
		charSize = textSize;
	if (textBright > 0)
		setTextIntensity(textBright);
}

void XYscope::print(char* text) {
	print(text, bool(false));
	return;
}
void XYscope::print(char* text, bool UL_Flag = false) {
	//	Print text sring to screen at current x-y locations. 
	//
	//	Calling parameters:
	//	
	//		text	String of text to be printed to screen
	//
	//		UL_Flag	Boolean, true=UNDERLINE text, False=NO-UNDERLINE
	//
	//  Other Notes
	//		Use PrintSetup(...) to set XY locations, textSize, and textBrightness
	//
	//	Returns:  NOTHING
	//
	//	201703610 Ver 0.0	E.Andrews	First cut
	//	
	int textPtr = 0;
	while (text[textPtr] != 0) {
		if (UL_Flag)
			plotCharUL(text[textPtr], charX, charY, charSize);
		else
			plotChar(text[textPtr], charX, charY, charSize);
		textPtr++;
	}

}
void XYscope::printUnderline(int nPlaces) {
	//	Print underline character to screen at current x-y locations.
	//
	//	Calling parameters:
	//
	//		nPlaces	Integer, value=Number of Underline places to write
	//
	//  Other Notes
	//		Use PrintSetup(...) to set XY locations, textSize, and textBrightness
	//
	//	Returns:  NOTHING
	//
	//	201703610 Ver 0.0	E.Andrews	First cut
	//	201703718 Ver 1.0	E.Andrews	Update to work with UNDERLINE flag
	//
	for (nPlaces; nPlaces > 0; nPlaces--)
		;
	{
		plotChar('_', charX, charY, charSize);
	}
	return;
}
void XYscope::print(int number) {
	//	Print integer number to screen at current x-y locations.
	//
	//	Calling parameters:
	//
	//		number	Integer, value to be printed to screen
	//
	//	Returns:  NOTHING
	//
	//  Other Notes
	//		Use PrintSetup(...) to set XY locations, textSize, and textBrightness
	//
	//	201703610 Ver 0.0	E.Andrews	First cut
	//	201703718 Ver 1.0	E.Andrews	Update to work with UNDERLINE flag
	//

	print(number, bool(false));
	return;
}
void XYscope::print(int number, bool UL_Flag = false) {
	//	Print integer number to screen at current x-y locations. 
	//
	//	Calling parameters:
	//	
	//		number	Integer, value to be printed to screen
	//
	//		UL_Flag	Boolean, true=UNDERLINE text, False=NO-UNDERLINE
	//
	//	Returns:  NOTHING
	//
	//  Other Notes
	//		Use PrintSetup(...) to set XY locations, textSize, and textBrightness
	//
	//	201703610 Ver 0.0	E.Andrews	First cut
	//	201703718 Ver 1.0	E.Andrews	Add UNDERLINE capability
	//

	char charsToPrint[30] = { 0 };
	int digitCount = 0;

	if (number < 0) {
		//Serial.print("-");
		if (UL_Flag)
			plotCharUL('-', charX, charY, charSize);
		else
			plotChar('-', charX, charY, charSize);
		number = abs(number);		//Convert NEG number to POSITIVE
	}
	//Disect value, extract individual digits into array and count digits extracted
	if (number == 0) {
		charsToPrint[0] = '0';
		digitCount = 0;
	} else {
		while (number > 0 && digitCount < 30) {
			//Serial.print("  Extracted["); Serial.print(digitCount); Serial.print("]:"); Serial.print(number%10);
			//Extract least significant digit and store it into array as an ASCII character
			charsToPrint[digitCount] = '0' + number % 10;
			number = number / 10;
			digitCount++;
		}
		digitCount--;
	}

	//Serial.println();

	//Now print stored Ascii digits in proper order!
	for (int i = digitCount; i >= 0; i--) {
		//Serial.print( i);Serial.print(":");Serial.print(charsToPrint[i]);Serial.print("   ");
		//Print each char digit to the screen!
		if (UL_Flag)
			plotCharUL(charsToPrint[i], charX, charY, charSize);
		else
			plotChar(charsToPrint[i], charX, charY, charSize);

	}
	//Serial.println();
}

void XYscope::print(float number) {
	//	Print floating point number to screen at current x-y locations. 
	//
	//	Calling parameters:
	//	
	//		number	Floating point value to be printed to screen, defaults to show two digits to the right of the DP displayed.
	//
	//	Returns:  NOTHING
	//
	//  Other Notes
	//		Use PrintSetup(...) to set XY location, textSize, and textBrightness
	//
	//	201703610 Ver 0.0	E.Andrews	First cut
	//

	print(float(number), 2, bool(false));//Default: 2 places to the right of the DP.

}

void XYscope::print(float number, int placesToPrint = 2) {
	//	Print floating point number to screen at current x-y locations. 
	//
	//	Calling parameters:
	//	
	//		number			Float, value to be printed to screen, defaults to show two digits to the right of the DP displayed.
	//
	//		placesToPrint	Integer, value defines number ot digits to the right of the dec point that should be plotted.
	//
	//	Returns:  NOTHING
	//
	//  Other Notes
	//		Use PrintSetup(...) to set XY location, textSize, and textBrightness
	//
	//	201703610 Ver 0.0	E.Andrews	First cut
	//

	print(float(number), placesToPrint, bool(false));

}

void XYscope::print(float number, int placesToPrint = 2, bool UL_Flag = false) {
	//	Print floating point number to screen at current x-y locations.
	//
	//	Calling parameters:
	//
	//		number			Float, value to be printed to screen, defaults to show two digits to the right of the DP displayed.
	//
	//		placesToPrint	Integer, value defines number ot digits to the right of the dec point that should be plotted.
	//
	//		UL_Flag=false	Boolean, TRUE=underline digits, FALSE= do not underline digits
	//
	//	Returns:  NOTHING
	//
	//  Other Notes
	//		Use PrintSetup(...) to set XY locations, textSize, and textBrightness
	//
	//	201703610 Ver 0.0	E.Andrews	First cut
	//

	//check and bound placesToPrint to a reasonable number
	if (placesToPrint < 0 || placesToPrint > 10)
		placesToPrint = 2;//Reset placesToPrint to reasonable number if it's out of bounds!

	if (number < 0) {
		//Serial.print("-");
		//Plot a NEGATIVE sign for negative numbers
		if (UL_Flag)
			plotCharUL('-', charX, charY, charSize);
		else
			plotChar('-', charX, charY, charSize);
	}

	number = abs(number);
	int digitCount = 0;

	// Figure out how many digits we will be printing for the Integer Portion of the number...
	//intNumber=int(number);	//Capture an integer version of the floating point number

	//Serial.println("  Figuring out how many digits to print...");
	while ((number / (pow(10, digitCount))) > 9.99999999) {
		//Serial.print(digitCount);Serial.print("=");Serial.print(pow(10,digitCount)); Serial.print( " : "); Serial.print((number/pow(10,digitCount))); Serial.print("_");
		digitCount++;
	}
	//Serial.print("  Here's the num of digits I think I must print for the Integer part: ");Serial.println(digitCount);
	//Now print just the integer part of the number...

	while (digitCount >= 0) {
		number = abs(number);	//disect and print it as a positive number...
		int digitToPrint = number / pow(10, digitCount);
		char charToPrint = '0' + digitToPrint;
		//plotChar(charToPrint,charX,charY,charSize);;	//Convert each digit to a single ASCII character and print it!
		if (UL_Flag)
			plotCharUL(charToPrint, charX, charY, charSize);
		else
			plotChar(charToPrint, charX, charY, charSize);//Convert digit to a single ASCII character and print it!
		//Serial.print(digitToPrint); Serial.print("_");
		//Serial.print("[");
		//Serial.print(charToPrint);Serial.Print("]  ");

		number = number - digitToPrint * pow(10, digitCount);
		digitCount--;
	}
	// Insert DP character Here!!
	//plotCircle(charX-int ix, int xc, int yc, int r)
	//plotPoint();
	if (UL_Flag)
		plotCharUL('.', charX, charY, charSize);
	else
		plotChar('.', charX, charY, charSize);
	number = number * pow(10, placesToPrint);
	digitCount = placesToPrint - 1;

	while (digitCount >= 0) {
		number = abs(number);	//disect and print it as a positive number...
		int digitToPrint = number / pow(10, digitCount);
		char charToPrint = '0' + digitToPrint;
		if (UL_Flag)
			plotCharUL(charToPrint, charX, charY, charSize);
		else
			plotChar(charToPrint, charX, charY, charSize);//Convert digit to a single ASCII character and print it!
		//Serial.print(digitToPrint); Serial.print("_");
		//Serial.print("[");
		//Serial.print(charToPrint);Serial.Print("]  ");

		number = number - digitToPrint * pow(10, digitCount);
		digitCount--;
	}
	//Serial.println();

}

void XYscope::setDmaClockRate(uint32_t New_XfrRate_hz) {
	//	Routine to setup Timer Counter 0 which is used to clock DMA transformers.
	//	(This is just a CALLER FREINDLIER version of txSetup)
	//
	//	Begin automatically calls this routine.  User only needs to call this 
	//	this routine if he/she wants to change clock rate during runtime.
	//
	//	Calling parameters:
	//	
	//	New_XfrRateHZ	This is the intended DMA Transfer Rate in Hertz.
	//				Note that Point_Clk_Freq = XfrRateHz/2.
	//
	//	CAUTIONS:	Usual XfrRateHz values should be in 300,000 to 800,000 range.
	//
	//				Lower values make the screen refresh take longer and therefore
	//				reduces the total number of points that can be displayed before
	//				CRT flicker becomes visually apparant.
	//
	//				Higher values demand more of the XY display/oscilloscope 
	//				hardware but do allow more points to be plotted before refresh
	//				flicker is a apparent.
	//
	//	Returns:	Nothing.
	//	Other Notes:
	//		This routine calculates and set the front and back porch of the blanking signals
	//		based on the clock rate specified when it is called.
	//
	//	20170405 Ver 0.0	E.Andrews	First cut
	//
	//This routine sets up TimerCounter0
	//TC0 is used to drive DAC0 & dAC1

	tcSetup(New_XfrRate_hz);
	DmaClkPeriod_us = (float(1 / float(New_XfrRate_hz)) * 1000000.);
	//Calculate new value and Update the 'FRONT-PORCH' blankCount
	//serial.print("\n   DmaClkPeriod_us=");Serial.print(DmaClkPeriod_us);Serial.print("  blankCount=").Serial.println(blankCount);
	frontPorchBlankCount = int(DmaClkPeriod_us * 186.23 - 52.1 + .5);
	backPorchBlankCount = int(DmaClkPeriod_us * 79. - 36 + .5);
	Serial.print("\n   DmaClkPeriod_us=");
	Serial.print(DmaClkPeriod_us);
	Serial.print("  frontPorchBlankCount=");
	Serial.print(frontPorchBlankCount);
	Serial.print(", backPorchBlankCount=");
	Serial.println(backPorchBlankCount);

}

void XYscope::tcSetup(uint32_t New_XfrRateHz) {
	//	Routine to setup Timer Counter 0.  TC0 is used to clock DMA transformers.
	//
	//	User does NOT usually directly call this routine; XYscope.Begin automatically
	//	does Counter timer setup.
	//
	//	Calling parameters:
	//	
	//	XfrRateHZ	User specifies the desired intended DMA Transfer Rate in Hertz.
	//				Note that Point_Clk_Freq = XfrRateHz/2.
	//
	//				Typically, XfrRateHz should be in the 300,000-800,000 range.
	//				User may need to experiment with this value to see what works best
	//				for the'scope'being used as a display screen.
	//
	//				Smaller values make the screen refresh take longer and therefore
	//				reduces the total number of points that can be displayed before
	//				CRT flicker becomes visually apparant.
	//
	//				Larger values demand more of the XY display/oscilloscope 
	//				hardware but do allow more points to be plotted before refresh
	//				flicker is a apparent.
	//
	//	Returns:	Nothing.
	//
	//	20170405 Ver 0.0	E.Andrews	First cut
	//

	if (New_XfrRateHz > 0)	//DO NOTHING if XfrRateHz is zero or less.
			{
		uint32_t tcTicks = FreqToTimerTicks(New_XfrRateHz);	//Convert "Hz value" to "Clock Ticks"
		// Send TC0 Clock signal to external pin so AGS hardware can se it to sync things up
		int ulPin = 2; // just an example:   it's 2  for the Timer0 TIOAO
		PIO_Configure(g_APinDescription[ulPin].pPort,
				g_APinDescription[ulPin].ulPinType,
				g_APinDescription[ulPin].ulPin,
				g_APinDescription[ulPin].ulPinConfiguration);

		// Enable TC0  
		pmc_enable_periph_clk (TC_INTERFACE_ID);

		//Set Mode and Frequency of TC0
		Tc * tc = TC0;
		TcChannel * t = &tc->TC_CHANNEL[0];
		t->TC_CCR = TC_CCR_CLKDIS;
		t->TC_IDR = 0xFFFFFFFF;
		t->TC_SR;
		t->TC_RC = tcTicks;    		// Sets PERIOD of timer in "tcTicks" units
		t->TC_RA = (tcTicks - tcTicks / 2);	// Use this value to alter clock signal symmetry...(tcTicks/2 = 50%)

		//TC_CMR = Timer Counter Channel Mode Register, a four byte register used to set WAVEFORM MODE
		//	Note: Since the DUE MCK = 84 MHz, the following master clock source settings are available
		// 		TC-CMR_TCCLOCKS_TIMER_CLOCK1 sets the clock source to be =  MCK/2  (48.0   MHz)
		// 		TC-CMR_TCCLOCKS_TIMER_CLOCK2 sets the clock source to be =  MCK/8  (10.5   MHz)
		// 		TC-CMR_TCCLOCKS_TIMER_CLOCK3 sets the clock source to be =  MCK/32 ( 2.652 MHz)
		// 		TC-CMR_TCCLOCKS_TIMER_CLOCK3 sets the clock source to be =  MCK/128( 0.65625 MHz or 656.25 KHz)
		t->TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_WAVE
				| TC_CMR_WAVSEL_UP_RC;
		t->TC_CMR = (t->TC_CMR & 0xFFF0FFFF) | TC_CMR_ACPA_CLEAR
				| TC_CMR_ACPC_SET;
		t->TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;

		//Setup is done; Remember the setting for later recall as needed
		DmaClkFreq_Hz = New_XfrRateHz;
	}
}

void XYscope::dacSetup(void) {
	//	Routine to setup DAC and Interrupt Controller for DMA transformers.
	//
	//	User calls this once during SETUP.
	//
	//	Calling parameters:
	//	
	//	None
	//
	//	Returns:	Nothing.
	//		ix		Returns updated pointer into array where next graphics point will go.
	//
	//	20170407 Ver 0.0	E.Andrews	First cut
	//

	//----------------------------------------------------    
	//  DAC SETUP
	//----------------------------------------------------
	pmc_enable_periph_clk (DACC_INTERFACE_ID); 	// start clocking DAC

	dacc_reset (DACC);		//This returns DAC hardware to power-up conditions

	dacc_set_transfer_mode(DACC, 0);//for this variable, 0=HALFWORD_MODE (16 bits integers) or 1=FULLWORD_MODE (32 bit integers)
	dacc_set_power_save(DACC, 0, 0);	//Setup of DAC Power_Save option
	// Set DACC Analog Current Register - Use typical recommended values of 0x01 for IBCTLDACCORE and 0x02 for IBCTLCH0/IBCTLCH1
	// This may sets the slew rate of DACC register & IBCTLDACCH0/IBCTLDACCH1:
	dacc_set_analog_control(DACC,
			DACC_ACR_IBCTLCH0(0x02) | DACC_ACR_IBCTLCH1(0x02)
					| DACC_ACR_IBCTLDACCORE(0x01));
	dacc_set_trigger(DACC, 1);

	dacc_enable_channel(DACC, 0);	//Enable DAC0
	dacc_enable_channel(DACC, 1);	//Enable DAC1

	// DACC_MR bit definitions
	//
	//	|	31	|	30	|	29	|	28	|	27	|	26	|	25	|	24	|
	//	+-------+-------+-------+-------+-------+-------+-------+-------+	STARTUP TIME: Time till initial startup conversion 
	//	|		|		|<-------------- STARTUP TIME ----------------->|	0=0 periods of DACC CLOCK, 63=4032 periods of DACC CLOCK 
	//	+-------+-------+-------+-------+-------+-------+-------+-------+	Typical value = 3 to 8. (See table on pg 1366 of HW Data Sheet for more details)
	//
	//	|	23	|	22	|	21	|	20	|	19	|	18	|	17	|	16	|
	//	+-------+-------+-------+-------+-------+-------+-------+-------+	MAXS: Startup after wake up; 0=Normal (No Sleep), 1=Fast Wake up Sleep Mode
	//	|		|		| MAXS	|  TAG	|		|		|<--USER  SEL-->|	TAG: 0=DIS (Tag Selection Mode Disabled, using USER_SEL value to chan conversion
	//	+-------+-------+-------+-------+-------+-------+-------+-------+	USER_SEL: 0=Chan 0 Selected, 1=Chan 1 Selected (Only meaningful when TAG=1)
	//
	//	|	15	|	14	|	13	|	12	|	11	|	10	|	 9	|	 8	|
	//	+-------+-------+-------+-------+-------+-------+-------+-------+	DAC AUTO REFRESH PERIOD
	//	|<----------------------   REFRESH   -------------------------->| 	0: No DAC Refresh, >0: DAC Refresh Period = 1024 * REFRESH_VALUE/DACC_CLOCK
	//	+-------+-------+-------+-------+-------+-------+-------+-------+
	//
	//	|	7	|	6	|	5	|	4	|	3	|	2	|	 1	|	 0	|
	//	+-------+-------+-------+-------+-------+-------+-------+-------+	FAST WKUP: 0=Sleep Mode defined by SLEEP bit, 1=Fast Wak up Sleep Mode (Vref ON Between Conversions but DAC core is OFF)
	//	|		|FSTWKUP| SLEEP	|WRD MOD|<----- TRIG SEL ------>| TRGEN	|	SLEEP: 0=Normal (No Sleep), 1=Sleep Mode (Sleeps between conversions)
	//	+-------+-------+-------+-------+-------+-------+-------+-------+	WORD/HALF-WORD MODE: 0=Half Word Mode (16 Bit), 1=Full Word Mode (32 Bit)
	//																		TRIG SEL: 0=Ext, 1=TC0_CH0, 2=TC0_CH1, 3=TC0_CH2,4=PWM Event_0,5=PWM Event_1,6=RESRVD,7=RESERVD
	//																		TRGEN (Trig Enable): 0=Disabled, DACC in Free Run Mode, 1=Ext Trig mode.
	//
	//

	//	DACC_MR REGISTER SETTINGS...
	//		STARTUP TIME
	DACC->DACC_MR |= 8 << 24;		//8=512 DACC clock periods of startup time.
	//		MAXS Mode bit
	DACC->DACC_MR |= 0 << 16;		//No Bits Set, MAXS=0 meaning NO SLEEP
	//		TAG MODE Bit
	DACC->DACC_MR |= 1 << 20;//TAG=1 (Bits 12,13 of incomming data selects DAC0/DAC1 destination )
	//		DAC REFRESH 
	DACC->DACC_MR |= 0 << 8;//No Bits Set.  REFRESH=0, meaning NO AUTO REFRESH DESIRED
	//		FSTWKUP, SLEEP,WRD MODE, TRIG SEL, TRig ENABLE
	DACC->DACC_MR |= 0;	//No Bits Set.  HALF_WORD MODE, EXT-TRIG, TRIG DISABLED

	//----------------------------------------------------    
	//  Interrupt Controller SETUP
	//----------------------------------------------------

	NVIC_DisableIRQ (DACC_IRQn);//Disable DACC IRQ in the Nested Vectored Interrupt Controller (NVIC)
	NVIC_ClearPendingIRQ(DACC_IRQn);//Clear any pending DACC IRQ in the Nested Vectored Interrupt Controller (NVIC)
	NVIC_EnableIRQ(DACC_IRQn);//Now, ENABLE DACC IRQ in the Nested Vectored Interrupt Controller (NVIC)

}

void XYscope::dacHandler(void) {
	//	Routine to handls END OF TRANSFER dma interrupt.  
	//	This ISR is called when the DMA transfer to the CRT screen is finished.
	//	The Primary task of the routine is to set crtBlaningPin output pin to HIGH
	//	so that the screen is turned off.
	//
	//	User does NOT directly call this routine.
	//
	//	Calling parameters:
	//				None.
	//
	//	Returns:	Nothing.
	//
	//
	//	20170405 Ver 0.0	E.Andrews	First cut.
	//	20170405 Ver 0.1	E.Andrews	Reworked to play nicely with timer driven refresh inteerupt
	//	20170526 Ver 0.2	E.Andrews	Cleaned up comments and throw out unused code fragments
	//

	//Retrive DACC interupt status
	uint32_t status = dacc_get_interrupt_status(DACC);

	if ((status & DACC_ISR_ENDTX) == DACC_ISR_ENDTX) {//Verify we have a true ENDTX interrupt event.
		//digitalWrite(crtBlankingPin,HIGH);	//turnoff crt beam

		//digitalWrite(crtBlankingPin,LOW);	//Keep Beam On a little while longer...
		//digitalWrite(crtBlankingPin,LOW);	//Keep Beam On a little while longer...
		//digitalWrite(crtBlankingPin,LOW);	//Keep Beam On a little while longer...
		//digitalWrite(crtBlankingPin,HIGH);	//turnoff crt beam
		///digitalWrite(crtBlankingPin,LOW);	//turnoff crt beam}
		///digitalWrite(crtBlankingPin,HIGH);	//turnoff crt beam}
		///digitalWrite(crtBlankingPin,LOW);	//turnoff crt beam}
		///digitalWrite(crtBlankingPin,LOW);	//turnoff crt beam}

		dacc_disable_interrupt(DACC, DACC_IER_ENDTX);//disable interrupt.  ENDTX = Interrupt at End_of_Transmit_Buffer event														//Skip this code and do nothing if not a ENDTX event
		//ENDTX = End of Transmit Buffer.  ENDTX is set when DACC_TCR = 0.
		//This statis check is insurance to be sure it is safe to start updating DMA registers
		//This interlocks with ENDTX status bit so that we only change and update the DMA registers inbetween active DMA transfers.
		// //digitalWrite(crtBlankingPin,HIGH);	//Reset External Sync Signal
		//Update CURRENT REGISTER SET to get it ready for next DMA Operation
		// //DACC->DACC_TPR = (uint32_t)XYlist;		//(DACC_TPR) = Transmit (source data) Pointer Register
		//Set this register to the starting address of the XYList[] array)
		// //DACC->DACC_TCR = XYlistEnd;				//(DACC_TCR)= Transmit Counter Register. Loading this value initiates the start of a transfer
		//The TCR should be set to number of integers we want to happen for each DMA transfer.
		//In this case, XYlistEnd is always pointing at the last element of array XYlist[].	
		//So...Set this TCR register = XYlistEnd counter value

		//NVIC_DisableIRQ(DACC_IRQn);			//Disable DACC IRQ in the Nested Vectored Interrupt Controller (NVIC)
		//NVIC_ClearPendingIRQ(DACC_IRQn);	//Clear any pending DACC IRQ in the Nested Vectored Interrupt Controller (NVIC)
		//NVIC_EnableIRQ(DACC_IRQn);			//Now, ENABLE DACC IRQ in the Nested Vectored Interrupt Controller (NVIC)
		//dacc_enable_interrupt(DACC, DACC_IER_ENDTX);	//Enable DAC interrupt.
		//dacc_enable_interrupt(DACC, DACC_IER_TXBUFE);	//Enable DAC interrupt. DACC_IER_TXBUFE = Interrupt at End_of_Transmit_Buffer_EMPTY event

	}

	for (int i = backPorchBlankCount; i > 0; i--) {
		__asm__ __volatile__("nop");
		//loop to generate a short (~60ns per count), programmable delays
	}
	digitalWrite(crtBlankingPin, HIGH);	//BLANK display...Last point has been plotted...
}

//void initiateDacDma(short& ArrayPtr,int NumOfPoints){

void XYscope::initiateDacDma(void) {
	//
	//	This routine will initiate a DMA transfer of the current XYlist of points to DAC0 (X) & DAC1 (Y)
	//	causing the points to "paint" onto the CRT screen.  It is called from the DacHandler ISR routine.
	//
	//	Timer3 is used to trigger this routine every 'refreshPeriodMs'.  Usually refreshPeriodMs is a constant
	//	(defined in XYscope.h) that is normally set to a value in the 16 to 32 range of values.  The XY points
	//	will be repainted and with the help of POV (persistance of vision) appear continuously to the observer.
	//	If refreshPeriodms is set too long, the screen image will flicker or be noticably blinking.
	//
	//	Calling parameters: NONE
	//
	//	Returns:	NOTHING
	//
	//	20170418 Ver 0.0	E.Andrews	First cut.
	//	20170724 Ver 0.1	E.Andrews	Reworked to automatically adjust FRONT PORCH blanking signal
	//									based on active DMA clock rate.

	DACC->DACC_TPR = (uint32_t) XY_List;//(DACC_TPR) = Transmit (source data) Pointer Register
	//DACC->DACC_TNPR = (uint32_t)XY_List;	//(DACC_TNPR) Transmit NEXT Pointer register. <--May not be needed...
	//Set this register to the starting address of the XY_List[] array)

	DACC->DACC_TCR = XYlistEnd * 2;	//(DACC_TCR)= Transmit Count Register (TCR=How many Short-Integers to transfer).
									//Set TCR register = XYlistEnd counter value
	DACC->DACC_TNCR = 0;//(DACC_TNCR)= Transmit NEXT Counter Register is NOT USED; set to zero!. <--May not be needed...
	//DACC->DACC_TNCR = XYlistEnd*2;		//(DACC_TNCR)= Transmit NEXT Counter Register. <--May not be needed...
	//The TCR should be set to number of integers we want to happen for each DMA transfer.
	//In this case, XYlistEnd is always pointing at the last element of array XY_List[].	

	//This is how we START a transfer! (This will acttually start the DMA transfer
	DACC->DACC_PTCR = DACC_PTCR_TXTEN;//(DACC_PTCR) = Receiver Transfer Enable Register  

	//Now that DMA transfer is underway, we must UNBLANK the Z-Axis by setting the Blanking Pin LOW.

	//The following repeated digitalWrites are used as fixed delays to properly time the
	//state change of the crtBlanking signal which is used by the hardware interface &
	//to properly unblank the output once actual data is starting to appear at DAC1/DAC2

	digitalWrite(crtBlankingPin, HIGH);	//Stay blanked...

	for (int i = frontPorchBlankCount; i > 0; i--) {
		__asm__ __volatile__("nop");
		//loop to generate a short (~60ns per count), programmable delays
	}

	if (millis() > _crtOffTOD_ms && _screenOnTime_ms != 0) {
		digitalWrite(crtBlankingPin, HIGH);	//Keep BLANKED if ScreenSave Time Not yet Exceeded
	} else
		digitalWrite(crtBlankingPin, LOW);	//Unblank and resume display

	dacc_enable_interrupt(DACC, DACC_IER_ENDTX); //Enable interrupt when dac runs out of data...
}

void paintCrt_isrsss() {	//TODO REMOVE THIS ROUTINE!
	//	paintCrtISR Interrupt Service Routine
	//
	//	This routine will initiate a DMA transfer of the current XYlist of points to DAC0 (X) & DAC1 (Y)
	//	causing the points to "paint" onto the CRT screen.
	//
	//	Timer3 is used to trigger this routine every 'refreshPeriodMs'.  Usually refreshPeriodMs is a constant
	//	(defined in XYscope.h) that is normally set to a value in the 16 to 32 range of values.  The XY points
	//	will be repainted and with the help of POV (persistance of vision) appear continuously to the observer.
	//	If refreshPeriodms is set too long, the screen image will flicker or be noticably blinking.
	//
	//	Calling parameters:
	//				None.
	//
	//	Returns:	Nothing.
	//
	//
	//	20170418 Ver 0.0	E.Andrews	First cut.
	//

	//digitalWrite(crtBlankingPin,HIGH);	//Go into blanking...

	// *** END TEST SIGNAL **********	TODO You can DELETE this block when scope testing is no longer needed
	const uint8_t Led13 = 13;//	*	Attach scope probe to pin 13 to monitor entry and exit into this routine
	digitalWrite(Led13, HIGH);	//	*   Entering ISR, Set LED13=HIGH
	// *** END TEST SIGNAL **********

	//initiateDacDma();	//Start the DMA transfer to paint the CRT screen

	// *** END TEST SIGNAL **********	TODO You can DELETE this block when scope testing is no longer needed
	digitalWrite(Led13, LOW);	//	*	Leaving ISR, Set LED13=LOW
	// *** END TEST SIGNAL **********

}

void XYscope::disableDac(void) {
	dacc_disable_interrupt(DACC, DACC_IER_ENDTX);
}

void XYscope::setScreenSaveSecs(long ScreenOnTime_sec) {
	//	Routine to set/change the screen-save period.
	//
	//	Calling parameters:
	//
	//	ScreenOnTime_sec	The time that the display will remain active after the last plotPoint call.
	//						This screen timer begins after the LAST plotPoint command is executed;
	//						Display will blank when timer expires.
	//
	//						ScreenOnTime_Sec = 0 means that the screen saver function is OFF.
	//
	//	Returns:	Nothing.
	//
	//	20170705 Ver 0.0	E.Andrews	First cut
	//
	_screenOnTime_ms = ScreenOnTime_sec * 1000;
	if (_screenOnTime_ms != 0)
		_crtOffTOD_ms = millis() + _screenOnTime_ms;
}
long XYscope::getScreenSaveSecs(void) {
	//	Routine to get the screen-save period.
	//
	//	Calling parameters:
	//
	//	ScreenOnTime_sec	The time that the display will remain active when there is no plotPoint activity.
	//						This screen timer begins after the LAST plotPoint command is executed.
	//						ScreenOnTime_Sec = 0 means that the screen saver function is OFF
	//						and the screen continuously on.
	//
	//	Returns:	ScreenOnTime_sec
	//
	//	20170705 Ver 0.0	E.Andrews	First cut
	//
	return _screenOnTime_ms / 1000;
}

void XYscope::setRefreshPeriodUs(uint32_t refresh_us) {
	//	Routine to set/change the refresh timer.  User can call any time to change the refresh period.
	//
	//	Calling parameters:
	//
	//	refresh_us	Refresh period in microseconds (us). Value is nominally 15000 < refresh_us < 33000.
	//
	//	CAUTIONS:
	//	1) 	For short persistance phosphors, refresh_us may need to <25000 (25 ms)
	//	   	to prevent display flicker.
	//	2)	If refresh time is set too short, the whole display list may not be visualized.
	//
	//	Returns:	Nothing.
	//
	//	20170705 Ver 0.0	E.Andrews	First cut
	//
	ActiveRefreshPeriod_us = refresh_us;//Store the value as the ActiveRefreshPeriod
	Timer3.start(refresh_us);			//Set the refresh timer
}
long XYscope::getRefreshPeriodUs(void) {
	//	Routine to Get the refresh timer.  User can call any time to change the refresh period.
	//
	//	Calling parameters:
	//
	//	refresh_us	Refresh period in microseconds (us). Value is nominally 15000 < refresh_us < 33000.
	//
	//	CAUTIONS:
	//	1) 	For short persistance phoshors, refresh_us may need to <25000 (25 ms)
	//	   	to prevent display flicker.
	//	2)	If refresh time is set too short, the whole display list may not be visualized.
	//
	//	Returns:	Currently active refresh period (in microseconds)
	//
	//	20170705 Ver 0.0	E.Andrews	First cut
	//

	return ActiveRefreshPeriod_us;
}

void XYscope::autoSetRefreshTime(void) {
	//	This routine automatically adjusts the crt refresh period.  Time will be
	//	set based to CrtMinRefresh_ms OR TimeReqdToPlotAllPoints_us, whichever
	//  is larger. User should call this function.
	//
	//	Call this routine AFTER every major write the XYlist to be sure all points are
	//	visualized.
	//
	//	Calling parameters: NONE
	//
	//	Returns:	NOTHING.
	//  Other Notes:  Use GetRefreshPeriodUs() routine to retrieve the active value.
	//
	//	201707017 Ver 0.0	E.Andrews	First cut
	//

	uint32_t crtRefreshTime_us, TimeReqdToPlotAllPoints_us;

	// Calculate the display time needed based on number of points in XYlist
	// Add a "20 Point Buffer" into the calculation to ensure float to integer round off errors
	// do not result in over-run conditions.
	TimeReqdToPlotAllPoints_us = int(DmaClkPeriod_us * 2 * (XYlistEnd + 20));

	//  Compare calculated TimeReqd.. to MinRefresh value as as spec'd in header file.
	//  Pick which ever time is slowest....
	if (TimeReqdToPlotAllPoints_us > CrtMinRefresh_ms * 1000) {
		crtRefreshTime_us = TimeReqdToPlotAllPoints_us;	//Set Refresh time = to miniumn defined refreshtime or that required to get all the points to the screen...
	} else {
		crtRefreshTime_us = CrtMinRefresh_ms * 1000;//Min Refresh Time in microseconds...
	}

	//	Now see if we need to actually change the refresh timer...
	//  Only Update REFRESH timer period (microseconds) when new time <> currently active timer
	if (ActiveRefreshPeriod_us != crtRefreshTime_us) {
		Timer3.start(crtRefreshTime_us);			//Upadate Timer Period
		ActiveRefreshPeriod_us = crtRefreshTime_us;	//Store new value "ActiveRefreshPeriod_us"
	}

}

/****************************************************************************/
/* Private Functions */
/****************************************************************************/

uint32_t XYscope::FreqToTimerTicks(uint32_t freqHz) {
	return VARIANT_MCK / 2UL / freqHz;//Converts frequency(Hz) into Timer Count values for TC programming
}

void XYscope::begin(uint32_t dmaFreqHz) {
	//	Routine to initialize DAC, CounterTimer, & DMA Controller.
	//
	//	User Calls ONCE inside application setup() procedure.
	//
	//	Calling parameters:
	//	
	//	dmaFreqHZ	This is the intended DMA Transfer Rate in Hertz.
	//				If value is omitted, startup frequency will set to XYscope.h default
	//				Note that Point_Clk_Freq = XfrRateHz/2.
	//
	//	CAUTIONS:	Usual XfrRateHz values should be in 300,000 to 800,000 range.
	//
	//				Lower values make the screen refresh take longer and therefore
	//				reduces the total number of points that can be displayed before
	//				CRT flicker becomes visually apparant.
	//
	//				Higher values demand more of the XY display/oscilloscope 
	//				hardware but do allow more points to be plotted before refresh
	//				flicker is a apparent.
	//
	//	Returns:	Nothing.
	//
	//	20170405 Ver 0.0	E.Andrews	First cut
	//	20170828 Ver 1.0	E.Andrews	Remove diag "print" statements and code/comment clean up
	//

	// Initialize the pin mode for BLANKING PIN and set default start up value=HIGH
	pinMode(crtBlankingPin, OUTPUT);
	digitalWrite(crtBlankingPin, HIGH);	//At the start, set CRT output to BLANKED

	//Reset global PlotErr flag.  This flag is set inside the POINT PLOT routine
	//whenever we run out of room in the XY_List array (ie: XYlistEnd > MaxBuffSize).
	plotErr = 0;

	//Initialize ScreenSaver time to default minute
	setScreenSaveSecs(screenOnTimeDefaultSec);

	// Initialize global GRAPHICS and TEXT intensity variables (NO PASSED VALUE=Set to 100%)
	setGraphicsIntensity();	//Set default Graphics intensity
	setTextIntensity();		//Set default Text intensity;

	// Initalize to normal Mono Spaced Characters
	setFontSpacing(mono);

	//Perform DMA Timer Counter setup
	setDmaClockRate(dmaFreqHz);

	tcSetup(dmaFreqHz);	//DO NOT DELETE - This 'redundant' call is needed to get the TC started!

	// Perform Refresh Timer (Timer #3) Setup
	Timer3.start(CrtMinRefresh_ms * 1000);

	// Perform DAC setup
	dacSetup();

	//Initialize XY_list array
	plotStart();

	//Set refresh timer
	autoSetRefreshTime();

	//Serial.print ("\n  -> BEGIN: Graphics/Text Intensity Default Set: ");Serial.print (GetGraphicsIntensity());Serial.print ("% / ");Serial.print (GetTextIntensity());;Serial.print ("%");

	//Serial.print ("\n  -> BEGIN: Calling dacSetup()");

	//Serial.print("...Returned");

	//Serial.print("\n  -> BEGIN: Calling tcSetup (");Serial.print(dmaFreqHz);Serial.print(")");

}

