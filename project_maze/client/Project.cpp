/*
 Project "Maze Runner"
*/
//include all the appropiate libraries and header files
#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <TouchScreen.h>
#include <screenAndTouch.h>



// define all the static variables
#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6


#define YP A2
#define XM A3
#define YM 5
#define XP 4

#define JOY_VERT_ANALOG A1
#define JOY_HORIZ_ANALOG A0
#define JOY_SEL 2



#define JOY_DEADZONE 64
#define JOY_CENTRE 506
#define JOY_STEPS_PER_PIXEL 64

#define TFT_WIDTH 320
#define TFT_HEIGHT 240

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define CURSOR_SIZE 9


#define r 21 //r is max row, c is max column for the maze, not the tft screen
#define c 29 //Array[r,c] is also the fix destination

//declare all the global variables and intialize some of them
int touchX, touchY;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

unsigned int time;
int wl;
int mode; //when mode = 0, the whole game quits
int digitalInPin = 13;
int analogInPin = A1;

int x = 1, y = 1; //curent value for the cursor, also the starting point
int oldx, oldy;
int array[r + 2][c + 2];








//1. maze set up section. 

void setup(){
	init();
	Serial.begin(9600);
	
	pinMode(JOY_SEL, INPUT_PULLUP);
	tft.begin();

	tft.fillScreen(ILI9341_BLACK);

	tft.setRotation(3);
	Serial.flush();
}

/*
 Function name: get_data(), 
 definition: communicatoin 
*/
void get_data(){
	String data;
	x = 1;
	y = 1;
	
	for(int i = 0; i <= r + 1; i++){
		while(Serial.available() == 0){}
		data = Serial.readString();
		for(int j = 0; j <= c + 1; j++){
			
			
			//Maze array, when array[i,j] = 1, it is a block, = 0 open path
			array[i][j] = String(data[j]).toInt(); 
		}
		Serial.println("N");
	}
}

/*
 Function name: gameMapSetup() , 
 definition: set up the maze and also print out the blocks 
*/
void gameMapSetup(){
	tft.fillScreen(ILI9341_BLUE);
	for (int i = 0; i <= r + 1; i++){
		for (int j = 0; j <= c + 1; j++){
			// print the blocks  
			if (array[i][j] == 1){
				tft.fillRect(j * 10, i * 10, CURSOR_SIZE, CURSOR_SIZE, ILI9341_BLACK);
			}
		}
	}
}

/*
 Function name: timer() , 
 definition: a timer count down, when it hits zero, you lose the game
*/
bool timer(){
	int count;
	count = (int(millis() - time) / 1000) % 24;
	tft.fillRect(10 * (c + 2), 0, CURSOR_SIZE, 10 * count, ILI9341_BLUE);
	if(int(millis() - time) / 1000 > 24){
		return false;
	}else{
		return true;
	}
}

// end of maze set up section. 










//2. game mode section, 

/*
 Function name: drawCursor() , subfunction for checkBlocks()
 definition: draw the current cursor red and replace it old one with blue 
*/
void drawCursor() {

	tft.fillRect(x * 10, y * 10, CURSOR_SIZE, CURSOR_SIZE, ILI9341_RED);
	tft.fillRect(oldx * 10, oldy * 10, CURSOR_SIZE, CURSOR_SIZE, ILI9341_BLUE);
}

/*
 Function name: checkBlocks(), subfunction for gameMode()
 definition: check if there is a block ahead of the moving direction, if there 
 is a block, cursor does not move.
*/
void checkBlocks(){
	int v = analogRead(JOY_VERT_ANALOG);
  	int h = analogRead(JOY_HORIZ_ANALOG);
  	
	if (x >= 1 && x <= c){ //constrasin the demain
		//up

		if (h - JOY_CENTRE > JOY_DEADZONE ) { 
			
			if(array[y][x - 1] == 0){
				oldx = x;
				oldy = y;
				x--;
				drawCursor();

			}

   		}
   		if (h - JOY_CENTRE < -JOY_DEADZONE){
   			if(array[y][x + 1] == 0){
				oldx = x;
				oldy = y;
				x++;
				drawCursor();

			}
   			
   		}

  	}

  	if (y >= 1 && y <= r){ //constrasin the demain
		//right
		if (v - JOY_CENTRE > JOY_DEADZONE ) { 
			if(array[y + 1][x] == 0){
				oldx = x;
				oldy = y;
				y++;
				drawCursor();

			}
        }
   		//left
   		if (v - JOY_CENTRE < -JOY_DEADZONE){
   			if(array[y - 1][x] == 0){
				oldx = x;
				oldy = y;
				y--;
				drawCursor();

			}
   		}
   	}
//  a time delay, otherwise it will be too hard to control	
	delay(100);
}

