
#define CompileDate __DATE__
#define CompileTime __TIME__
char CompileDateStamp[] = CompileDate;//This is how we get a compile date and time stamp into the program
char CompileTimeStamp[] = CompileTime;

char startMsg[] = "CRT_SCOPE (Ver_1.60) ";	//Program Revision Text

/*
 CRT_SCOPE.ino  E.Andrews  Brookfield, WI USA

 This program is an EXAMPLE program to demonstrate and test the
 Arduino Graphics Interface (AGI) hardware.  This hardware provides
 an interface between an Arduino DUE processor and an Oscillocope that
 permits XY drawing and display of graphics objects onto the scope CRT.
 
 Hardwareand software details can be are found in the EXTRA folder.  
 This project is urther described in the February 2018 and March 2018 
 editions of of the electronics magazine "Nuts & Volts"
 
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

 Note: This program was written and tested using Arduino IDE, Version 1.8.2
       Modifications may be required if using other platforms or versions of the IDE.
	   Also, you must select Arduino DUE as the target processor for correct and valid compilation.
 
 20170311 Ver  0.00	E.Andrews	Development begins
 20170826 Ver  1.00	E.Andrews	All key functions working, General Clean up
 20171122 Ver  1.58 E.Andrews	Cleanup and tuning; Reset master revision level to 1.58
 20180103 Ver  1.59 E.Andrews   Added clarifying comments throughout code base
 20180103 Ver  1.60 E.Andrews   Adapt to Lib-structure, enhance comments, & test to ensure
 								 that package can be compiled within ARDUINO-IDE Ver 1.8.2 as a library

 This mainline requires the use of XYscope library, the set of drivers
 that manage and drive the Arduino Graphics Interface (AGI) hardware.

 XYscope libraries were first created by E. Andrews (March-September 2017) to enable
 an Ardino DUE to drive a X-Y CRT such as an Oscilloscope or XYZ Monitor.

 This program requires the following three (3) XYscope library routines to function properly
	XYscope.h			Include file
	XYscope.cpp			All grapahics control and plotting routines
	VectorFontROM.h		FONT file used by XYscope to display variable sized vector-stroke characters.
						All defined characters can be displayed using XYscope option 'S' and 's'.
	
 */

#include <Arduino.h>	//Provided as part of the Arduino IDE

#include <DueTimer.h>	//Timer library for DUE; download this library from the Arduino.org site
						//Timer library is also available from author at https://github.com/ivanseidel/DueTimer
						
//UNCOMMENT ONE OF THE FOLLOWING TWO INCLUDE STATEMENTS TO SET THE PATH TO THESE THE LIBRARY PROGRAMS

#include <XYscope.h>	//UNCOMMENT THIS LINE for PUBLIC XYscope Drivers & Graphics Functions for Arduino Graphics Engine (AGI)
						//Use this INCLUDE if you store XYscope library routines inside of ...\Library\XYscope folder

//#include "XYscope.h"	//UNCOMMENT THIS LINE for PRIVATE XYscope Drivers & Graphics Functions for Arduino Graphics Engine (AGI)
						//Use this INCLUDE if you keep XYscope library routines in same directory as mainline.ino Directory

//Download this library from GitHub

XYscope XYscope;

// 	+---------Begin Critical Interrupt Service Routines ------------+
//	|  These routines MUST be declared as shown at the top of the	|
//	|      user's main line code for all XYscope Projects!			|
// 	+---------------------------------------------------------------+
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

//	V																V
//	|																|
// 	+---------- END Critical Interrupt Service Routines ------------+

//	Define/initialize critical global constants and variables

double dacClkRateHz, dacClkRateKHz;

int EndOfSetup_Ptr;
double TimeForRefresh;


char shiftVal = 0;
uint32_t dispTimer = 0;
int MovingX = 2048, MovingY = 2048;


int enabSecondHand;	//Flag to turn "Radar Scope clock Second-hand" demo feature on/off
					//0=Disable second-hand animation, <>0=ENABLE second-hand animation


float angle = 0;

#include <malloc.h>	//Required for RAM usage monitoring routines

// The following lines are used for RAM monitoring routines
extern char _end;
extern "C" char *sbrk(int i);
char *ramstart = (char *) 0x20070000;
char *ramend = (char *) 0x20088000;


void setup() {
	// Mainline program SETUP routine.
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170708 Ver 0.0	E.Andrews	First cut
	//						(Updated throughout development cycle without version change)

	//Set Serial Monitor must be setup to agree with this initializtion.
	//  [X] Autoscroll
	//  "No Line Ending"
	//  "115200 baud"
	Serial.begin(115200);
	
	//Send startup messages out to Serial monitor port...
	Serial.println("");
	Serial.print(startMsg);
	Serial.print(" (");
	Serial.print(CompileDateStamp);
	Serial.print(" ");
	Serial.print(CompileTimeStamp);
	Serial.println(")");

	double DmaFreq=800000;		//800000 (Hz) is the default startup value for the DMA frequency.
								//You can try various alternate values to find an optimal
								//value that works best for your scope, setup, & application.
								//Use program menu option "c" & "C" to vary the frequency while
								//watching your scope display; Using "c" & "C" options will allow you
								//to see how frequency changes effect the display quality in real-time.

	XYscope.begin(DmaFreq);

	//Timer3 is used as the CRT refresh timer.  This timer is setup inside of XYscope.begin( ).
	//However, paintCRT_ISR must be "attached" to timer 3.  To be properly link to the
	//refresh-screen XYscope interupt service routine, it must be linked in the Arduino 
	//setup() code as follows:
	
	Timer3.attachInterrupt(paintCrt_ISR);

	//Here is just some stuff to paint onto CRT at startup
	//v----------BEGIN SETUP SPLASH SCREEN ---------------v
	ArduinoSplash();				//Paint an Arduino logo
	int xC = 1800, yC = 2800;		//Set values of XY center coordinates for start of text
	int textSize = 400;				//Set Text Size (in pixels)
	bool const UndrLined = true;	//Turn underline ON
	int textBright = 150;
	XYscope.printSetup(xC - 150, yC + 50 + 700, textSize, textBright);
	XYscope.print((char *)"AGI", UndrLined);
	xC = 100;
	yC = 2900;
	textSize = 250;
	XYscope.printSetup(xC + 50, yC + 50 + textSize, textSize, textBright);
	XYscope.setFontSpacing(XYscope.prop);			//Select Proportional Spacing
	if (XYscope.getFontSpacing() != XYscope.mono)	//Adjust coordinate in currently active spacing mode is 'prop'
		XYscope.printSetup(xC + 50 + 500, yC + 50 + textSize, textSize,
				textBright);
	XYscope.print((char *)"Arduino Graphics Interface", false);	//(false=No underline)
	XYscope.setFontSpacing(XYscope.mono);

	XYscope.autoSetRefreshTime();
	XYscope.plotRectangle(0, 0, 4095, 4095);

	XYscope.printSetup(350, 275, 175, 100);
	XYscope.print(startMsg);XYscope.print((char *)" LibRev:");XYscope.print(XYscope.getLibRev(),2);

	//Now put the compile date & time onto the screen
	//The time and data stamp come from #define statements at the top of the program...
	XYscope.printSetup(1100, 100, 150, 100);
	XYscope.print((char *)"(");
	XYscope.print(CompileDateStamp);
	XYscope.print((char *)"  ");
	XYscope.print(CompileTimeStamp);
	XYscope.print((char *)")");

	//^----------BEGIN SETUP SPLASH SCREEN ---------------^

	//Send option menu out to PC via Serial.print
	Serial.println();
	Print_CRT_Scope_Menu();
	PrintStatsToConsole();

}

