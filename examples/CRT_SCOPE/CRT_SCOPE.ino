
#define CompileDate __DATE__
#define CompileTime __TIME__
char CompileDateStamp[] = CompileDate;//This is how we get a compile date and time stamp into the program
char CompileTimeStamp[] = CompileTime;

char startMsg[] = "CRT_SCOPE_Ver__20.56 ";	//Program Revision Text

/*
 CRT_SCOPE_20.xx_DMA	(c) E.Andrews  Brookfield, WI USA

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 20170811 Ver 17.00	E.Andrews	First cut
 20170826 Ver 20.50	E.Andrews	General Clean up

 This mainline requires the use of XYscope library, the set of drivers
 that manage and drive the Arduino Graphics Interface (AGI) hardware.

 XYscope libraries were created by E. Andrews (March-September 2017) to enable
 an Ardino DUE to drive a X-Y CRT such as an Oscilloscope or XYZ Monitor.


 */

#include <Arduino.h>	//Provided as part of the Arduino IDE
#include <DueTimer.h>	//Timer library for DUE; get this library at Arduino.org site
#include <XYscope.h>	//Drivers & Graphics Functions for Arduino Graphics Engine (AGI)
//Download this library from GitHub

XYscope XYscope;

// 	+---------Begin Critical Interrupt Service Routines ------------+
//	|  These routines must be declared as shown at the top of the	|
//	|      user's main line code for all XYscope Projects!			|
// 	+----------Begin Critical Interrupt Service Routines -----------+
//	|																|
//	V																V
void DACC_Handler(void) {
	//	DACC_Handler Interrupt Service Routine. This routine
	//	provides a 'wrapper' to link the AVR DAC INTERRUPT
	//	to the 'XYscope-class' ISR routine.
	XYscope.dacHandler();	//Link the AVR DAC ISR/IRQ to the XYscope.
							//It is called whenever the DMA controller
							//'DAC_ready_for_More_data' event occurs
}

void paintCrt_ISR(void) {
	//	paintCrtISR  Interrupt Service Routine. This routine
	//	provides a 'wrapper' to link the Timer3.AttachedInterrupt()
	//	function to the 'XYscope-class' ISR routine.
	XYscope.initiateDacDma();	//Start the DMA transfer to paint the CRT screen
}
//	^																^
//	|																|
// 	+---------- END Critical Interrupt Service Routines ------------+

//	Define/initialize critical global contants and variables

double dacClkRateHz, dacClkRateKHz;

int Array_Ptr;	//Number of steps in graphics
int Density;
int EndOfSetup_Ptr;
double TimeForRefresh;

const int ScopeTrigPin = 3;

char shiftVal = 0;
uint32_t dispTimer = 0;
int MovingX = 2048, MovingY = 2048;
int enabSecondHand;	//Flag to turn "Radar Scope"/"clock Second-hand" demo feature on/off
//0=Disable second-hand animation, >0=ENABLE second-hand animation

int lastXYlistEnd = 0;
//TODO REMOVE -vvv
//So, exactly how are standard Arduino timer functions altered by AGI taking over Timer 0?  Let's see...
long TimerTestNext;			//Variable to test actual accuracy of ms timer
long TimerTestDurationMs;	//Variable to test actual accuracy of ms timer
const int Led13 = 13;		//We will toggle LED (D13) for this test
//TODO REMOVE -^^^

boolean Led13LastState = false;
float angle = 0;

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

extern char _end;
extern "C" char *sbrk(int i);
char *ramstart = (char *) 0x20070000;
char *ramend = (char *) 0x20088000;


void setup() {
	Serial.begin(115200);
	Serial.println("");
	Serial.print(startMsg);
	Serial.print(" (");
	Serial.print(CompileDateStamp);
	Serial.print(" ");
	Serial.print(CompileTimeStamp);
	Serial.println(")");

	//AGI_Startup();
	XYscope.begin();

	//XYscope.setDmaClockRate(dacClkRateHz);		//Set DmaClock rate value...

	//Timer3 is used as the CRT refresh timer.  This timer is setup by XYscope.begin( ).
	//However, paintCRT_ISR must be "attached" to timer 3.  To be properly linked to the
	//AGI interupt service routine, it must be linked in the Arduiono setup() code as follows.
	Timer3.attachInterrupt(paintCrt_ISR);

	//Here is just some stuff to paint onto CRT at startup
	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	ArduinoSplash();
	int xC = 1800, yC = 2800;
	int textSize = 400;
	bool const UndrLined = true;
	int textBright = 150;
	XYscope.printSetup(xC - 150, yC + 50 + 700, textSize, textBright);
	XYscope.print("AGI", UndrLined);
	xC = 100;
	yC = 2900;
	textSize = 250;
	XYscope.printSetup(xC + 50, yC + 50 + textSize, textSize, textBright);
	XYscope.setFontSpacing(XYscope.prop);
	if (XYscope.getFontSpacing() != XYscope.mono)
		XYscope.printSetup(xC + 50 + 500, yC + 50 + textSize, textSize,
				textBright);
	XYscope.print("Arduino Graphics Interface", false);
	XYscope.setFontSpacing(XYscope.mono);

	XYscope.autoSetRefreshTime();
	XYscope.plotRectangle(0, 0, 4095, 4095);

	XYscope.printSetup(1100, 275, 175, 100);
	XYscope.print(startMsg);

	//Now printout  compile date & time
	XYscope.printSetup(1100, 100, 150, 100);
	XYscope.print("(");
	XYscope.print(CompileDateStamp);
	XYscope.print("  ");
	XYscope.print(CompileTimeStamp);
	XYscope.print(")");

	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	//XYscope.SetplotDensity(Density);

	Array_Ptr = 1000;	//TODO Remove?

	Serial.println();
	Print_CRT_Scope_Menu();
	PrintStatsToConsole();
}

