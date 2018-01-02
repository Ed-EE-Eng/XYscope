
#ifndef XYs		//Include-Guard to prevent multiple includes...
#define XYs

#if (ARDUINO >=100)
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//Make sure we are compiling for an Arduino DUE processor
#if not(__SAM3X8E__)
	#error WRONG PROCESSOR SELECTED - XYscope Library is only usable with Arduino DUE Processor
#endif



//void paintCrt2_ISR();						//This is the Refresh CRT ISR declaration


//void XY_TimerTestISR();						//TODO this is a test routine...Delete ASAP
//void initiateDacDma(void);					//Fires off a DacDMA transfer
//void initiateDacDma(short *ArrayPtr,int NumPoints,int blankCount,int crtBlankingPin); //Another version of the InitialDacDMA routine


class XYscope{
  public:
	// Constructor
	XYscope();
	
	//Methods

	//Setup and Control Routines
	void begin(uint32_t dmaFreqHz=800000);				//Initializes and enables DAC, DAC Counter Timer, DMA controller, Refresh Counter Timer
	void initiateDacDma(void);							//NOT A USER ROUTINE: Fires off a DacDMA transfer
	void dacHandler(void);								//NOT A USER ROUTINE: This is link to DAC_TRANSFER_COMPLETED interrupt

	void setScreenSaveSecs(long ScreenOnTime_sec);	//Set the screen saver time. Set to Zero to disable screen saver.
	long screenOnTimeDefaultSec=600;				//Define power-up default setting.

	long getScreenSaveSecs(void);					//Retrieve current screen saver time. Zero=screen saver disabled.

	void setDmaClockRate(uint32_t New_XfrRateHz);	//Allows user to change output Clk rate during run-time

	void setRefreshPeriodUs(uint32_t refresh_us);	//Can be used to manually change the refresh period (us) during run-time.

	void autoSetRefreshTime();						//Automatically sets the best refresh time based on the number points being plotted
	long getRefreshPeriodUs(void);					//Retrieves the currently active refresh period (us).

	//Buffer Management Routines
	void plotStart();				//Reset current buffer pointer to zero, effectively erasing the existing XY_List array
	void plotClear();				//Reset current buffer pointer to zero, effectively erasing the existing XY_List array and turning the display OFF
	void plotEnd();					//Makes sure last points in XYlist are actually actually visualized

	//Graphics Plotting Routines

	void setGraphicsIntensity(short GraphBright=100);	//Set (Get) brightness of Lines, Circles, Ellipse, Rectangles (in percent)
	short getGraphicsIntensity();							//Nominal setting is 100. Usable range is 50-200.

	short getGraphDensity();								//OBSOLETE-DO NOT USE. Function returns dot-to-dot spacing value in use (driven by intensity setting)

	void setTextIntensity(short TextBright=100);		//Set (Get) brightness of Text (in percent)
	short getTextIntensity();								//Nominal setting is 100. Usable range is 50-200.
	short getTextDensity();								//OBSOLETE-DO NOT USE. Function returns dot-to-dot spacing value in use (driven by intensity setting)

	void plotPoint(int x0, int y0);											// Plots a POINT
	void plotLine(int x0, int y0, int x1, int y1);							// Plots Lines (aka: a Vector)
	void plotRectangle(int x0, int y0, int x1, int y1);						// Plots a rectangle	
	void plotCircle(int xc, int yc, int r);									// Plots a circle centered at (xc,yc) of radius "r"	
	void plotCircle(int xc, int yc, int r, uint8_t arcSegment);				// Plots a circle centered at (xc,yc) of radius "r" BUT just specified arcSegment(s) of the circle.
	void plotCircleBres(int xc, int yc, int r, uint8_t arcSegment);			// Bresenham Algorthm:Plots a circle centered at (xc,yc) of radius "r" BUT just specified arcSegment(s) of the circle.
	void plotEllipse(int xc, int yc, int xr, int yr);						// Plots an ellipse
	void plotEllipse(int xc, int yc, int xr, int yr,uint8_t arcSegment);	// Plots an ellipical arc BUT just specified arcSegment(s)of the ellipse.
	void plotEllipseBres(int xc, int yc, int xr, int yr,uint8_t arcSegment=255);	//Bresenham Algorthm:Plots an ellipse (does Not use Bressham!)
	