void loop() {	
	// Begin CRT_SCOPE Mainline code!
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170708 Ver 0.0	E.Andrews	First cut
	//						(Updated throughout development cycle without version change)
	//					

	//Here is the start of the DEMO CONTROL panel, run via Serial.print link to PC...
	int x0, y0, x1, y1;
	int xCtr;
	int yCtr;
	int radius;
	int xRadius;
	int yRadius;
	// This is main operator input decoder loop
	if (Serial.available()) {

		char d = Serial.read();
		Serial.println();
		Serial.println(d);

		switch (d) {	//Decode operator input received via the serial monitor link-up

			case 'h': {	//Print main HELP menu to console
				Print_CRT_Scope_Menu();	//Show Help Menu
				break;
			}

			case '?': {	//Print current memory usage stats to console
				Print_CRT_Scope_Menu();	//Show Help Menu
				PrintStatsToConsole();	//Print current STATS to console
				break;

			}

			case '0': {		//Test rouine for XYscope.print (number) routines.
							//Print samaple Numbers WITHOUT Underline
							//This is just a bunch of random nums sent to screen
							//using various number formats...
							//Note: this shows that only about 7 digits of resolution
							//are truely available for floating point numbers 
							
				short ChInt = 100;	//Brightness of text for this test
				float TstNum = random(-160000000, 160000000);	//Create a random number
				TstNum = TstNum / 10000;	//make it a bit smaller
				Serial.print((char *)"Trying to print: ");	//Send Message to Serial Monitor
				Serial.print(TstNum, 9);
				Serial.println((char *)"  ");
				//These two routines do essentially the same thing...
				XYscope.plotClear();	//clear screen and intialize a fresh plot
				XYscope.plotStart();	//clear screen and intialize a fresh plot
				
				int CharSize = random(200, 325);
				int xPos = 20, yPos = 4095 - CharSize - 50;
				
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				
				XYscope.print(int(TstNum));						//Print Integer
				
				yPos = yPos - CharSize - 50;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum);			//Print Float to default (2) places

				yPos = yPos - CharSize - 50;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 3);//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 4);	//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 5);	//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 6);	//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 7);	//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 8);	//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 9);//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 10);	//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 11);	//Print Floating point to specified number of places

				XYscope.autoSetRefreshTime();
				PrintStatsToConsole();	//Send memory usage report out to Serial Monitor

				break;
			}
			case '1': {		//Test rouine for XYscope.print (number) routines.
							//Print formatted Numbers WITH Underline
							//This is just a bunch of random numbers sent to screen
							//using various number formats with every other row underlined
							
				const int ChInt = 100;	//Brightness of text for this test
				float TstNum = random(-160000000, 160000000);
				TstNum = TstNum / 10000;
				Serial.print((char *)"Trying to print: ");
				Serial.print(TstNum, 9);
				Serial.println("  ");

				XYscope.plotClear();
				XYscope.plotStart();
				int CharSize = random(200, 325);
				int xPos = 20, yPos = 4095 - CharSize - 50;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(int(TstNum), bool(true));	//Print Integer

				yPos = yPos - CharSize - 50;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum);					//Print float to default (2) places

				yPos = yPos - CharSize - 50;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 3, bool(true));	//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 4);				//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 5, bool(true));	//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 6);				//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 7, bool(true));	//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 8);				//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 9, bool(true));	//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 10);				//Print Floating point to specified number of places

				yPos = yPos - CharSize;
				XYscope.printSetup(xPos, yPos, CharSize, ChInt);
				XYscope.print(TstNum, 11, bool(true));	//Print Floating point to specified number of places

				XYscope.autoSetRefreshTime();
				PrintStatsToConsole();

				break;
			}

			case '2': {	//Draw Circle inside of Box Pattern for CENTERING
				drawCenteringPattern();	//Use this pattern for scope and gain adjustments
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
			case '4': {	//Simulated graphical chart plot
				SimGraph_0();
				XYscope.autoSetRefreshTime();
				PrintStatsToConsole();
				break;
			}

			case '5': {	// Plot Random Eliptical figures
				XYscope.plotClear();
				XYscope.plotStart();
				XYscope.plotErr = 1;

				while (XYscope.plotErr > 0) {
					xCtr = int(random(0, 4096));
					yCtr = int(random(0, 4096));
					xRadius = int(random(0, 2048));
					yRadius = int(random(0, 2048));
					XYscope.plotEllipse(xCtr, yCtr, xRadius, yRadius);
					//TODO XYscope.plotEll(xCtr,yCtr,xRadius,yRadius);
					XYscope.autoSetRefreshTime();
				}
				PrintStatsToConsole();	//Send mem usage report to console
				break;
			}
			case '6': {	//Plot one single fixed size Ellipse
				XYscope.plotClear();
				XYscope.plotStart();
				xCtr = 2048;
				yCtr = 2048;
				xRadius = 500;
				yRadius = 1000;
				XYscope.plotEllipse(xCtr, yCtr, xRadius, yRadius);
				XYscope.autoSetRefreshTime();

				PrintStatsToConsole();	//Send mem usage report to console
				break;
			}
			case '7': {		//Scope sine wave pattern.  This one is good for op-amp adjustment
				XYscope.plotClear();
				XYscope.plotStart();
				XYscope.plotPoint(0, 4095);
				xCtr = 2047;
				yCtr = 2047;
				xRadius = 2047;
				yRadius = 2047;

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
				XYscope.plotEnd();
				XYscope.autoSetRefreshTime();

				PrintStatsToConsole();	//Send mem usage report to console

				break;

			}
			case '8': {	//plotARC test function - Draw series of Circle arc segments from 0 to 7
						//This shows how the segment variable can be used.  Note that Multiple segments can be 
						//plotted in one call as the 'segment' variable is 'bit' based.

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


					XYscope.plotLine(0, 4095, 4095, 0);		//Draw 135 Deg Line, TopLeft to BotRright
					XYscope.plotLine(4095, 4095, 0, 0);		//Draw 45 Deg Line, TopRight to BotLeft
					XYscope.plotLine(0, 2047, 4095, 2047);	//Center Horizontal Cross Hair
					XYscope.plotLine(2047, 0, 2047, 4095);	//Center Vertical Cross Hair

					XYscope.plotCircle(xCtr, yCtr, xRadius, aSeg);

					XYscope.printSetup(50, 2200, 300, 100);	//Set char height, x-location, y-location, Brightness%
					//Print segment report onto screen
					XYscope.print((char *)"Seg:");

					XYscope.print(segment);
					//Send status report to console to show which segment was generated...
					Serial.print(" Plotted ArcCircle pattern... Segment: ");
					Serial.print(segment);
					Serial.print(", Code: ");
					Serial.println(aSeg);

					//PAUSE (Stop and wait for any keypress...)
					WaitForAnyKey(" DONE ");
					aSeg = aSeg * 2;	//Shift left one position

				}
				PrintStatsToConsole();	//Send mem usage status report to console
				break;

			}
			case '9': {	//plotArcEllipse test function - Draw series of Ellilpse arc segments from segment code 0 to 7
						//This shows how the segment variable can be used.  Note that Multiple segments can be 
						//plotted in one call as the as the 'segments' varabile is 'bit' based.
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

					XYscope.setGraphicsIntensity(tempIntensity);//Return density to normal value

					XYscope.plotLine(0, 4095, 4095, 0);		//Draw 135 Deg Line, TopLeft to BotRright
					XYscope.plotLine(4095, 4095, 0, 0);		//Draw 45 Deg Line, TopRight to BotLeft
					XYscope.plotLine(0, 2047, 4095, 2047);	//Center Horizontal Cross Hair
					XYscope.plotEllipse(xCtr, yCtr, xRadius, yRadius, aSeg);//Draw just desired segment

					XYscope.printSetup(50, 2200, 300, 100);
					XYscope.print((char *)"Seg:");
					XYscope.print(segment);

					Serial.print(" Plotted ArcEllipse pattern... Segment: ");
					Serial.print(segment);
					Serial.print(", Code: ");
					Serial.println(aSeg);
					//PAUSE (Stop and wait for any keypress...)
					WaitForAnyKey(" DONE ");
					aSeg = aSeg * 2;	//Shift left one position
				}
				PrintStatsToConsole();	//Send mem usage status report to console
				break;
			}

			case '-': {	//Decrease Graphics Intensity Value
				if (XYscope.getGraphicsIntensity() - 10 > 0)
					//Decrement current Graphics Intensity by 10
					XYscope.setGraphicsIntensity(
							XYscope.getGraphicsIntensity() - 10);
				break;
			}

			case '+': {	//Increase Graphics Intensity Value
				if (XYscope.getGraphicsIntensity() + 10 < 255)
					//Increment current Graphics Intensity by 10
					XYscope.setGraphicsIntensity(XYscope.getGraphicsIntensity() + 10);
				break;
			}

			case '*': {	//Toggles sweep second hand animation ON and OFF

				if (enabSecondHand == 0) {
					enabSecondHand = 1;
					Serial.println("   Seconds Hand ENABLED");
				} else {
					enabSecondHand = 0;
					Serial.println("   Seconds Hand DISABLED");
				}
				break;
			}
			case 'F': {	//Increase FRONT PORCH timing by one count
				//Detailed timing adjustment...you shouldn't normally need to change this value
				//Increment Front-Porch Blanking
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
			case 'f': {	//Decrease FRONT PORCH timing by one count
				//Detailed timing adjustment...you shouldn't normally need to change this value
				//Decrement Front-Porch Blanking
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

			case 'B': {//Increase BACK PORCH timing by one count
				//Detailed timing adjustment...you shouldn't normally need to change this value
				//Increment Back-Porch Blanking
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
			case 'b': {	//Decrease BACK PORCH timing by one count
				//Detailed timing adjustment...you shouldn't normally need to change this value
				//Decrement Back Front-Porch Blanking
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
			case 'c': {	//Change Clock Frequency...
				//DECREASE DMA Clock Frequency in 50khz steps (Limit min value to 500 Khz)
				// Try different values to see how well your scope and setup perform.
				// Look for artifacts and defects in the graphics display, particularly when
				// graphics objects are spaced far apart.
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
			case 'C': {	//Change Clock Frequency...
				//INCREASE DMA Clock Frequency in 50 kHz steps (Limit max value to 1.2 Mhz)
				// Try different values to see how well your scope and setup perform.
				// Look for artifacts and defects in the graphics display, particularly when
				// graphics objects are spaced far apart.

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

			case 'd':	//Draw pattern to measure DAC raise and fall times
				//For use in testing and setting detailed timing adjustments...
				//Draw corner to corner dot pattern to Screen for worst case DAC evaluation
				PeakToPeakTest_D();
				PrintStatsToConsole();
				break;

			case 'D':
				//Draw Coordinate System Plot
				SimCoordinateSys();
				PrintStatsToConsole();
				break;

			case 'H':
				//Draw HORIZONTAL Sq Wave Pattern to Screen
				//For use in testing and setting detailed timing adjustments...
				PeakToPeakTest_H();
				PrintStatsToConsole();

				break;
			case 'V':
				//Draw VERTICAL Sq Wave Pattern to Screen
				//For use in testing and setting detailed timing adjustments...
				PeakToPeakTest_V();
				PrintStatsToConsole();
				break;

			case 'i': {
				//Decrease TEXT Intensity Value
				if (XYscope.getTextIntensity() - 10 > 0)
					XYscope.setTextIntensity(XYscope.getTextIntensity() - 10);
				break;
			}
			case 'w': {
				//Example of How To Wakeup from a screen saver blank event
				//To wakeup the screen, just call the setScreenSaverSecs routine with the Current Setting...
				// 1) Read the Current Setting
				long curScreenSaverSetting = XYscope.getScreenSaveSecs();
				// 2) Call routine with Current Setting
				XYscope.setScreenSaveSecs(curScreenSaverSetting);//This sequence wakes up screen and resets the time to sleep? counterresets save timer
				break;
			}
			case 'I': {
				//Increase TEXT Intensity Value
				if (XYscope.getTextIntensity() + 10 < 255)
					XYscope.setTextIntensity(XYscope.getTextIntensity() + 10);
				break;
			}
			case 'l': {	//Animation example...
				//Animation that Plots Arduino LOGO at various sizes
				int Lx = 100, Ly = 2000, Lht = 1000;
				XYscope.plotStart();
				bool const Underlined = true;
				XYscope.printSetup(2047 - int(float(4000 / 16) * 1.5), 1200, 400,
						100);
				XYscope.print((char *)"AGI", Underlined);
				short Tx = 150, Ty = 900;
				if (XYscope.getFontSpacing() != XYscope.mono)
					Tx = 775;
				XYscope.printSetup(Tx, Ty, 230, 100);
				XYscope.print((char *)"A", Underlined);
				XYscope.print((char *)"rduino ");		//(No-underline)
				XYscope.print((char *)"G", Underlined);
				XYscope.print((char *)"raphics ");		//(No-underline)
				XYscope.print((char *)"I", Underlined);
				XYscope.print((char *)"nterface");		//(No-underline)
				XYscope.printSetup(1100, 100, 175, 100);
				XYscope.print(startMsg);				//(No-underline)
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
					delay(40);	//Just a slight delay to slow redraws down a bit


				}

				XYscope.autoSetRefreshTime();
				PrintStatsToConsole();
				break;
			}
			case 'm': {
				//Toggle Font Space mode back and forth between Mono and Proportional
				if (XYscope.getFontSpacing() == XYscope.mono)
					XYscope.setFontSpacing(XYscope.prop);
				else
					XYscope.setFontSpacing(XYscope.mono);
				break;
			}

			case 's': {
				//Display sample text, Test #1
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
			case 'S': {
				//Display sample text, Test #2
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
			case 't': {
				//Show Analog Clock Hands simulation 1
				SimClock_1();
				EndOfSetup_Ptr = XYscope.XYlistEnd;
				XYscope.autoSetRefreshTime();
				PrintStatsToConsole();
				break;
			}
			case 'T': {
				//Show Analog Clock Hands simulation 2
				SimClock_0();
				EndOfSetup_Ptr = XYscope.XYlistEnd;
				XYscope.autoSetRefreshTime();
				PrintStatsToConsole();
				break;
			}
			case 'p':
				//Show Pong Demo (Animated; Type any key to end demo)
				PongDemo();
				break;

			default: {
				//Undefined input character detected; just print HELP screen to user
				Serial.println("^-- invalid/unknown command character entered ");
				XYscope.plotStart();		//Clear Screen and plot message...
				XYscope.setFontSpacing(XYscope.prop);	//Set Proportional Spacing
				XYscope.printSetup(100, 2047, 230, 100);
				XYscope.print((char *)"Unknown Option Entered...Try Again");
				XYscope.plotEnd();
				XYscope.autoSetRefreshTime();
				delay(3000);	//Wait 3 seconds
				//drawCenteringPattern();
				setup();		//Reset All and draw startup image
				PrintStatsToConsole();
				break;
			}

		}	//End of operator input decoder

	}

	else {

		float delta_Ang = (2 * PI) / (60);

		if ((enabSecondHand == 1) && (millis() > TimeForRefresh)) {
			//This is an early version of a moving second hand demo...Redundant to menu option 't' (Clock simulation)
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
	// PONG style game display to demo how to do animation.
	// This demo just moves a 'ball' around the screen bouncing off of walls.
	// Touch any key to terminate the demo and select another program
	// option.  The program could easily sample some analog ports and
	// use that data to 'move the paddles' and in turn actually code
	// a working 'PONG-Style' game.
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170708 Ver 0.0	E.Andrews	First cut
	//						(Updated slightly throughout development cycle without version change)
	//
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
	XYscope.print((char *)"18");
	XYscope.printSetup(2700, 3000, 800);
	XYscope.print((char *)"09");

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
			int d=0;
			while (d>=0) {
				d=Serial.read();	//clean out the buffer (Retrieved data is NOT USED)
			} 						//...flush buffer, keep reading until buffer is empty.  Note, Serial.read will return as -1 when no data is available

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
		//	Need to add scoring & reporting, ball reaction, ball sounds, etc...
		//  You Add this Part!!!

	}

}
//=========== END PONG DEMO =============================

//=======================================================
//	SPLASH SCREEN DEMO
//=======================================================
void ArduinoSplash() {
	// Locate and plot the Arduino-Logo to screen.
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20171008 Ver 0.0	E.Andrews	First cut
	//

	int Lx = 1150, Ly = 2000, Lht = 1000;
	XYscope.plotStart();
	XYscope.plotArduinoLogo(Lx, Ly, Lht);

}
//=========== END SPLASH =================================

void drawCenteringPattern(void) {
	// Plots a circle and square that can be used for scope centering and gain adjust
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170708 Ver 0.0	E.Andrews	First cut
	//

	XYscope.plotStart();
	int X_Center = 2047;
	int Y_Center = 2400;
	int Circ_Radius = 800;
	X_Center = 2048;
	Y_Center = 2048;

	XYscope.plotCircle(X_Center, Y_Center, Circ_Radius);		//try a circle
	Circ_Radius = 2045;
	XYscope.plotCircle(X_Center, Y_Center, Circ_Radius);		//try a circle

	XYscope.plotPoint(X_Center, Y_Center);	// Put a DOT at the center of the circ.
	XYscope.plotLine(4095, 4095, 0, 4095);	//TOP Border
	XYscope.plotLine(0, 4095, 4095, 0);		//TopLt to BotRt Diag
	XYscope.plotLine(4095, 0, 0, 0);		//BOTTOM Border
	XYscope.plotLine(0, 0, 4095, 4095);		//TopRt to BotLt Diag

	XYscope.plotLine(4095, 0, 4095, 4095);	//RIGHT Border
	XYscope.plotLine(0, 0, 0, 4095);		//LEFT Border

	EndOfSetup_Ptr = XYscope.XYlistEnd;		//Remember end of buffer ptr in case second hand is enabled

	XYscope.plotEnd();						//end with plot to 0,0
	XYscope.autoSetRefreshTime();
	ShowMemory();
	PrintStatsToConsole();
}

//================ START OF SIMULATED CLOCK ROUTINES ========================//
//DEFINE Global (Gbl) Variables used to draw the clock features

float Gbl_X_Center, Gbl_Y_Center, Gbl_Center_Radius, Gbl_MinHubCenterRadius;
float Gbl_Hour_HandLength,Gbl_Hour_EndRadius, Gbl_Hour_End_X_Center, Gbl_Hour_End_Y_Center;
float Gbl_Min_HandLength, Gbl_Min_EndRadius, Gbl_Min_End_X_Center, Gbl_Min_End_Y_Center;
float Gbl_Sec_HandLength, Gbl_Sec_End_X, Gbl_Sec_End_Y;
int Gbl_HR_ListPtr, Gbl_MIN_ListPtr, Gbl_SEC_ListPtr;

void SimClock_0(void) {
	//  This routine does a one time paint of a fixed clock image.
	//	You can use  the '*' key (at the main menu) to active a moving second-hand.
	//
	//	Note: This is just a fixed image of a clock face and was my first
	//	attempt to see how AGI would look as a CRT-Clock driver.  SimClock_1 came 
	//	later and is a more fully animated version with moving hour, minute, & second hands.
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170708 Ver 0.0	E.Andrews	First cut
	//						(Updated throughout project without version change)
	
	XYscope.plotClear();
	XYscope.plotStart();
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
			XYscope.print((char *)"12");
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

void SimClock_1(void) {
	//  This routine shows the basics of a CRT clock application
	//	This shows how DrawClockHourHand(Hr), DrawClockMinuteHand(Min),
	//	and DrawClockSecondHand(Sec) routines can be written and used.
	//	Take note of how the XYscope.XYlistEnd is managed to animate and
	//	redraw the hands. Also, note that ClockHandsSetup(...) is a routine
	//	that is used to define the location and size parameters for hands.
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20171106 Ver 0.0	E.Andrews	First cut
	//						(Minor updates throughout development without version change)

	XYscope.plotClear();
	XYscope.plotStart();
	int xCtr = 2047;
	int yCtr = 2047;
	int CenterRadius=200;
	int Min_HubCenterRadius = 100;
	int ClockRadius=1900;
	int HR_armRadius=ClockRadius-500;
	int MN_armRadius=ClockRadius;
	int SEC_armRadius=ClockRadius+120;
	int xRadius = 1900;
	int yRadius = 1900;
	//Initialize ClockHands global variables

	ClockHandsSetup(xCtr,yCtr,CenterRadius,Min_HubCenterRadius, HR_armRadius,MN_armRadius,SEC_armRadius);

	//	Here is where we draw the fixed, CLOCK FACE parts...
	Serial.print(" Run: CRT-Clock-Sim Routines...");
	float numTestPoints = 12;	//We want to have 12 text Hour Markers
	float testAngle = 0;
	int patternCount = 0;
	float angleStep = 2 * 3.141526 / numTestPoints;
	testAngle = testAngle - angleStep;
	const int charHt = 500;
	int HoursIntensity = 0;
	int OldTextIntensity = XYscope.getTextIntensity();
	XYscope.setGraphicsIntensity(100);
	for (patternCount = 0; patternCount < numTestPoints; patternCount++) {
		HoursIntensity = HoursIntensity + 20;	//Just for fun and test purposes, change
												//intensity of every hour value. Start at
												//20% and step brightness for each HOUR.
												//A real CRT clock will probably want to
												// have one intensity for all hour markers.
		HoursIntensity=150;	//Comment this line out if you want to plot each hour at different level
		if (HoursIntensity > 255)  HoursIntensity = 255;
		testAngle = testAngle + angleStep;
		int xP, yP;
		xP = int(xCtr + xRadius * sin(testAngle));
		yP = int(yCtr + yRadius * cos(testAngle));

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
			XYscope.printSetup(xP, yP, charHt, HoursIntensity);
			XYscope.print((char *)"12");
		} else
			XYscope.print(patternCount);
	}
	//Restore prior Text Intensity
	XYscope.setTextIntensity(OldTextIntensity);

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

		//Note: Here we skip plotting the SECONDS-MARK at each of the HOUR marks
		if ((patternCount + 1) % 5 != 0)
			XYscope.plotLine(xP1, yP1, xP2, yP2);
	}

	//Uncomment the following four (4) lines if you want add the Arduino-logo on the screen...
	//int logoHt = 1000;
	//int logoX = 2047 - logoHt * .85;
	//int logoY = 2047 - logoHt;
	//XYscope.plotArduinoLogo(logoX, logoY, logoHt);

	//Draw the center hubs of the HOURS and MINITE hands...
	XYscope.plotCircle(xCtr, yCtr, CenterRadius-5);			//Hour Hand Center Circle ('-5' just makes it look nicer!)
	XYscope.plotCircle(xCtr, yCtr, Min_HubCenterRadius-5);	//Minute Hand Center Circle
	XYscope.plotCircle(xCtr, yCtr, 10);						//Second-Hand Center Circle
	//Post message on how to stop demo...
	XYscope.printSetup(10,10,200,100);
	XYscope.setFontSpacing(XYscope.prop);
	XYscope.print((char*)"(Key Press=Stop)");

	//###################### Draw Hour Hand (in a Test mode) #########################
	Gbl_HR_ListPtr = XYscope.XYlistEnd;
	for (float TestHour=0; TestHour<13; TestHour=TestHour+.1){
		if (Serial.available()) TestHour=13; //Terminate early if key pressed
		XYscope.XYlistEnd=Gbl_HR_ListPtr;
		DrawClockHourHand(TestHour);
		delay(10);
	}
	XYscope.XYlistEnd=Gbl_HR_ListPtr;
	float TestHour=3.00;
	DrawClockHourHand(TestHour);	// Position & paint final location of Hour Hand at 3:30

	//###################### Draw Minute Hand (in a Test mode) #########################
	Gbl_MIN_ListPtr = XYscope.XYlistEnd;
	float TestMin;
	TestHour=3.00;
	for (TestMin=0; TestMin<60; TestMin=TestMin+.1){
		if (Serial.available()) TestMin=60; //Terminate early if key pressed
		XYscope.XYlistEnd=Gbl_HR_ListPtr;
		DrawClockHourHand(TestHour+TestMin/60);
		//XYscope.XYlistEnd=Gbl_MIN_ListPtr;
		Gbl_MIN_ListPtr=XYscope.XYlistEnd;
		DrawClockMinHand(TestMin);
		delay(10);
	}
	TestHour=2.00;
	TestMin=23;

	XYscope.XYlistEnd=Gbl_HR_ListPtr;
	DrawClockHourHand(TestHour+TestMin/60);
	DrawClockMinHand(TestMin);	// Position & paint final location of Minute Hand at :27 past the hour

	//###################### Draw Second Hand (in a Test mode) #########################
	Gbl_SEC_ListPtr = XYscope.XYlistEnd;
	//Second Hand
	float TestSec;
	for (TestSec=0; TestSec<60; TestSec++){
		if (Serial.available()) TestSec=60; //Terminate early if key pressed
		XYscope.XYlistEnd = Gbl_SEC_ListPtr;
		DrawClockSecHand(TestSec);
		delay(10);
	}

	TestMin++;
	XYscope.XYlistEnd=Gbl_HR_ListPtr;
	DrawClockHourHand(TestHour+TestMin/60);
	DrawClockMinHand(TestMin);
	Gbl_SEC_ListPtr =XYscope.XYlistEnd;
	DrawClockSecHand(TestSec);
	delay(1000);	//Slight pause before demo continues

	for (int TestLoop=0;TestLoop<2;TestLoop++){

		for (TestSec=0; TestSec<60; TestSec++){
			if (Serial.available()){	//Terminate early if key pressed
				TestSec=60;
				TestLoop=5000;
			}

			if (Serial.available()) TestSec=60; //Terminate early if key pressed
			XYscope.XYlistEnd = Gbl_SEC_ListPtr;
			DrawClockSecHand(TestSec);

			//Draw Digital Readout of Time as well...
			XYscope.printSetup(1200,2300,450,100);
			XYscope.print(int(TestHour));
			if (TestMin<10)XYscope.print((char *) ":0"); else XYscope.print((char *) ":");
			XYscope.print(int(TestMin));
			if (TestSec<10)XYscope.print((char *) ":0"); else XYscope.print((char *) ":");
			XYscope.print(int(TestSec));
			delay(1000);
		}
		TestSec=0;
		TestMin++;
		if(TestMin>59){
			TestMin=0;
			TestHour++;
			if (TestHour>12) TestHour=1;
		}
		XYscope.XYlistEnd=Gbl_HR_ListPtr;
		DrawClockHourHand(TestHour+TestMin/60);
		DrawClockMinHand(TestMin);
		Gbl_SEC_ListPtr =XYscope.XYlistEnd;
		DrawClockSecHand(TestSec);
		//Draw Digital Readout of Time as Well
		XYscope.printSetup(1200,2300,450,100);
		XYscope.print(int(TestHour));
		if (TestMin<10)XYscope.print((char *) ":0"); else XYscope.print((char *) ":");
		XYscope.print(int(TestMin));
		if (TestSec<10)XYscope.print((char *) ":0"); else XYscope.print((char *) ":");
		XYscope.print(int(TestSec));

	}


	//Redraw Second Hand...
	DrawClockSecHand(0);	//Position & paint final location of Second Hand at 53 second position

	if (Serial.available()){	//If stopped by Oper Key Press, dump the serial Buffer
		int d=0;
		while (d>=0) {			//clean out the buffer (Retrieved data is NOT USED)
			d=Serial.read();	//...flush buffer, keep reading until buffer is empty.
		} 						// (Serial.read returns -1 when no data is available)
	}

	XYscope.plotPoint(4095, 4095);
	XYscope.plotEnd();
	XYscope.autoSetRefreshTime();

	Serial.print(" DONE with CLOCK SIM.\n  Num of Points Plotted: ");
	Serial.print(numTestPoints);
	Serial.print("  XYlistEnd=");
	Serial.println(XYscope.XYlistEnd);
	Serial.println(" (Press a key to continue)");
	XYscope.plotEnd();
	EndOfSetup_Ptr = XYscope.XYlistEnd - 1;
	XYscope.autoSetRefreshTime();
	//flush serial buffer
	if (Serial.available()){
		char d = Serial.read();
		Serial.println();
	}

}


void ClockHandsSetup(int X_Center, int Y_Center, int Center_Radius,int MinCenterRadius, int Hour_Length, int Min_Length, int Sec_Length){
	//	Routine to set global variables used to drive the clock hands display
	//
	//	Calling parameters (All coordinates, radius, and length parameters are in "pixels")
	//		X_Center, Y-Center - These are the coordinates of the center of the clock
	//		Center_Radius - Defines the radius of the center of the Hour hand
	//		MinCenterRadius - Defines the radius for center of the Minute hand
	//		Hour_Length, Min_Length - Defines the length of the Hour/Minute hands
	//
	//	Returns:
	//		NOTHING
	//
	//	20171106 Ver 0.0	E.Andrews	First cut

	Gbl_X_Center=X_Center;
	Gbl_Y_Center=Y_Center;
	Gbl_Center_Radius=Center_Radius;
	Gbl_MinHubCenterRadius=MinCenterRadius;
	Gbl_Hour_HandLength=Hour_Length;
	Gbl_Min_HandLength=Min_Length;
	Gbl_Sec_HandLength=Sec_Length;


}
void DrawClockMinHand(float CurrentMinutes){
	//	CLOCK routine that plots a minute-hand
	//
	//	Passed Parameters:
	//		float Minutes	This is the current minutes value, 0<= Minutes <= 59
	//						If this value is out of range, 0 will be the minutes value used
	//
	//	This routine uses the following global parameters:
	//		float Gbl_X_Center, Gbl_Y_Center	- This is the XY center of the clock face.
	//		float Gbl_Center_Radius			- Radius of the central clock face 'circle'
	//		float Gbl_Min_HandLength			- This is the length of the clock arm
	//		int XYscope.ListEnd					- Standard XY_List array pointer
	//
	//	Returns: NOTHING
	//
	//	20171106 Ver 0.0	E.Andrews	First cut
	//
	int X0, Y0, X1, Y1;
	float mins;
	double TwoPi=atan(1)*8;

	//Figure out the Theta, the ANGLE (in radians) of the hand based on the passed parameter.FYI: 0=12 o'clock, Pi=6 o'clock, etc...
	mins=CurrentMinutes;
	if (mins<0 or mins>59) mins=0;			//Bound the incoming value
	double theta = TwoPi * mins/60.;		//This is the angle of the Minute hand
	//theta = const_2pi / 60. * seconds;	//This is the angle of the Second Hand
	//theta = const_2pi / 12. * hours;		//This is the angle of the Hour Hand

	//Calculate the Coordinates for the center point of the END of the hand
	//Gbl_Hour_End_X_Center = Gbl_X_Center + Gbl_Hour_HandLength * sin(theta);	//=+(Xc0)+Hand_Len*SIN(Theta)
	//Gbl_Hour_End_Y_Center = Gbl_Y_Center + Gbl_Hour_HandLength * cos(theta);	//=+(Yc0)+Hand_Len*COS(Theta)
	Gbl_Min_End_X_Center = Gbl_X_Center + Gbl_Min_HandLength * sin(theta);	//=+(Xc0)+Hand_Len*SIN(Theta)
	Gbl_Min_End_Y_Center = Gbl_Y_Center + Gbl_Min_HandLength * cos(theta);	//=+(Yc0)+Hand_Len*COS(Theta)



	//Calculate the Coordinates for LINE#1 of arm and draw this line	(min and hour hand only)
	X0= int(Gbl_X_Center - Gbl_MinHubCenterRadius*cos(theta));					//+=Xc0-Radius_1*COS(Theta)
	Y0= int(Gbl_Y_Center + Gbl_MinHubCenterRadius*sin(theta));					//=+Yc0-Radius_1*SIN(Theta)
	X1=	int(Gbl_Min_End_X_Center);			//=+(Xc1_)-Radius_2*COS(Theta)
	Y1=	int(Gbl_Min_End_Y_Center);			//=Yc1_+Radius_2*SIN(Theta)
	XYscope.plotLine(X0,Y0,X1,Y1);

	//Calculate the Coordinates for LINE#2 of arm and draw this line	(min and hour hand only)
	X0= int(Gbl_X_Center + Gbl_MinHubCenterRadius*cos(theta));					//=+Xc0+Radius_1*COS(Theta)
	Y0= int(Gbl_Y_Center - Gbl_MinHubCenterRadius*sin(theta));					//=+Yc0-Radius_1*SIN(Theta)
	X1=	int(Gbl_Min_End_X_Center);			//=+Xc1_+Radius_2*COS(Theta)
	Y1=	int(Gbl_Min_End_Y_Center);			//=+Yc1_-Radius_2*SIN(Theta)
	XYscope.plotLine(X1,Y1,X0,Y0);	//Reverse order of vector for best plot quality
}
void DrawClockHourHand(float CurrentHour){
	//	CLOCK routine that plots the Hour-hand
	//
	//	Passed Parameters:
	//		float Hours		This is the current Hours value, 0<= Hour <= 12
	//						If this value is out of range, 12 will be the Hours value used
	//						Note: An Hours value such as 3.5 will put hour hand at 3:30 position.
	//
	//	This routine uses the following global parameters:
	//		float Gbl_X_Center, Gbl_Y_Center	- This is the XY center of the clock face.
	//		float Gbl_Center_Radius			- Radius of the central clock face 'circle'
	//		float Gbl_Hour_HandLength			- This is the length of the clock arm
	//		float Gbl_Hour_EndRadius			- Radius of the circle plotted at the 'end' of the clock arm
	//		int XYscope.ListEnd					- Standard XY_List array pointer
	//
	//	Returns: NOTING
	//
	//	20171106 Ver 0.0	E.Andrews	First cut
	//
	int X0, Y0, X1, Y1;
	float Hours;
	double TwoPi=atan(1)*8;

	//Figure out the Theta, the ANGLE (in radians) of the hand based on the passed parameter.FYI: 0=12 o'clock, Pi=6 o'clock, etc...
	Hours=CurrentHour;
	if (Hours<0 or Hours>12) Hours=12;			//Bound the incoming value
	//double theta = TwoPi * mins/60.;			//This is the angle of the Minute hand
	//double theta = TwoPi * seconds / 60.;		//This is the angle of the Second Hand
	double theta = TwoPi * Hours/ 12.;			//This is the angle of the Hour Hand

	//Calculate the Coordinates for the center point of the END of the hand
	Gbl_Hour_End_X_Center = Gbl_X_Center + Gbl_Hour_HandLength * sin(theta);	//=+(Xc0)+Hand_Len*SIN(Theta)
	Gbl_Hour_End_Y_Center = Gbl_Y_Center + Gbl_Hour_HandLength * cos(theta);	//=+(Yc0)+Hand_Len*COS(Theta)
	//Gbl_Min_End_X_Center = Gbl_X_Center + Gbl_Min_HandLength * sin(theta);	//=+(Xc0)+Hand_Len*SIN(Theta)
	//Gbl_Min_End_Y_Center = Gbl_Y_Center + Gbl_Min_HandLength * cos(theta);	//=+(Yc0)+Hand_Len*COS(Theta)



	//Calculate the Coordinates for LINE#1 of arm and draw this line	(min and hour hand only)
	X0= int(Gbl_X_Center - Gbl_Center_Radius*cos(theta));					//+=Xc0-Radius_1*COS(Theta)
	Y0= int(Gbl_Y_Center + Gbl_Center_Radius*sin(theta));					//=+Yc0-Radius_1*SIN(Theta)
	X1=	int(Gbl_Hour_End_X_Center);			//=+(Xc1_)-Radius_2*COS(Theta)
	Y1=	int(Gbl_Hour_End_Y_Center);			//=Yc1_+Radius_2*SIN(Theta)
	XYscope.plotLine(X0,Y0,X1,Y1);


	//Calculate the Coordinates for LINE#2 of arm and draw this line	(min and hour hand only)
	X0= int(Gbl_X_Center + Gbl_Center_Radius*cos(theta));					//=+Xc0+Radius_1*COS(Theta)
	Y0= int(Gbl_Y_Center - Gbl_Center_Radius*sin(theta));					//=+Yc0-Radius_1*SIN(Theta)
	X1=	int(Gbl_Hour_End_X_Center);			//=+Xc1_+Radius_2*COS(Theta)
	Y1=	int(Gbl_Hour_End_Y_Center);			//=+Yc1_-Radius_2*SIN(Theta)
	XYscope.plotLine(X1,Y1,X0,Y0);	//Reverse order of vector for best plot quality
}
void DrawClockSecHand (int Secs){
	//	Clock Routine that plots the SECONDS-hand
	//
	//	Passed Parameters:
	//		int Secs		This is the current Secs value, 0<= Secs <= 59
	//						If this value is out of range, 0 will be used
	//
	//	This routine uses the following global parameters:
	//		float Gbl_X_Center, Gbl_Y_Center	- This is the XY center of the clock face.
	//		float Gbl_Center_Radius			- Radius of the central clock face 'circle'
	//		float Gbl_Secs_HandLength			- This is the length of the clock arm
	//		int XYscope.ListEnd					- Standard XY_List array pointer
	//
	//	Returns: NOTING
	//
	//	20171106 Ver 0.0	E.Andrews	First cut
	//
	double TwoPi=atan(1)*8;
	float angle= (TwoPi*Secs) / (60);

	int X_end=Gbl_X_Center+Gbl_Sec_HandLength*sin(angle);
	int Y_end=Gbl_Y_Center+Gbl_Sec_HandLength*cos(angle);
	//Plot a simple vector (line) as the second hand
	XYscope.plotLine(Gbl_X_Center, Gbl_Y_Center, X_end, Y_end);


}
//================ END SIMULATED CLOCK ROUTINES ========================//

void SimCoordinateSys(void) {
	//	Routine to plot a sample graphic to show the AGI (X,Y) Coordinate system
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
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
	XYscope.print((char *)"(");
	XYscope.print(xC);
	XYscope.print((char *)",");
	XYscope.print(yC);
	XYscope.print((char *)")");

	xC = 0;
	yC = 4095;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC + 100, yC - 50 - textSize, textSize, textBright);
	XYscope.print((char *)"(");
	XYscope.print(xC);
	XYscope.print((char *)",");
	XYscope.print(yC);
	XYscope.print((char *)")");

	xC = 4095;
	yC = 4095;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC - 50 - (11 * (textSize * .65)), yC - 50 - textSize,
			textSize, textBright);
	XYscope.print((char *)"(");
	XYscope.print(xC);
	XYscope.print((char *)",");
	XYscope.print(yC);
	XYscope.print((char *)")");

	xC = 4095;
	yC = 0;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC - 50 - (8 * (textSize * .65)), yC + 50, textSize,
			textBright);
	XYscope.print((char *)"(");
	XYscope.print(xC);
	XYscope.print((char *)",");
	XYscope.print(yC);
	XYscope.print((char *)")");

	xC = 2047;
	yC = 2047;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC + 50, yC - 50 - textSize, textSize, textBright);
	XYscope.print((char *)" (");
	XYscope.print(xC);
	XYscope.print((char *)",");
	XYscope.print(yC);
	XYscope.print((char *)")");

	xC = 1023;
	yC = 1023;
	PlotCrossHair(xC, yC);
	XYscope.printSetup(xC + 50, yC - 50 - textSize, textSize, textBright);
	XYscope.print((char *)" (");
	XYscope.print(xC);
	XYscope.print((char *)",");
	XYscope.print(yC);
	XYscope.print((char *)")");

	xC = 1650;
	yC = 2047;

	textSize = 400;
	textBright = 150;
	XYscope.printSetup(xC + 50, yC + 50 + 700, textSize, textBright);
	XYscope.print((char *)"AGI", UndrLined);
	xC = 100;
	yC = 2047;
	textSize = 350;
	XYscope.printSetup(xC + 50, yC + 50 + textSize, textSize, textBright);
	if (XYscope.getFontSpacing() != XYscope.mono)
		XYscope.printSetup(xC + 50 + 500, yC + 50 + textSize, textSize,
				textBright);
	XYscope.print((char *)"Coordinate System", UndrLined);

	//PlotCrossHair(2047,2047);

	XYscope.plotEnd();
	XYscope.autoSetRefreshTime();
	ShowMemory();
	PrintStatsToConsole();
}