void loop() {

	//Provide Serial Control panel...
	int ix;
	int x0, y0, x1, y1;
	int xCtr;
	int yCtr;
	int radius;
	int xRadius;
	int yRadius;

	if (Serial.available()) {
		//XYscope.feedDac(2,Array_Ptr);	//Dac_channel, Array_Ptr
		char d = Serial.read();
		Serial.println();
		Serial.println(d);
		switch (d) {
		case 'h': {	//Print main HELP menu to console
			Print_CRT_Scope_Menu();	//Show Help Menu
			break;
		}

		case '?': {	//Print current Stats to console
			Print_CRT_Scope_Menu();	//Show Help Menu
			PrintStatsToConsole();	//Print current STATS to console
			break;

		}

		case '0': {	//Print formatted Numbers WO/Underline
			short ChInt = 100;	//Brightness of text for this test
			float TstNum = random(-160000000, 160000000);
			TstNum = TstNum / 10000;
			Serial.print("Trying to print: ");
			Serial.print(TstNum, 9);
			Serial.println("  ");

			XYscope.plotClear();
			XYscope.plotStart();
			int CharSize = random(200, 325);
			int xPos = 20, yPos = 4095 - CharSize - 50;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(int(TstNum));						//Print Integer

			yPos = yPos - CharSize - 50;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum);			//Print float to default (2) places

			yPos = yPos - CharSize - 50;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 3);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 4);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 5);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 6);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 7);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 8);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 9);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 10);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 11);//Print Floatint point to specified number of places

			XYscope.autoSetRefreshTime();
			PrintStatsToConsole();

			break;
		}
		case '1': {	//Print formatted Numbers W/Underline
			const int ChInt = 100;	//Brightness of text for this test
			float TstNum = random(-160000000, 160000000);
			TstNum = TstNum / 10000;
			Serial.print("Trying to print: ");
			Serial.print(TstNum, 9);
			Serial.println("  ");

			XYscope.plotClear();
			XYscope.plotStart();
			int CharSize = random(200, 325);
			int xPos = 20, yPos = 4095 - CharSize - 50;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(int(TstNum), bool(true));				//Print Integer

			yPos = yPos - CharSize - 50;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum);			//Print float to default (2) places

			yPos = yPos - CharSize - 50;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 3, bool(true));//Print Floating point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 4);//Print Floating point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 5, bool(true));//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 6);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 7, bool(true));//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 8);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 9, bool(true));//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 10);//Print Floatint point to specified number of places

			yPos = yPos - CharSize;
			XYscope.printSetup(xPos, yPos, CharSize, ChInt);
			XYscope.print(TstNum, 11, bool(true));//Print Floatint point to specified number of places

			XYscope.autoSetRefreshTime();
			PrintStatsToConsole();

			break;
		}

		case '2': {	//Draw Circle inside of Box Pattern for CENTERING
			drawCenteringPattern();
			PrintStatsToConsole();
			break;
		}

		case '3': {	//Random graphics figures
			XYscope.plotClear();
			XYscope.plotStart();
			for (int i = 1; i < 5; i++) {

				x0 = int(random(0, 4096));
				y0 = int(random(0, 4096));
				x1 = int(random(0, 4096));
				y1 = int(random(0, 4096));
				//TODO Serial.print(" ==> Rectangle (x0,y0,x1,y1) = (");Serial.print(x0); Serial.print(",");Serial.print(y0); Serial.print(",");Serial.print(x1); Serial.print(",");Serial.print(y1); Serial.print(")");
				//WaitForAnyKey(" ");
				XYscope.plotRectangle(x0, y0, x1, y1);

				xCtr = int(random(0, 4096));
				yCtr = int(random(0, 4096));
				radius = int(random(0, 2048));
				XYscope.plotCircle(xCtr, yCtr, radius);

				xCtr = (random(0, 4096));
				yCtr = (random(0, 4096));
				xRadius = int(random(0, 2048));
				yRadius = int(random(0, 2048));
				XYscope.plotEllipse(xCtr, yCtr, xRadius, yRadius);

			}

			XYscope.autoSetRefreshTime();
			PrintStatsToConsole();
			break;
		}
		case '4': {	//Simplot
			SimGraph_0();

			XYscope.autoSetRefreshTime();
			PrintStatsToConsole();
			break;
		}

		case '5': {	// Random Ellipse Test Plots
			XYscope.plotClear();
			XYscope.plotStart();
			XYscope.plotErr = 1;
			int oldix = ix;
			while (XYscope.plotErr > 0) {
				xCtr = int(random(0, 4096));
				yCtr = int(random(0, 4096));
				xRadius = int(random(0, 2048));
				yRadius = int(random(0, 2048));
				XYscope.plotEllipse(xCtr, yCtr, xRadius, yRadius);
				//TODO XYscope.plotEll(xCtr,yCtr,xRadius,yRadius);
				XYscope.autoSetRefreshTime();
			}
			//todo Serial.print(" ==> Ellipse (x,y,xRadius,yRadius) = (");Serial.print(xCtr); Serial.print(",");Serial.print(yCtr); Serial.print(",");Serial.print(xRadius); Serial.print(",");Serial.print(yRadius);Serial.print(")");
			//Check by plotting a rectangle too!
			//ix=XYscope.plotRectangle(ix,xCtr-xRadius,yCtr-yRadius,xCtr+xRadius,yCtr+yRadius);
			//todo Serial.println(XYscope.XYlistEnd);

			PrintStatsToConsole();
			break;
		}
		case '6': {	//Plot a single fixed size Ellipse
			XYscope.plotClear();
			XYscope.plotStart();
			xCtr = 2048;
			yCtr = 2048;
			xRadius = 500;
			yRadius = 1000;
			//todo Serial.print(" ==> Ellipse (x,y,xRadius,yRadius) = (");Serial.print(xCtr); Serial.print(",");Serial.print(yCtr); Serial.print(",");Serial.print(xRadius); Serial.print(",");Serial.print(yRadius);Serial.println(")");
			//WaitForAnyKey(" ");
			XYscope.plotEllipse(xCtr, yCtr, xRadius, yRadius);
			XYscope.autoSetRefreshTime();
			PrintStatsToConsole();
			break;
		}
		case '7': {		//Scope sine wave pattern for amplifier setup
			XYscope.plotClear();
			XYscope.plotStart();
			XYscope.plotPoint(0, 4095);
			xCtr = 2047;
			yCtr = 2047;
			xRadius = 2047;
			yRadius = 2047;
			//doto Serial.print(" Generating test pattern...");
			//Serial.print(" ==> Ellipse (ix,x,y,xRadius,yRadius) = (");Serial.print(ix); Serial.print(",");Serial.print(xCtr); Serial.print(",");Serial.print(yCtr); Serial.print(",");Serial.print(xRadius); Serial.print(",");Serial.print(yRadius);Serial.println(")");
			//WaitForAnyKey(" ");
			float numTestPoints = XYscope.getGraphicsIntensity();
			float testAngle = 0;
			int patternCount = 0;
			float angleStep = 2 * 3.141526 / numTestPoints;
			testAngle = testAngle - angleStep;
			for (patternCount = 0; patternCount < numTestPoints;
					patternCount++) {
				testAngle = testAngle + angleStep;
				XYscope.plotPoint(int(xCtr + xRadius * sin(testAngle)),
						int(yCtr + yRadius * cos(testAngle)));
			}
			XYscope.plotPoint(4095, 4095);
			//todo Serial.print(" DONE.  Num of Points Plotted: ");
			Serial.print(numTestPoints);
			Serial.print("  XYlistEnd=");
			Serial.println(XYscope.XYlistEnd);
			XYscope.plotEnd();
			//ix=XYscope.plotEllipse(ix,xCtr,yCtr,xRadius,yRadius);
			XYscope.autoSetRefreshTime();
			PrintStatsToConsole();

			break;

		}
		case '8': {	//plotARC test function - Draw series of Circle arc segments from 0 to 7
			int aSeg = 1;
			for (int segment = 0; segment < 8; segment++) {
				XYscope.plotClear();
				XYscope.plotStart();
				xCtr = 2047;
				yCtr = 2047;
				xRadius = 2047;
				yRadius = 2047;

				int tempIntensity = XYscope.getGraphicsIntensity();
				XYscope.setGraphicsIntensity(10);//Reset density to high value so we can draw a dotted ellipse
				XYscope.plotCircle(xCtr, yCtr, xRadius);	//draw dotted circle
				XYscope.setGraphicsIntensity(tempIntensity);//Return density to normal value

				XYscope.plotCircle(xCtr, yCtr, xRadius, aSeg);
				XYscope.plotLine(0, 4095, 4095, 0);		//TopLt to BotRt Diag
				XYscope.plotLine(4095, 4095, 0, 0);		//TopRt to BotLt Diag
				XYscope.plotLine(0, 2047, 4095, 2047);//Center Horizontal Cross Hair
				XYscope.plotLine(2047, 0, 2047, 4095);//Center Vetical Cross Hair
				//ix = XYscope.plotLine(ix,4095,0,4095,4095);		//RIGHT Border
				//ix = XYscope.plotLine(ix,0,0,0,4095);			//LEFT Border
				XYscope.printSetup(50, 2200, 300, 100);
				XYscope.print("Seg:");
				XYscope.print(segment);
				Serial.print(" Generating Arc pattern... Segment: ");
				Serial.print(segment);
				Serial.print(", Code: ");
				Serial.println(aSeg);
				//Serial.print(" ==> Ellipse (ix,x,y,xRadius,yRadius) = (");Serial.print(ix); Serial.print(",");Serial.print(xCtr); Serial.print(",");Serial.print(yCtr); Serial.print(",");Serial.print(xRadius); Serial.print(",");Serial.print(yRadius);Serial.println(")");
				WaitForAnyKey(" DONE ");
				aSeg = aSeg * 2;	//Shift left one position

			}
			PrintStatsToConsole();
			break;

		}
		case '9': {	//plotArcEllipse test function - Draw series of Elilpse arc segments from 0 to 7
			int aSeg = 1;
			for (int segment = 0; segment < 8; segment++) {
				XYscope.plotClear();
				XYscope.plotStart();
				xCtr = 2047;
				yCtr = 2047;
				xRadius = 1200;
				yRadius = 1800;

				int tempIntensity = XYscope.getGraphicsIntensity();
				XYscope.setGraphicsIntensity(10);//Reset intensity low to draw a dotted ellipse
				XYscope.plotEllipse(xCtr, yCtr, xRadius, yRadius, 255);	//draw dotted ellipse
				//TODO XYscope.plotEll(xCtr,yCtr,xRadius,yRadius,255);
				XYscope.setGraphicsIntensity(tempIntensity);//Return density to normal value

				XYscope.plotLine(0, 4095, 4095, 0);		//TopLt to BotRt Diag
				XYscope.plotLine(4095, 4095, 0, 0);		//TopRt to BotLt Diag
				XYscope.plotLine(0, 2047, 4095, 2047);//Center Horizontal Cross Hair
				XYscope.plotLine(2047, 0, 2047, 4095);//Center Vetical Cross Hair
				XYscope.printSetup(50, 2200, 300, 100);
				XYscope.print("Seg:");
				XYscope.print(segment);
				XYscope.plotEllipse(xCtr, yCtr, xRadius, yRadius, aSeg);//Draw just desired segment
				//TODO XYscope.plotEll(xCtr,yCtr,xRadius,yRadius,aSeg);//Draw just desired segment
				Serial.print(" Generating ArcEllipse pattern... Segment: ");
				Serial.print(segment);
				Serial.print(", Code: ");
				Serial.println(aSeg);
				Serial.print(
						" ==> Ellipse (x,y,xRadius,yRadius,arcSegment) = (");
				Serial.print(xCtr);
				Serial.print(",");
				Serial.print(yCtr);
				Serial.print(",");
				Serial.print(xRadius);
				Serial.print(",");
				Serial.print(yRadius);
				Serial.print(",");
				Serial.print(aSeg);
				Serial.println(")");
				WaitForAnyKey(" DONE ");
				aSeg = aSeg * 2;	//Shift left one position
			}
			PrintStatsToConsole();
			break;
		}

		case '-': {	//Decrease Graphics Intensity Value
			if (XYscope.getGraphicsIntensity() - 10 > 0)
				XYscope.setGraphicsIntensity(
						XYscope.getGraphicsIntensity() - 10);
			break;
		}

		case '+': {	//Increase Graphics Intensity Value
			if (XYscope.getGraphicsIntensity() + 10 < 255)
				XYscope.setGraphicsIntensity(
						XYscope.getGraphicsIntensity() + 10);
			break;
		}

		case '*': {

			if (enabSecondHand == 0) {
				enabSecondHand = 1;
				Serial.println("   Seconds Hand ENABLED");
			} else {
				enabSecondHand = 0;
				Serial.println("   Seconds Hand DISABLED");
			}
			break;
		}
		case 'F': {	//Incr Front-Porch Blanking
			XYscope.frontPorchBlankCount++;
			Serial.print("    DMA Clock Freq: ");
			Serial.print(XYscope.DmaClkFreq_Hz / 1000);
			Serial.print(" Khz");
			Serial.print(" (Period: ");
			Serial.print(XYscope.DmaClkPeriod_us);
			Serial.println(" us)");
			Serial.print("   INCR FRONT PORCH: New Blank Count = ");
			Serial.println(XYscope.frontPorchBlankCount);
			break;
		}
		case 'f': {	//Decr Front-Porch Blanking
			XYscope.frontPorchBlankCount--;
			Serial.print("    DMA Clock Freq: ");
			Serial.print(XYscope.DmaClkFreq_Hz / 1000);
			Serial.print(" Khz");
			Serial.print(" (Period: ");
			Serial.print(XYscope.DmaClkPeriod_us);
			Serial.println(" us)");
			Serial.print("   DECR FRONT PORCH: New Blank Count = ");
			Serial.println(XYscope.frontPorchBlankCount);
			break;
		}

		case 'B': {	//Incr Back-Porch Blanking
			XYscope.backPorchBlankCount++;
			Serial.print("    DMA Clock Freq: ");
			Serial.print(XYscope.DmaClkFreq_Hz / 1000);
			Serial.print(" Khz");
			Serial.print(" (Period: ");
			Serial.print(XYscope.DmaClkPeriod_us);
			Serial.println(" us)");
			Serial.print("   INCR BACK PORCH: New Blank Count = ");
			Serial.println(XYscope.backPorchBlankCount);
			break;
		}
		case 'b': {	//Back Front-Porch Blanking
			XYscope.backPorchBlankCount--;
			Serial.print("    DMA Clock Freq: ");
			Serial.print(XYscope.DmaClkFreq_Hz / 1000);
			Serial.print(" Khz");
			Serial.print(" (Period: ");
			Serial.print(XYscope.DmaClkPeriod_us);
			Serial.println(" us)");
			Serial.print("   DECR BACK PORCH: New Blank Count = ");
			Serial.println(XYscope.backPorchBlankCount);
			break;
		}
		case 'c': {	//Decrease DMA Clock Frequency, Limit min value to 500 Khz
			uint32_t New_XfrRateHz = XYscope.DmaClkFreq_Hz - 50000;	//Decr in 50Khz steps
			if (New_XfrRateHz < 500000)
				New_XfrRateHz = 500000;
			XYscope.setDmaClockRate(New_XfrRateHz);
			XYscope.autoSetRefreshTime();
			Serial.print("   DECR DMA CLK: New Freq = ");
			Serial.print(XYscope.DmaClkFreq_Hz);
			Serial.println(" (Hz)");

			break;
		}
		case 'C': {	//Increase DMA Clock Frequency, Limit max value to 1.2 Mhz
			uint32_t New_XfrRateHz = XYscope.DmaClkFreq_Hz + 50000;	//Incr in 50Khz steps
			if (New_XfrRateHz > 120000000)
				New_XfrRateHz = 1800000;
			XYscope.setDmaClockRate(New_XfrRateHz);
			XYscope.autoSetRefreshTime();
			Serial.print("   INCR DMA CLK: New Freq = ");
			Serial.print(XYscope.DmaClkFreq_Hz);
			Serial.println(" (Hz)");

			break;
		}

		case 'd':
			PeakToPeakTest_D();
			PrintStatsToConsole();
			break;//Draw corner to corner dot pattern to Screen for worst case DAC evaluation
		case 'D':
			SimCoordinateSys();
			PrintStatsToConsole();
			break;	//Draw Coordinate System Plot
		case 'H':
			PeakToPeakTest_H();
			PrintStatsToConsole();
			break;	//Draw HORIZONTAL Sq Wave Pattern to Screen
		case 'V':
			PeakToPeakTest_V();
			PrintStatsToConsole();
			break;	//Draw VERTICAL Sq Wave Pattern to Screen

		case 'i': {	//Decrease Text Intensity Value
			if (XYscope.getTextIntensity() - 10 > 0)
				XYscope.setTextIntensity(XYscope.getTextIntensity() - 10);
			break;
		}
		case 'w': {	//Example of How To Wakeup from a screen saver blank event
			long curScreenSaverSetting = XYscope.getScreenSaveSecs();
			XYscope.setScreenSaveSecs(curScreenSaverSetting);//This sequence wakes up screen and resets the time to sleep? counterresets save timer
			break;
		}
		case 'I': {	//Increase Text Intensity Value
			if (XYscope.getTextIntensity() + 10 < 255)
				XYscope.setTextIntensity(XYscope.getTextIntensity() + 10);
			break;
		}
		case 'l': {	//Animation that Plots Arduino LOGO at various sizes
			int Lx = 100, Ly = 2000, Lht = 1000;
			XYscope.plotStart();
			bool const Underlined = true;
			XYscope.printSetup(2047 - int(float(4000 / 16) * 1.5), 1200, 400,
					100);
			XYscope.print("AGI", Underlined);
			short Tx = 150, Ty = 900;
			if (XYscope.getFontSpacing() != XYscope.mono)
				Tx = 775;
			XYscope.printSetup(Tx, Ty, 230, 100);
			XYscope.print("A", Underlined);
			XYscope.print("rduino ");
			XYscope.print("G", Underlined);
			XYscope.print("raphics ");
			XYscope.print("I", Underlined);
			XYscope.print("nterface");
			XYscope.printSetup(1100, 100, 175, 100);
			XYscope.print(startMsg);
			XYscope.plotRectangle(0, 0, 4095, 4095);

			XYscope.printSetup(20, 100, 175, 100);
			int StartOfLogo = XYscope.XYlistEnd;//Grab the current pointer value

			for (Lht = 30; Lht < 2025; Lht = Lht + 50) {
				XYscope.XYlistEnd = StartOfLogo;//Here's how to do ANIMATION at the endof XYlist...
												//Keep the pointer fixed to keep rewriting
												//the same part of the array
				XYscope.plotErr = 1;
				Lx = 2047 - int(float(Lht / 16) * 14);
				Ly = 2000;
				XYscope.plotArduinoLogo(Lx, Ly, Lht);
				delay(40);
				//Serial.print("Logo Ht = ");Serial.println(Lht);

			}

			XYscope.autoSetRefreshTime();
			PrintStatsToConsole();
			break;
		}
		case 'm': {	//Toggle Font Space mode back and forth between Mono and Proportional
			if (XYscope.getFontSpacing() == XYscope.mono)
				XYscope.setFontSpacing(XYscope.prop);
			else
				XYscope.setFontSpacing(XYscope.mono);
			break;
		}

		case 's': {	//Display sample text, Test #1
			XYscope.plotClear();
			XYscope.plotStart();
			XYscope.plotErr = 1;
			int cX = 50, cY = 4050, charHt = 500;
			const int charPerLine = 12;
			uint8_t startChar = ' ' - charPerLine;
			for (int Row = 1; Row < 9; Row++) {
				startChar = startChar + charPerLine;
				cY = 4050 - Row * 500;
				cX = 0;
				for (uint8_t tstChar = startChar;
						tstChar < startChar + charPerLine; tstChar++) {
					XYscope.plotChar(tstChar, cX, cY, charHt);
				}
			}
			XYscope.autoSetRefreshTime();
			PrintStatsToConsole();
			break;
		}
		case 'S': {	//Display sample text, Test #2
			XYscope.plotClear();
			XYscope.plotStart();
			XYscope.plotErr = 1;
			int cX = 50, cY = 4050, charHt = 500;
			const int charPerLine = 12;
			uint8_t startChar = '0' - charPerLine;
			for (int Row = 1; Row < 9; Row++) {
				startChar = startChar + charPerLine;
				cY = 4050 - Row * 500;
				cX = 0;
				for (uint8_t tstChar = startChar;
						tstChar < startChar + charPerLine; tstChar++) {
					XYscope.plotChar(tstChar, cX, cY, charHt);
				}
			}
			XYscope.autoSetRefreshTime();
			PrintStatsToConsole();
			break;
		}
		case 't': {	//Simulated Analog Clock Face
			SimClock_0();
			EndOfSetup_Ptr = XYscope.XYlistEnd;
			XYscope.autoSetRefreshTime();
			PrintStatsToConsole();
			break;
		}
		case 'p':
			PongDemo();
			break;
		default: {
			Serial.println("^-- invalid/unknown command character entered ");
			PrintStatsToConsole();
			drawCenteringPattern();

			break;
		}

		}

	}

	else {
		float delta_Ang = (2 * PI) / (60);

		if ((enabSecondHand == 1) && (millis() > TimeForRefresh)) {

			TimeForRefresh = millis() + 1000;
			angle = (angle + delta_Ang);
			if (angle >= 2 * PI)
				angle = delta_Ang;
			MovingX = 2047 * sin(angle) + 2047;
			if (MovingX > 4095)
				MovingX = 4095;
			if (MovingX < 0)
				MovingX = 0;
			MovingY = 2047 * cos(angle) + 2047;
			if (MovingY > 4095)
				MovingX = 4095;
			if (MovingY < 0)
				MovingX = 0;
			XYscope.setGraphicsIntensity(XYscope.getGraphicsIntensity());
			XYscope.XYlistEnd = EndOfSetup_Ptr - 1;
			XYscope.plotLine(2047, 2047, MovingX, MovingY);
			XYscope.plotEnd();
			XYscope.autoSetRefreshTime();
		}
	}

}