/*
 Function name: gameMode() 
 definition: start the time, draw the start and the end, 
 determine the wl value, so we know if you win or lose the game
*/

void gameMode(){
	
	
	tft.fillRect(10 , 10, CURSOR_SIZE, CURSOR_SIZE, ILI9341_RED); //starting piont
	tft.fillRect(10 * c, 10 * r ,CURSOR_SIZE, CURSOR_SIZE, ILI9341_YELLOW); //destination 
	for(int i = 0; i <= r + 1; i++){
		tft.fillRect(10 * (c + 2), 10 * i ,CURSOR_SIZE, CURSOR_SIZE, ILI9341_GREEN); // timer
	}
	time = millis();
	while(x != c || y != r){
	 
		if(!timer()){
			wl = 1; //lose 
			break;
		}
		

		checkBlocks(); 

	}
	if(x == c && y == r){
		wl = 0; //win 
	}

}

//end of  game mode section






//3.screen and Touch section
/*
  Touch Functions: touch_control(), touch_c1()
 definitions: determine what the touches do on the tft screens.  
*/

void touch_control(){
    while(true){
        TSPoint touch = ts.getPoint();
        
        if(touch.z > 0){
            if(touch.x >= 580 && touch.x <= 700 && touch.y >= 550 && touch.y <= 850){
                Serial.println("N");
                mode = 1;
                break;
            }else if(touch.x >= 580 && touch.x <= 700 && touch.y >= 140 && touch.y <= 440){
                Serial.println("E");
                mode = 0;
                break;
            }
        }
        
    }
}

void touch_c1(){
    while(true){
        TSPoint touch = ts.getPoint();
        
        if(touch.z > 0){
            if(touch.x >= 580 && touch.x <= 750 && touch.y >= 200 && touch.y <= 850){
                Serial.println("N");
                
                break;
            }
        }
        
    }
}

/*
 Screen Functions: startScreen(), lose(), win()
 definitions: draws a screen for each case.   
*/
void startScreen(){
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.setTextSize(4);
    tft.setCursor(30,0);
    tft.print("Maze Runner");
    tft.fillRect(60, 140, 200, 60, ILI9341_BLUE);

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    tft.setTextSize(3);
    tft.setCursor(80, 150);
    tft.print("PLay now");
}


void lose(){
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
    tft.setTextSize(5);
    tft.setCursor(40,0);
    tft.print("You LOSE");
    tft.fillRect(20, 140, 120, 40, ILI9341_BLUE);
    tft.fillRect(180, 140, 120, 40, ILI9341_BLUE);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    tft.setTextSize(3);
    tft.setCursor(25, 150);
    tft.print("Again");
    tft.setCursor(185, 150);
    tft.print("Exit");
}

void win(){
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.setTextSize(5);
    tft.setCursor(60,0);
    tft.print("You Win");
    tft.fillRect(20, 140, 120, 40, ILI9341_BLUE);
    tft.fillRect(180, 140, 120, 40, ILI9341_BLUE);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    tft.setTextSize(3);
    tft.setCursor(25, 150);
    tft.print("Again");
    tft.setCursor(185, 150);
    tft.print("Exit");
}

// end of screen and Touch section










// 4. combined main section

/*
  MAIN Function: main()
 definitions: used all the functions above, lay out the maze, check the movement, 
 returns win or lose screens. 
*/
int main(){
	
	// start the game, 
	setup();
	startScreen();  // show the 
	touch_c1(); // determine when the start communicating the maze arry and start the game
				

	while(true){
		
		//communication, and receive the array
		get_data();

		//print out the maze on the screen 
		gameMapSetup();
		//check the movements of the cursor and determine the win or lose 
        gameMode();
		if(wl){
			lose();// show the lose screen 
		}else{
			win();// show the win screen 
		}
        //user choices 
		touch_control();
        // if user decides to quit the game
		if(mode == 0){
			tft.fillScreen(ILI9341_BLACK);
			break;
		}
	}
	
	Serial.end();
	return 0;
}

// end of combined main section