void PlotCrossHair(int Xcoord, int Ycoord) {
	//	Routine to draw a cross hair symbol to screen.  Size is fixed by const definitions.
	//            |
	//          - * -
	//            |
	//	Calling parameters (All coordinate parameters are in "pixels")
	//		X_Coord, Ycoord - Defines the XY-Center of the symbol
	//
	//	Returns:
	//		NOTHING
	//
	//	20171022 Ver 0.0	E.Andrews	First cut
	
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
	//	Routine to plot a sample graphic chart display
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170811 Ver 0.0	E.Andrews	First cut
	//
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
	XYscope.print((char *)"      Flux-Capacitance");
	XYscope.printSetup(100, 3600, 250);
	XYscope.print((char *)"       Vs Giga-Watts");
	XYscope.plotEnd();
	XYscope.autoSetRefreshTime();
	ShowMemory();
	PrintStatsToConsole();
}

void PeakToPeakTest_V() {
	//	Routine to plot a Vertical edge-to-edge Freq Response Test pattern
	//	   +--+  +--+  +--+  +--+
	//     |  |  |  |  |  |  |  |
	//   ..+  +--+  +--+  +--+  +...
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170811 Ver 0.0	E.Andrews	First cut
	//
	XYscope.plotStart();
	//Dashed Line, Top and Bottom
	for (int i = 0; i < 4095; i = i + 256) {
		XYscope.plotLine(i, 4095, i + 127, 4095);//Dashed line at Top of Screen
		XYscope.plotLine(i + 128, 0, i + 255, 0);//Dashed Line at Bottom of Screen
	}
	Serial.println(" DONE PeakToPeak_Vert Sq Wave ");
}
void PeakToPeakTest_H() {
	//	Routine to plot Horizontal edge-to-edge Freq Response Test pattern
	//	:
	//  +----+
	//       |
	//	+----+
	//	|
	//  +----+
	//       |
	//	+----+
	//	:
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170811 Ver 0.0	E.Andrews	First cut
	//
	XYscope.plotStart();
	//Dashed Line, Left & Right
	for (int i = 0; i < 4095; i = i + 256) {
		XYscope.plotLine(0, i, 0, i + 127);	//Dashed line at Left Side of Screen
		XYscope.plotLine(4095, i + 128, 4095, i + 255);	//Dashed Line at Right Side of Screen
	}
	Serial.println(" DONE PeakToPeak_Horiz Sq Wave ");
}