//=======================================================
//	PONG Demo
//=======================================================
void PongDemo() {

	float ballX = 2048, ballY = 2048;//define and initialize ball at center of screenBall Center Point
	int ballR = 75;					//Define Ball Radius
	float ballDX = .5, ballDY = .31;	//Define initial Ball Velocities

	int Lwall = 0, Rwall = 4095;		//Define Left & right Wall	X-Coord

	int Twall = 4095, Bwall = 0;		//Top & Bottom wall Y-Coord

	int ballCount = 5, ballMax = 1;	//Define number of times to draw ball

	// Clear screen and draw court
	XYscope.plotStart();
	XYscope.plotLine(Lwall, Twall, Rwall, Twall);	//Top line
	XYscope.plotLine(Rwall, Twall, Rwall, Bwall);	//Right line
	XYscope.plotLine(Rwall, Bwall, Lwall, Bwall);	//Bottom line
	XYscope.plotLine(Lwall, Bwall, Lwall, Twall);	//Left line
	//Draw "net" as a dashed line down the center of the screen
	const int dashLength = 200;
	for (int dash = dashLength / 2; dash < Twall - dashLength / 2;
			dash = dash + dashLength) {
		XYscope.plotLine(Rwall / 2, dash, Rwall / 2, dash + dashLength / 2);//Center line
	}

	//Draw a simulated score...Note, scores do not change in DEMO code!
	XYscope.printSetup(700, 3000, 800);
	XYscope.print("18");
	XYscope.printSetup(2700, 3000, 800);
	XYscope.print("09");

	//Draw simulated paddles...Paddles remain stationary in DEMO code!
	XYscope.plotRectangle(200, 1200, 300, 1700);	//Left Paddle
	XYscope.plotRectangle(3800, 1600, 3900, 2100);	//Right Paddle

	//Remember the current XYlistEnd value so we can return to it every time we
	//want to move and rewrite the ball
	int ixEmptyCourt = XYscope.XYlistEnd;

	Serial.println("**** press a key to stop PONG DEMO ****");
	int wait = true;
	ballCount = 0;

	while (wait)	//In this loop, we just move the ball around the court!
	{
		if (Serial.available()) {//Check for key press...any key = Leave the DEMO
			char d = Serial.read();	//clean out the buffer
			wait = false;
			Serial.println(F("_KP_"));
			delay(100);
		}
		//Draw Ball
		XYscope.plotCircle(ballX, ballY, ballR);
		ballCount = ballCount + 1;

		//Plot the ball multiple times before moving it to make it brighter
		if (ballCount > ballMax) {
			XYscope.XYlistEnd = ixEmptyCourt;
			ballCount = 0;
		}

		//Move Ball...
		ballX = ballX + ballDX;
		ballY = ballY + ballDY;
		//Define Ball to Wall collision limits based on size of ball
		int TopLimit = Twall - ballR;
		int BotLimit = Bwall + ballR;
		int RightLimit = Rwall - ballR;
		int LeftLimit = Lwall + ballR;

		//---Check for Top-Bottom collisions with walls...
		if (ballY >= TopLimit) {	//Top Wall Impact
			ballY = +TopLimit - (abs(ballDY) - abs(ballY - TopLimit));
			ballDY = -ballDY;
		}
		if (ballY <= BotLimit) {	//Bottom wall Impact
			ballY = BotLimit + (abs(ballDY) - abs(ballY - BotLimit));
			ballDY = -ballDY;
		}
		//Check Left-Right collisions with walls...
		if (ballX >= RightLimit) {	//Right Wall Impact
			ballX = RightLimit - (abs(ballDX) - abs(ballX - RightLimit));
			ballDX = -ballDX;
		}
		if (ballX <= LeftLimit) {	//Left Wall Impact
			ballX = LeftLimit + (abs(ballDX) - abs(ballX - LeftLimit));
			ballDX = -ballDX;
		}
		//---Check for & react to ball collisions with Paddles.....
		//   You Add this Part!!!

	}

}
//=========== END PONG DEMO =============================