	void plotChar(char c, int& x0, int& y0, int& charHt);					//Plot a single Character
	void plotCharUL(char c, int& x0, int& y0, int& charHt);					//Plot a single character WITH UNDERLINE

	void plotArduinoLogo(int& charX, int& charY, int& charHt);				//Plot the Arduino LOGO to screen

	const short prop=0, monoTight=8, mono=10, monoNorm=10, monoWide=12;	//Define FontSpacing  Constants
	void setFontSpacing(short spacingMode=0);	//Set Font Proportional or mono spaced mode...
	short getFontSpacing();						//retrieve current font spacing

	void printSetup(short textX, short textY);									//Set X-Y location for start of text
	void printSetup(short textX, short textY, short textSize);					//Set X-Y location for start of text, set textSixe (pixels)
	void printSetup(short textX, short textY, short textSize, short textDensity);	//Set X-Y location for start of text, set textSixe (pixels), textDensity(Brightness)
	
	void printUnderline(int nPlaces);			//Print UNDERLINE character(s)
	void print(char* text);						//Print text string (starting at current X-Y location, size, & density, no underline)
	void print(char* text, bool UL_flag);		//Print text string (starting at current X-Y location, size, & density, with/without UNDERLINE)
	void print(int number);						//Print integer to screen (starting at current X-Y location, size, & density, no underline)
	void print(int number,bool UL_Flag);		//Print integer to screen (starting at current X-Y location, size, & density, with/without UNDERLINE)
	void print(float number);					//Print floating point number to screen (starting at current X-Y location, size, & density), nPlaces to right of DP
	void print(float number,int placesToPrint);	//Print floating point number to screen (starting at current X-Y location, size, & density), nPlaces to right of DP, without UNDERLINE
	void print(float number,int placesToPrint,bool UL_Flag);	//Print floating point number to screen (starta at current X-Y location, size, & density), nPlaces to right of DP, with/without UNDERLINE

	

	void disableDac(void); 					//NOT A USER ROUTINE - Turns dac Off...May not need this

	//========================================
	//Global Variables & Constants
	//========================================
	static const uint8_t crtBlankingPin=3;		//CRT_Blanking pin (1=CRT_OFF, 0=CRT_ON)
	uint16_t frontPorchBlankCount=100;			//Calculated and set by setDmaClockRate(int dmaClkFreq); Used to define the duration before CRT unblanks & displays starts at START of DMA transfer
	uint16_t backPorchBlankCount=100;			//Calculated and set by setDmaClockRate(int dmaClkFreq); Used to define the duration before CRT blanking starts and display ENDS after the completion of a DMA transfer
	uint32_t DmaClkFreq_Hz;						//Currently Active DMA Clock FREQUENCY (Hz)
	float DmaClkPeriod_us;						//Currently Active DMA Clock PERIOD (us)

	uint32_t ActiveRefreshPeriod_us;			//Currently Active REFRESH Clock PERIOD (us)

	int MaxBuffSize=15000;						//This sets the size of the XY point plotting buffer.
												// NOTE: Each XY point pair consumes 4 bytes of RAM. This means that a buffer size of 15000
												// actually uses 15K X 4 = 60K bytes of RAM.  To leave room for other variables, MaxBufferSize should
												// NEVER exceed MaxArraySize (about 20000).  Also note that a lots of XY points can require longer refresh times
												// which will lead to display flicker!

	static const uint32_t MaxArraySize=17000;	//This sets the max Array size reserved for XY list.  MaxBufferSize must always be <= MaxArraySize!