void PeakToPeakTest_D() {
	//	Routine to plot just dots in the corners of the screen.
	//	This pattern was used to vary and optimize the blanking timing
	//	parameters.	
	//	
	//    *            *
	//      
	//	
	//	
	//       
	//    *            *
	//	
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170811 Ver 0.0	E.Andrews	First cut
	//

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
	XYscope.plotPoint(4095, 4095);
	XYscope.plotPoint(0, 0);
	XYscope.plotPoint(4095, 4095);
	XYscope.plotPoint(0, 0);
	XYscope.plotPoint(4095, 4095);
	XYscope.plotPoint(0, 0);
	XYscope.plotPoint(4095, 4095);
	XYscope.plotEnd();

	Serial.println(" DONE PeakToPeak Corner_to_Corner Test ");
}
void PrintStatsToConsole(void) {
	//	Routine to send memory usage status reports out to the 
	//	serial monitor port.  Use this to observe how various graphics
	//	objects consume the graphics list RAM memory.  Routine also
	//	also displays current clock speeds in use so as you change
	//	them and observe performance differences, you can zero in on
	//	what works best for your scope.
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170811 Ver 0.0	E.Andrews	First cut
	//
	Serial.print("\n STATS..............\n MaxBuffSize: ");
	Serial.print(XYscope.MaxBuffSize);
	Serial.print("  Total Array Used: ");
	Serial.print((XYscope.XYlistEnd - 1));
	Serial.print(" (");
	Serial.print((XYscope.XYlistEnd * 100) / XYscope.MaxBuffSize);
	Serial.println(" %)");
	Serial.print("     DMA Clock Freq: ");
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
	Serial.print("       Refresh Freq: ");
	Serial.print(float(1000000) / float(XYscope.ActiveRefreshPeriod_us),1);
	Serial.print(" Hz (Period: ");
	Serial.print(XYscope.ActiveRefreshPeriod_us/1000);
	Serial.print(" ms)\n");
	Serial.print("Graphics Int: ");
	Serial.print(XYscope.getGraphicsIntensity(), DEC);
	Serial.print(" %   Text Int: ");
	Serial.print(XYscope.getTextIntensity(), DEC);
	Serial.println(" %");
	Serial.print("Font Spcng Mode = ");
	if (XYscope.getFontSpacing() == 0)
		Serial.print("PROP");
	else
		Serial.print("MONO");
	Serial.print(", Second_Hand_Enab = ");
	if (enabSecondHand)
		Serial.print("YES,");
	else
		Serial.print("NO,");
	Serial.print(" Scrn_Sav_Secs: ");
	Serial.print(XYscope.getScreenSaveSecs());
	Serial.println();
}
void Print_CRT_Scope_Menu() {
	//	Routine to send a menu of all available options to 
	//	the serial monitor port.  This routine is continuously
	//	updated as options are added or changed in XYscope.
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170725 Ver 0.0	E.Andrews	First cut
	//						(changes made continuously without version num updates)
	//

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
	Serial.println("  t     TIME - A Simulated Clock Display (Random Time)");
	Serial.println("  *     Toggle 'SimClock' second-hand On-Off");
	Serial.println("  p     Pong Simulation");

	Serial.println("==========================================");

}