//=======================================================
//	SPLASH SCREEN DEMO
//=======================================================
void ArduinoSplash() {
	int Lx = 1150, Ly = 2000, Lht = 1000;
	XYscope.plotStart();
	XYscope.plotArduinoLogo(Lx, Ly, Lht);

}
//=========== END SPLASH =================================

void drawCenteringPattern(void) {
	XYscope.plotStart();
	int X_Center = 2047;
	int Y_Center = 2400;
	int Circ_Radius = 800;
	X_Center = 2048;
	Y_Center = 2048;

	XYscope.plotCircle(X_Center, Y_Center, Circ_Radius);		//try a circle
	Circ_Radius = 2045;
	XYscope.plotCircle(X_Center, Y_Center, Circ_Radius);		//try a circle

	XYscope.plotPoint(X_Center, Y_Center);// Put a DOT at the center of the circ.
	XYscope.plotLine(4095, 4095, 0, 4095);		//TOP Border
	XYscope.plotLine(0, 4095, 4095, 0);		//TopLt to BotRt Diag
	XYscope.plotLine(4095, 0, 0, 0);			//BOTTOM Border
	XYscope.plotLine(0, 0, 4095, 4095);		//TopRt to BotLt Diag

	XYscope.plotLine(4095, 0, 4095, 4095);		//RIGHT Border
	XYscope.plotLine(0, 0, 0, 4095);			//LEFT Border

	EndOfSetup_Ptr = XYscope.XYlistEnd;	//Remember end of buffer ptr in case second hand is enabled

	XYscope.plotEnd();						//end with plot to 0,0
	XYscope.autoSetRefreshTime();
	ShowMemory();
	PrintStatsToConsole();
}