	int XYlistEnd;								//This value points to the last element loaded into XYlist[] array
												//and is automatically maintained by Driver Routine "plotPoint"
												//By using the plotPoint routine, new points are automatically added at the END of the current list.
												//If you want to add point pairs directly into XYlist yourself, you must watch & maintain this value
												//and you must FOLLOW THESE RULES!
												//  1. Always use plotStart routine to setup a new list.  This places the needed snychronization pattern into the
												//     very start of a new list.  The hardware interface needs this pattern to run properly.
												//	2. When adding data into the XY_List array, you MUST include the X & Y flag values
												//     that are set into the high order bits of the X and Y integers. Look inside of plotPoint for details!
												//	3. XYlistEnd must stay BELOW MaxBufferSize or else you'll clobber other variables!
												//  4. XYlistEnd must be updated every time a point is added to the lsit.
												//	5. XYlistEnd always points the the NEXT AVAILABLE LOCATION in the XYList[] array.
												//	6. If the total number of points in the list gets too big, they may not all be displayed!
												//     If this happens, either reduce the the number of points in the list, or increase the refresh period.
												//	   You increase the the regresh period by changing the value of crtRefreshMs found else where in this file.						//


	// Define Structure of XY Point List.  This is the actual data array that holds the points to be plotted to the CRT.
	//  Use plotPoint or the other graphic draw routines to add points into this array.
	//  Use plotStart to initialize and/or start over with a new list

	struct pointList{
		short X;	//X-coordinate value of a point. Valid range: 0-4095 (See also the "rules" in XYlistEnd comment above!)
		short Y;	//Y-coordinate value of a point. Valid range: 0-4095 (See also the "rules" in XYlistEnd comment above!)
	};
	pointList XY_List[MaxArraySize];	//This reserves & defines the RAM allocated for the XY_List.  Actual value of usable space is set by variable MaxBuffSize



	//Define partial-circle/partial-ellipse "Arc-Codes"
	static const uint8_t arc0=1;
	static const uint8_t arc1=2;
	static const uint8_t arc2=4;
	static const uint8_t arc3=8;
	static const uint8_t arc4=16;
	static const uint8_t arc5=32;
	static const uint8_t arc6=64;
	static const uint8_t arc7=128;
	
	
		
	uint8_t plotErr;						//plotPoint routine sets this variable when ever we attempt to plot too many points



	static const long CrtMinRefresh_ms=20;	//This variable sets the target refresh rate for the display
											//  Note: 20ms = 50Hz CRT refresh rate
											//  Note: Although not recommended, you can set value=0 for fastest possible refresh.
													
	
	static const uint16_t X_flag=0x0000;	//These are routing codes used by the DMA hardware to route each integer the right DAC
	static const uint16_t Y_flag=0x1000;

	// Define & initialize global text X-Y coordinates and text size variables
	int charX=0,charY=4095, charSize=50;
	


	#include "VectorFontROM.h"		//Include Font ROM file

  private:
	//Methods
	//void paintCrt_isr();			//This routine Calls for a SCREEN REFRESH every crtRefreshMs milliseconds...this routine to refresh CRT as needed
	void dacSetup (void);				//Called within begin(). Initializes and enables dac peripherals.
	void tcSetup (uint32_t XfrRateHz);	//Called within begin().  Used to initialize Timer Counter TC0 (Drive DAC_DMA channel) at target transfer rate
	uint32_t FreqToTimerTicks(uint32_t freqHz);	//Used within tcSetup to set DMA_Clock Rate


	//Private Variables

	//int _density=5;		//This is the global "active_density" value
	int _graphDensity;		//value calculated by/set by call to SetGraphicsIntensity(int brightness)
	int _graphBrightness;	//value that is set by call to SetGraphicsIntensity(int graphbrightness)

	int _textDensity;		//value calculated by/set by call to SetTextIntensity(int brightness)
	int _textBrightness;	//value that is set by call to SetGraphicsIntensity(int graphbrightness)

	long _crtOffTOD_ms;		//Screen save time in ms; Zero means screen saver disabled.
	long _screenOnTime_ms;	//TOD (ms) when Screen should next be blanked;
							//Define Font Spacing variable
	short _fontSpacing=8;

};
#endif // XYscope


