/*
 * Copyright (c) Mnemonics Ltd
 *
 * androidAutIF_1.ino
 *
 * Rotary encoder sends:
 * 		'1' for left rotation
 *		'2' for right rotation
 * 		'up' for left rotation whilst nob pushed down
 * 		'down' for right rotation whilst nob pushed down
 * 		'enter' for the release of nob (as long as no rotations seen)
 * 		'esc' for the button
 *
 * Uses a leonardo equivalent Arduino Micro Pro (utilising keyboard and RotaryEncoder libraries) all open source
 * Rotary Encoder courtesy of Mathew Hertel
 *
 * The encode is jaycar 
 * jaycar.co.nz/digital-rotation-sensor-for-arduino/p/XC3736
 * has 100nF caps across the clk and det
 * 
 * ----------------------------------------------------------------------------------------------------
 * PIN 2 - has encoder click button (attached as interrupt on rising edge)
 * PIN 3 - encoder clk
 * PIN 4 - encoder detect
 * PIN 6 - extra ESC button 
 *
 * ----------------------------------------------------------------------------------------------------
 *
 *
 * @date  Mar 2021  
 *
 *
 */
#include <Keyboard.h>
#include <RotaryEncoder.h>

// Pin defs
#define ENC_DET 	4
#define ENC_CLK		3
#define ENC_BUT		2
#define BUT_ONE		6


// Setup a RotaryEncoder with 2 steps per latch for the 2 signal input pins:
RotaryEncoder encoder(ENC_CLK, ENC_DET, RotaryEncoder::LatchMode::TWO03);

// Keyboard chars
char upKey = KEY_UP_ARROW;
char downKey = KEY_DOWN_ARROW;
char leftRotate = '1';
char rightRotate = '2';
char enterKey = KEY_RETURN;
char escapeKey = KEY_ESC;

// Debounce timings
long debouncing_time = 120; //Debouncing Time in milliseconds
long debouncing_time_button_millis = 500;
volatile unsigned long last_micros;
volatile unsigned long last_micros_encButton;

// Storing states
byte wasSeenInRotation = false;

byte lastButtonSeen = false;
long lastButtonSeenTime = 0;

/**
 * Name: setup
 * 
 * Return: None
 * 
 * Parameters: None
 * 
 * Description: Runs on start-up
 *
 */
void setup()
{
	// Open comms to serial port
	Serial.begin(9600);       

	// Start Button First two are donw by library
	//pinMode(ENC_DET, INPUT_PULLUP);
	//pinMode(ENC_CLK, INPUT_PULLUP);
	pinMode(ENC_BUT, INPUT_PULLUP);
	pinMode(BUT_ONE, INPUT_PULLUP);
	

	attachInterrupt(digitalPinToInterrupt(ENC_BUT), encButton, RISING);
	
}
//================================================
//            IDLE LOOP
//================================================
void loop()
{
	// Check status of any buttons
	checkButtonStates();
	 
	 // Set up for rotary testing
	 static int pos = 0;
	 encoder.tick();
	 int direction;
	// Get the new position 
	int newPos = encoder.getPosition();
	
	// if there is - process
	if (pos != newPos) 
	{
		// get the direction
		direction = (int)(encoder.getDirection());
		
		//test if the ENC_BUT is down
		if ( !digitalRead(ENC_BUT) )
			wasSeenInRotation = true;
		else
			wasSeenInRotation = false;
	
		// logic for what Keyboard character to send
		if ( direction == 1 && wasSeenInRotation == false )
			Keyboard.press(leftRotate);
		if ( direction == 1 && wasSeenInRotation == true )
			Keyboard.press(upKey);
		if ( direction == -1 && wasSeenInRotation == false )
			Keyboard.press(rightRotate);
		if ( direction == -1 && wasSeenInRotation == true )
			Keyboard.press(downKey);
		
		// release it
		Keyboard.releaseAll();
		// update pos
		pos = newPos;
	} 
  
  
}
/**
 * Name: encButton
 * 
 * Return: None
 * 
 * Parameters: None
 * 
 * Description: Interrupt driven function
 *
 */
void encButton()
{
	if ( wasSeenInRotation )
	{
		last_micros_encButton = micros();
		wasSeenInRotation = false;
		return;
	}
	
	if((long)(micros() - last_micros_encButton) >= debouncing_time * 1000)
	{	
		Keyboard.press(enterKey);
		Keyboard.releaseAll();
		wasSeenInRotation = false;
		last_micros_encButton = micros();
	}

}
/**
 * Name: checkButtonStates
 * 
 * Return: None
 * 
 * Parameters: None
 * 
 * Description: Check if a button is being pressed
 *
 */
void checkButtonStates()
{
	long now = millis();
	
	if( now >= debouncing_time_button_millis + lastButtonSeenTime )
	{
		lastButtonSeen = false;
	}	

	if ( !digitalRead(BUT_ONE) && lastButtonSeen == false )
	{
		lastButtonSeen = true;
		lastButtonSeenTime = millis();
		Keyboard.press(escapeKey);
		Keyboard.releaseAll();
	}
	
}