void SimClock_0(void) {
	XYscope.plotClear();
	XYscope.plotStart();
	int xChar, yChar;
	int xCtr = 2047;
	int yCtr = 2047;
	int xRadius = 1900;
	int yRadius = 1900;
	Serial.print(" Generating Clock Face test pattern...");
	//Serial.print(" ==> Ellipse (ix,x,y,xRadius,yRadius) = (");Serial.print(ix); Serial.print(",");Serial.print(xCtr); Serial.print(",");Serial.print(yCtr); Serial.print(",");Serial.print(xRadius); Serial.print(",");Serial.print(yRadius);Serial.println(")");
	//WaitForAnyKey(" ");
	float numTestPoints = 12;
	float testAngle = 0;
	int patternCount = 0;
	float angleStep = 2 * 3.141526 / numTestPoints;
	testAngle = testAngle - angleStep;
	const int charHt = 500;
	int HoursIntensity = 0;
	int OldTextIntensity = XYscope.getTextIntensity();
	XYscope.setGraphicsIntensity(100);
	for (patternCount = 0; patternCount < numTestPoints; patternCount++) {
		HoursIntensity = HoursIntensity + 20;//For test purposes, change intensity of every hour value. Start at 20%
		if (HoursIntensity > 255)
			HoursIntensity = 255;

		testAngle = testAngle + angleStep;
		int xP, yP;
		xP = int(xCtr + xRadius * sin(testAngle));
		yP = int(yCtr + yRadius * cos(testAngle));
		//XYscope.plotPoint(xP,yP);
		//XYscope.plotPoint(xP,yP);
		//XYscope.plotPoint(xP,yP);
		//XYscope.plotPoint(xP,yP);
		XYscope.plotCircle(xP, yP, 40);	//Plot small circles at HOUR marks
		// Now Plot Hour numbers 1-12.  Change in text-plot-radius
		// and offsets were needed to get the numbers in the right place...
		int xDelta = -350;
		int yDelta = -350;
		int xOffset = -charHt / 2;
		int yOffset = -charHt / 2;
		xP = int(xCtr + xOffset + (xRadius + xDelta) * sin(testAngle));
		yP = int(yCtr + yOffset + (yRadius + yDelta) * cos(testAngle));
		//Plot HOURS text
		if (patternCount > 0 && patternCount < 7)
			XYscope.printSetup(xP + 150, yP, charHt, HoursIntensity);
		else
			XYscope.printSetup(xP, yP, charHt, HoursIntensity);
		if (patternCount == 0) {
			XYscope.printSetup(xP, yP, charHt, 250);
			XYscope.print("12");
		} else
			XYscope.print(patternCount);
	}
	//Restore prior Text Intensity
	XYscope.setTextIntensity(OldTextIntensity);
	//Draw some sample hour hands...
	XYscope.plotCircle(xCtr, yCtr, 150);	//Center Circle
	//Hour Hand
	XYscope.plotLine(xCtr, yCtr + 150, xCtr + xRadius - 500, yCtr);
	XYscope.plotLine(xCtr, yCtr - 150, xCtr + xRadius - 500, yCtr);
	//Minute Hand
	XYscope.plotLine(xCtr + 150, yCtr, xCtr, yCtr + yRadius);
	XYscope.plotLine(xCtr - 150, yCtr, xCtr, yCtr + yRadius);

	//Draw short dashed lines for each of the SECOND marks
	numTestPoints = 60;
	angleStep = 2 * 3.141526 / numTestPoints;
	for (patternCount = 0; patternCount < numTestPoints; patternCount++) {
		testAngle = testAngle + angleStep;
		int xP1, yP1, xP2, yP2;
		xP1 = int(xCtr + (xRadius + 50) * sin(testAngle));
		yP1 = int(yCtr + (yRadius + 50) * cos(testAngle));
		xP2 = int(xCtr + (xRadius - 50) * sin(testAngle));
		yP2 = int(yCtr + (yRadius - 50) * cos(testAngle));
		//Skip plotting the SECONDS-MARK at each of the HOUR marks
		if ((patternCount + 1) % 5 != 0)
			XYscope.plotLine(xP1, yP1, xP2, yP2);
	}
	int logoHt = 1000;
	int logoX = 2047 - logoHt * .85;
	int logoY = 2047 - logoHt;

	XYscope.plotArduinoLogo(logoX, logoY, logoHt);

	XYscope.plotPoint(4095, 4095);
	Serial.print(" DONE.\n  Num of Points Plotted: ");
	Serial.print(numTestPoints);
	Serial.print("  XYlistEnd=");
	Serial.println(XYscope.XYlistEnd);
	XYscope.plotEnd();
	//ix=XYscope.plotEllipse(ix,xCtr,yCtr,xRadius,yRadius);
	EndOfSetup_Ptr = XYscope.XYlistEnd - 1;
	XYscope.autoSetRefreshTime();

}