//=======================================================
// DIAGNOSTIC ROUTINES
//=======================================================
void WaitForAnyKey(String msg) {
	// Routine outputs 'msg' to serial monitor port using Serial.print then halts execution and 
	// waits for Opr to hit any key.  Then it will return so execution can continue.
	// Used as a diagnostic tool to provide a 'halt execution and wait' function for
	// debugging use
	//
	//	Passed Parameters	
	//		msg - 	This string is output to the Serial Monitor port, then
	//				execution halts until the operator touches any key.
	//
	//	Returns: NOTHING
	//
	//	20170708 Ver 0.0	E.Andrews	First cut
	//

	boolean wait;
	Serial.print(msg);
	Serial.println(F(" <-WAITING! (...Hit any Key to CONTINUE...)"));
	wait = true;
	while (wait) {
		if (Serial.available()) {
			int d=0;
			while (d>=0) {
				d=Serial.read();	//clean out the buffer (Retrieved data is NOT USED)
			}						//...keep reading until buffer is empty; note, Serial.read will return as -1 when no data is available
			wait = false;
			Serial.println(F("_KP_"));
			delay(100);
		}

	}
}
void ShowMemory() {
	// Routine outputs an estimate of the total RAM in active use to the serial monitor port.
	// Used during development to observe RAM consumption.
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20170725 Ver 0.0	E.Andrews	First cut
	//

	char *heapend = sbrk(0);
	register char * stack_ptr asm ("sp");
	struct mallinfo mi = mallinfo();
	printf("\n DUE Dynamic ram used: %d\n", mi.uordblks);
	printf("  DUE Program static ram used %d\n", &_end - ramstart);
	printf("  DUE Stack ram used %d\n\n", ramend - stack_ptr);
	printf("     ... My guess at free mem: %d\n",
			stack_ptr - heapend + mi.fordblks);
}