void SimCoordinateSys(void) {
	//	Routine to plot a graphic that shows the AGI (X,Y) Coordinate system
	//
	//	Passed Parameters
	//
	//	Returns: NOTING
	//
	//	20170811 Ver 0.0	E.Andrews	First cut
	//
	int xC, yC;
	const boolean UndrLined = true;

	XYscope.plotClear();
	XYscope.plotStart();
	XYscope.setGraphicsIntensity(10);
	XYscope.plotRectangle(0, 0, 4095, 4095);

	XYscope.setGraphicsIntensity(150);
	int textSize = 250, textBright = 150;

	xC = 0;
	yC = 0;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC + 100, yC + 40, textSize, textBright);
	XYscope.print("(");
	XYscope.print(xC);
	XYscope.print(",");
	XYscope.print(yC);
	XYscope.print(")");

	xC = 0;
	yC = 4095;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC + 100, yC - 50 - textSize, textSize, textBright);
	XYscope.print("(");
	XYscope.print(xC);
	XYscope.print(",");
	XYscope.print(yC);
	XYscope.print(")");

	xC = 4095;
	yC = 4095;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC - 50 - (11 * (textSize * .65)), yC - 50 - textSize,
			textSize, textBright);
	XYscope.print("(");
	XYscope.print(xC);
	XYscope.print(",");
	XYscope.print(yC);
	XYscope.print(")");

	xC = 4095;
	yC = 0;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC - 50 - (8 * (textSize * .65)), yC + 50, textSize,
			textBright);
	XYscope.print("(");
	XYscope.print(xC);
	XYscope.print(",");
	XYscope.print(yC);
	XYscope.print(")");

	xC = 2047;
	yC = 2047;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC + 50, yC - 50 - textSize, textSize, textBright);
	XYscope.print(" (");
	XYscope.print(xC);
	XYscope.print(",");
	XYscope.print(yC);
	XYscope.print(")");

	xC = 1023;
	yC = 1023;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC + 50, yC - 50 - textSize, textSize, textBright);
	XYscope.print(" (");
	XYscope.print(xC);
	XYscope.print(",");
	XYscope.print(yC);
	XYscope.print(")");

	xC = 1650;
	yC = 2047;

	textSize = 400;
	textBright = 150;
	XYscope.printSetup(xC + 50, yC + 50 + 700, textSize, textBright);
	XYscope.print("AGI", UndrLined);
	xC = 100;
	yC = 2047;
	textSize = 350;
	XYscope.printSetup(xC + 50, yC + 50 + textSize, textSize, textBright);
	if (XYscope.getFontSpacing() != XYscope.mono)
		XYscope.printSetup(xC + 50 + 500, yC + 50 + textSize, textSize,
				textBright);
	XYscope.print("Coordinate System", UndrLined);

	//PlotCrossHair(2047,2047);

	XYscope.plotEnd();
	XYscope.autoSetRefreshTime();
	ShowMemory();
	PrintStatsToConsole();
}

void PlotCrossHair(int Xcoord, int Ycoord) {
	const int LineLeng = 100;
	const int LineSpace = 50;
	const int DotRepeatCnt = 10;
	int x0, y0, x1, y1;
	//Xcoord=Xcoord && 4095;	//Make sure coordinates are ON-SCREEN
	//Ycoord=Ycoord && 4095;	//Make sure coordinates are ON-SCREEN

	//Plot the target Point...Make it Bright by plotting multiple times
	for (int i = 0; i < DotRepeatCnt; i++) {
		XYscope.plotPoint(Xcoord, Ycoord);
	}

	//Plot Left Horizontal Cross-hair line
	x0 = Xcoord - LineSpace;
	x1 = Xcoord - (LineSpace + LineLeng);
	if (x1 < 0)
		x1 = 0;
	y0 = Ycoord;
	y1 = Ycoord;
	if (x0 >= 0 && x1 <= 4095)
		XYscope.plotLine(x0, y0, x1, y1);

	//Plot Right Horizontal Cross-hair line
	x0 = Xcoord + LineSpace;
	x1 = Xcoord + (LineSpace + LineLeng);
	if (x1 > 4095)
		x1 = 4095;
	if (x0 >= 0 && x1 <= 4095)
		XYscope.plotLine(x0, y0, x1, y1);

	//Plot Top Vertical Cross-hair line
	x0 = Xcoord;
	x1 = Xcoord;
	if (x1 < 0)
		x1 = 0;
	y0 = Ycoord + LineSpace;
	y1 = Ycoord + (LineSpace + LineLeng);
	if (y1 > 4095)
		y1 = 4095;
	if (y0 >= 0 && y1 <= 4095)
		XYscope.plotLine(x0, y0, x1, y1);

	//Plot Bottom Vertical Cross-hair line
	y0 = Ycoord - LineSpace;
	y1 = Ycoord - (LineSpace + LineLeng);
	if (y1 < 0)
		y1 = 0;
	if (y0 >= 0 && y1 >= 0)
		XYscope.plotLine(x0, y0, x1, y1);

}

void SimGraph_0(void) {

	Serial.print("ERASING Data Table...");
	XYscope.plotClear();
	XYscope.plotStart();
	XYscope.plotRectangle(500, 500, 4000, 3500);

	for (int yLine = 1000; yLine < 3501; yLine = yLine + 500) {
		XYscope.printSetup(20, yLine - 100, 200);
		XYscope.print(yLine / 100);
		XYscope.plotLine(450, yLine, 4000, yLine);
	}

	for (int xLine = 1000; xLine < 4001; xLine = xLine + 500) {
		XYscope.printSetup(xLine - 700, 200, 200);
		XYscope.print(float(xLine) / 1000., 1);
		XYscope.plotLine(xLine, 450, xLine, 3500);
	}
	int yPoint = 2000;
	for (int xPoint = 500; xPoint < 4001; xPoint = xPoint + 2) {
		yPoint = 2000 + 1200 * sin(float(xPoint) / 500.);
		XYscope.plotPoint(xPoint, yPoint);
	}
	XYscope.printSetup(100, 3800, 250);
	XYscope.print("      Flux-Capacitance");
	XYscope.printSetup(100, 3600, 250);
	XYscope.print("       Vs Giga-Watts");
	XYscope.plotEnd();
	XYscope.autoSetRefreshTime();
	ShowMemory();
	PrintStatsToConsole();
}

void PeakToPeakTest_V() {
	XYscope.plotStart();
	//Dashed Line, Top and Bottom
	for (int i = 0; i < 4095; i = i + 256) {
		XYscope.plotLine(i, 4095, i + 127, 4095);//Dashed line at Top of Screen
		XYscope.plotLine(i + 128, 0, i + 255, 0);//Dashed Line at Bottom of Screen
	}
	Serial.println(" DONE PeakToPeak_Vert Sq Wave ");
}
void PeakToPeakTest_H() {
	XYscope.plotStart();
	//Dashed Line, Left & Right
	for (int i = 0; i < 4095; i = i + 256) {
		XYscope.plotLine(0, i, 0, i + 127);	//Dashed line at Left Side of Screen
		XYscope.plotLine(4095, i + 128, 4095, i + 255);	//Dashed Line at Right Side of Screen
	}
	Serial.println(" DONE PeakToPeak_Horiz Sq Wave ");
}

void PeakToPeakTest_D() {
	XYscope.plotStart();
	//Dots in lower left hand corner and upper right hand corner of display screen
	//Used to measure frequency response of the DACs

	XYscope.plotPoint(0, 0);
	XYscope.plotPoint(4095, 4095);
	XYscope.plotPoint(0, 0);
	XYscope.plotPoint(4095, 4095);
	XYscope.plotPoint(0, 0);
	XYscope.plotPoint(4095, 4095);
	XYscope.plotPoint(0, 0);
	XYscope.plotPoint(4095, 4095);
	XYscope.plotPoint(0, 0);
	XYscope.plotEnd();

	Serial.println(" DONE PeakToPeak Corner_to_Corner Test ");
}
void PrintStatsToConsole(void) {
	Serial.print("\n STATS..............\n MaxBuffSize: ");
	Serial.print(XYscope.MaxBuffSize);
	Serial.print("  Total Array Used: ");
	Serial.print((XYscope.XYlistEnd - 1));
	Serial.print(" (");
	Serial.print((XYscope.XYlistEnd * 100) / XYscope.MaxBuffSize);
	Serial.println(" %)");
	Serial.print("    DMA Clock Freq: ");
	Serial.print(XYscope.DmaClkFreq_Hz / 1000);
	Serial.print(" Khz");
	Serial.print(" (Period: ");
	Serial.print(XYscope.DmaClkPeriod_us);
	Serial.println(" us)");
	Serial.print("   Point Clock Freq: ");
	Serial.print(XYscope.DmaClkFreq_Hz / 2000);
	Serial.print(" Khz");
	Serial.print(" (Period: ");
	Serial.print(XYscope.DmaClkPeriod_us * 2);
	Serial.println(" us)");

	Serial.print("        Refresh Freq: ");
	Serial.print(float(1000000) / float(XYscope.ActiveRefreshPeriod_us));
	Serial.print(" Hz  (Period: ");
	Serial.print(XYscope.ActiveRefreshPeriod_us);
	Serial.print(" us)\n");
	Serial.print("Graphics Int: ");
	Serial.print(XYscope.getGraphicsIntensity(), DEC);
	Serial.print(" %   Text Int: ");
	Serial.print(XYscope.getTextIntensity(), DEC);
	Serial.println(" %");
	Serial.print("Font Spacing Mode=");
	if (XYscope.getFontSpacing() == 0)
		Serial.println("PROPORTIONAL");
	else
		Serial.println("MONO");
	Serial.print("Second Hand Enab = ");
	if (enabSecondHand)
		Serial.print("YES,");
	else
		Serial.print("NO, ");
	Serial.print(" Screen_Save_Secs: ");
	Serial.print(XYscope.getScreenSaveSecs());
	Serial.println();
}
void Print_CRT_Scope_Menu() {

	Serial.print("========= ");
	Serial.print(startMsg);
	Serial.println(" =========");
	Serial.println("  h     Show this HELP Screen");
	Serial.println("  ?     Show current plotting STATS");
	Serial.println("---- Hardware setup and diagnostics -----");
	Serial.println("  7     Signal Scope Pattern for Gain & Center Adj.");
	Serial.println("  C/c   Incr(C)/Decr(c) DMA Clock Freq by 50 Khz");
	Serial.println("  F/f   Front-Porch blank count [Incr=F/Decr=f]");
	Serial.println("  B/b   Back-Porch blank count [Incr=B/Decr=b]");
	Serial.println("  2     Display Centering Pattern");
	Serial.println("  V/H   Peak-to-Peak Vertical/Horiz Sq-Wv Test Pattern");
	Serial.println("  d     Peak-to-Peak Dots_in_the_Corners Test Pattern");

	Serial.println("---- Test plots for TEXT routines");
	Serial.println("  m     Toggle Font mode Monospace<->Propotional");
	Serial.println("  I/i   TEXT intensity [Incr=I)/Decr=i]");
	Serial.println("  0/1   0=Print Random Nums, 1=Nums w/Underlines");
	Serial.println("  s/S   plotChar Test, s=Test 1, S=Test 2");
	Serial.println("  w     Wakeup from Screen-save timeout");

	Serial.println("---- Test plots for GRAPHICS routines ----");
	Serial.println("  +/-   GRAPHICS Intensity [Incr=+/Decr=-]");
	Serial.println("  3     Display Random Rectangles, Circles, & Ellipses");
	Serial.println("  4     Simulated Graph Plot");
	Serial.println("  5     Display Random Ellipse");
	Serial.println("  6     Display a fixed Ellipse");
	Serial.println("  8     Demo Circle arc segments from 0 to 7");
	Serial.println("  9     Demo Ellipse arc segments from 0 to 7");

	Serial.println("---- Simulated Application Screens ----");
	Serial.println("  l     Plot an animated Arduino Logo");
	Serial.println("  D     Show AGI Coordinate System Plot");
	Serial.println("  t     TIME - A Simulated Clock Display");
	Serial.println("  *     Toggle 'SimClock' second-hand On-Off");
	Serial.println("  p     Pong Simulation");

	Serial.println("==========================================");

}

//=======================================================
// DIAGNOSTIC ROUTINES
//=======================================================
void WaitForAnyKey(String msg) {
	// Routine outputs 'msg' to screen using Serial.print then halts execution and 
	// waits for Opr to hit any key.  Then it will return so execution can continue.

	boolean wait;
	Serial.print(msg);
	Serial.println(F(" <-WAITING! (...Hit any Key to CONTINUE...)"));
	wait = true;
	while (wait) {
		if (Serial.available()) {
			char d = Serial.read();	//clean out the buffer (Retrieved data is NOT USED)
			wait = false;
			Serial.println(F("_KP_"));
			delay(100);
		}

	}
}
void ShowMemory() {

	char *heapend = sbrk(0);
	register char * stack_ptr asm ("sp");
	struct mallinfo mi = mallinfo();
	printf("\n DUE Dynamic ram used: %d\n", mi.uordblks);
	printf("  DUE Program static ram used %d\n", &_end - ramstart);
	printf("  DUE Stack ram used %d\n\n", ramend - stack_ptr);
	printf("     ... My guess at free mem: %d\n",
			stack_ptr - heapend + mi.fordblks);
}
