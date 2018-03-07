/*-----------------------------------------------------------------------------------------------------------
dbBoardsBrainGames.ino

  Summary:
    This program uses photocells as touch screen buttons by comparing the brightness over all three
    "buttons." The DB Brain Game is assentially a one player digital version of the classic board game Master
    Mind. The program randomly selects one of 5 colors for the three locations. Hovering over or touching the
    photocells will progress the three positions in your guess. Once you have chosen the last color, the set
    slides up the recent answers bar. The results are shown to the right where green LEDs represent a correct
    color in the correct position and red LEDs represent a correct color in the wrong position. When a round
    is completed, the score remains on the board so the user can challenge others to beat their score.
  
  Utilizing:
    Adafruit NeoPixel Library: https://github.com/adafruit/Adafruit_NeoPixel
   
  Programmer:
    Duncan Brandt @ DB Boards, LLC
    Created: March 5, 2018
  
  Development Environment Specifics:
    Arduino 1.6.11
  
  Hardware Specifications:
    Arduino Drawing Board (UNO) DB1000
    WS2812 Strip 10 DB7001
    Mini-Photocells DB7011

  Beerware License:
    This program is free, open source, and public domain. The program is distributed as is and is not
    guaranteed. However, if you like the code and find it useful I would happily take a beer should you 
    ever catch me at the local.
*///---------------------------------------------------------------------------------------------------------
#include <Adafruit_NeoPixel.h>                  // https://github.com/adafruit/Adafruit_NeoPixel
#define PIN 13                                  // Pin 13 for communication with LED strip
// 75  Number of pixels in strip, NEO_GRB  Pixels are wired for GRB bitstream, NEO_KHZ800  800 KHz bitstream
Adafruit_NeoPixel strip = Adafruit_NeoPixel(75, PIN, NEO_GRB + NEO_KHZ800);
//-----------------------------------------------------------------------------------------------------------
// Booleans are used to record the current state of the game
boolean game = false, guessing = false, victory = false;
int button = 0, pick = 0, solveCnt = 0;         // These variables are used to track the photocells
float lightDifference = .15;                    // The % difference of brightness required to trigger button
int win1 = 0, win2 = 0, win3 = 0;               // These store the three winning answers
int b1 = 0, b2 = 0, b3 = 0;                     // Used to count the length the buttons are pressed
int guess1 = 0, guess2 = 0, guess3 = 0;         // The users guess is stored in these three variables
int guesses = 0;                                // Keeps track of how many guesses have been made
//-----------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------
void setup() {
  strip.begin();                                // Start the LED strip
  strip.setBrightness(20);                      // Turn down the brightness
  strip.show();                                 // Turn off all the LEDs
  randomSeed(analogRead(4));                    // Creat a random sequnce from a floating analog pin
}
//-----------------------------------------------------------------------------------------------------------
void loop() {
  if(!game){                                    // If there is not a current game being played
    while(button < 1) button = touchCompare();  // Wait for one of the buttons to be pressed
    setupGame();                                // Then create a new game and winning sequence
  }
  else{                                         // If the game has started
    if(guessing) displayUpdate();               // If the player is mid-guess, update the guess display
    else runAndCheck();                         // If they have completed a guess, slide and check the answer
    if(victory) showScore();                    // If the player wins or times out, show guess count
  }
}
//-----------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------
void displayUpdate(){                           // Check the buttons and update the guess display
  button = touchCompare();                      // Which button is being pressed?
  pick = 0;                                     // Reset the guess counter
  if(guess1 > 0) pick++;                        // Add one if the user has selected guess color one
  if(guess2 > 0) pick++;                        // Add one if the user has selected guess color two
  if(guess3 > 0) pick++;                        // Add one if the user has selected guess color three
  if(button == 0 && pick == 0) delay(5);        // If the user has not made a selection, take a small break
  else{                                         // If the user has started guessing
    if(button == 0 && pick == 3) solveCnt++;    // If they have finished guessing, add to the mistake timer
    else{                                       // If they are mid-guess
      delay(1);                                 // Small delay for color change timing
      if(button == 1) b1++;                     // If the first button is pressed add to the color counter
      if(button == 2) b2++;                     // If the second button is pressed add to the color counter 
      if(button == 3) b3++;                     // If the third button is pressed add to the color counter
      if(b1 == 600){                            // If the button was held for 600 milliseconds
        b1 = 0;                                 // Restart the color counter
        if(guess1 < 5) guess1++;                // If we aren't at the last color, go to the next color
        else guess1 = 1;                        // Otherwise start at the first color again
        showPick(1, guess1);                    // Update the color on the first guess location
      }
      if(b2 == 600){                            // If the button was held for 600 milliseconds   
        b2 = 0;                                 // Restart the color counter
        if(guess2 < 5) guess2++;                // If we aren't at the last color, go to the next color
        else guess2 = 1;                        // Otherwise start at the first color again
        showPick(2, guess2);                    // Update the color on the first guess location
      }
      if(b3 == 600){                            // If the button was held for 600 milliseconds
        b3 = 0;                                 // Restart the color counter
        if(guess3 < 5) guess3++;                // If we aren't at the last color, go to the next color
        else guess3 = 1;                        // Otherwise start at the first color again
        showPick(3, guess3);                    // Update the color on the first guess location
      }
    }
    if(solveCnt > 5000){                        // If the user's final answer has remained unchanged
      solveCnt = 0;                             // Restart the timer
      guessing = false;                         // Complete the guessing cycle for this round
    }
  }
}
//-----------------------------------------------------------------------------------------------------------
void showPick(int btn, int color){              // Update the color of the selected guess button
  int pickLoc[] = {64, 65, 74, 62, 67, 72, 60, 69, 70}; // These are the strip locations of the guess LEDs
  for(int i = 0; i < 3; i++){                   // For the three guess LEDs in the selection
    strip.setPixelColor(pickLoc[((btn-1)*3) + i], chosenColor(color)); // Update to the new guess color
  }
  strip.show();                                 // Show the new color on the strip
  solveCnt = 0;                                 // Restart the mistake timer
}
//-----------------------------------------------------------------------------------------------------------
uint32_t chosenColor(int color){                // Uses the stored RGB values of the color options
  int colors[] = {0, 0, 0, 0, 0, 255, 133, 133, 0, 0, 255, 0, 85, 85, 85, 255, 0, 0};
  return strip.Color(colors[color*3], colors[color*3+1], colors[color*3+2]); // For use as color in setPixel
}
//-----------------------------------------------------------------------------------------------------------
void setupGame(){                               // Create new game
  game = true;                                  // Record start of game
  guessing = true;                              // Ready for users guess entries
  solveCnt = 0;                                 // Restart mistake timer
  win1 = int(random(1, 6));                     // Creat and record random color for first answer position
  win2 = int(random(1, 6));                     // Creat and record random color for second answer position
  win3 = int(random(1, 6));                     // Creat and record random color for third answer position
}
//-----------------------------------------------------------------------------------------------------------
int touchCompare(){                             // Check the photocells for a button press
  int btn1 = analogRead(0);                     // Record the first buttons "brightness" level
  int btn2 = analogRead(1);                     // Record the second buttons "brightness" level
  int btn3 = analogRead(2);                     // Record the third buttons "brightness" level
  if(btn1 < ((btn2+btn3)/2)-(((btn2+btn3)/2)*lightDifference)){
    return 1;                                   // The first cell is darker then the average of other two
  }
  else if(btn2 < ((btn1+btn3)/2)-(((btn1+btn3)/2)*lightDifference)){
    return 2;                                   // The second cell is darker then the average of other two
  }
  else if(btn3 < ((btn2+btn1)/2)-(((btn2+btn1)/2)*lightDifference)){
    return 3;                                   // The third cell is darker then the average of other two
  }
  else{                                         // If no button is being pressed
    b1 = 0;                                     // Clear the color timer for button 1
    b2 = 0;                                     // Clear the color timer for button 2
    b3 = 0;                                     // Clear the color timer for button 3
    return 0;                                   // Return 0 to show no button is pressed
  }
}
//-----------------------------------------------------------------------------------------------------------
void runAndCheck(){                             // Slide the answer up the answers bar and check the results
  // These arrays are the LED strip locations for the answer and result bars
  int ansLoc[] = {0,19,20,1,18,21,2,17,22,3,16,23,4,15,24,5,14,25,6,13,26,7,12,27,8,11,28,9,10,29};
  int resultsLoc[] = {39,40,59,38,41,58,37,42,57,36,43,56,
  35,44,55,34,45,54,33,46,53,32,47,52,31,48,51,30,49,50};
  for(int a = 9; a > guesses; a--){             // Starting at the bottom of the answers list step to current
    strip.setPixelColor(ansLoc[a*3], chosenColor(guess1));
    strip.setPixelColor(ansLoc[a*3+1], chosenColor(guess2));
    strip.setPixelColor(ansLoc[a*3+2], chosenColor(guess3));
    strip.show();                               // Show answer on current anser block
    delay(200);
    strip.setPixelColor(ansLoc[a*3], chosenColor(0));
    strip.setPixelColor(ansLoc[a*3+1], chosenColor(0));
    strip.setPixelColor(ansLoc[a*3+2], chosenColor(0));
    strip.show();                               // Turn off block to move answers to next block
  }
  strip.setPixelColor(ansLoc[guesses*3], chosenColor(guess1));
  strip.setPixelColor(ansLoc[guesses*3+1], chosenColor(guess2));
  strip.setPixelColor(ansLoc[guesses*3+2], chosenColor(guess3));
  strip.show();                                 // Save the answer in its actuall guess location
  int ans[] = {win1,win2,win3};                 // Save winners for comparrison
  int guess[] = {guess1,guess2,guess3};         // Save guesses for comparrison
  int perf = 0, near = 0;                       // Reset result counts
  for(int b = 0; b < 3; b++){                   // Check each guess position
    if(ans[b] == guess[b]){                     // Is it a perfect match to that position in the answer
      perf++;                                   // Add one to perfect count
      ans[b] = 0;                               // Clear the answer so it is not used again
      guess[b] = 0;                             // Clear the guess so it is not used again
    }
  }
  for(int c = 0; c < 3; c++){                   // For all three guesses
    if(guess[c] > 0){                           // If the guess was not already perfect
      if(guess[c] == ans[0]){                   // Check to see if it matches the first position
        near++;                                 // If it does add a near answer point
        ans[0] = 0;                             // Clear the answer so it is not used again
      }
      else if(guess[c] == ans[1]){              // Check to see if it matches the second position
        near++;                                 // If it does add a near answer point
        ans[1] = 0;                             // Clear the answer so it is not used again
      }
      else if(guess[c] == ans[2]){              // Check to see if it matches the third position
        near++;                                 // If it does add a near answer point
        ans[2] = 0;                             // Clear the answer so it is not used again
      }
    }
  }
  if(perf == 3) victory = true;                 // If all three guesses are perfect, user wins
  else{
    guessing = true;                            // If they have not won, they may select a new guess
  }
  guess1 = 0;                                   // Clear the last guess one
  guess2 = 0;                                   // Clear the last guess two
  guess3 = 0;                                   // Clear the last guess three
  for(int c = 0; c < 3; c++){                   // Step through the three result positions
    if(perf > 0){                               // If there is another perfect answer in the guesses
      strip.setPixelColor(resultsLoc[guesses*3+c], strip.Color(0,255,0));
      perf--;                                   // Show the green led in results and delete the count
    }
    else if(near > 0){                          // If we are out of perfects and have near answers
      strip.setPixelColor(resultsLoc[guesses*3+c], strip.Color(255,0,0));
      near--;                                   // Show the red led in results and delete the count
    }    
  }
  if(guesses == 10) victory = true;             // If the user has hit ten guesses, they are done
  if(!victory) guesses++;                       // If the game is not over, add one to the total guesses
  for(int e = 60; e< 75; e++) strip.setPixelColor(e, chosenColor(0));
  strip.show();                                 // Clear current guess display
  delay(1000);                                  // Wait one second in case the user gets to see a victoy set
}
//-----------------------------------------------------------------------------------------------------------
void showScore(){
  for(int e = 0; e< 60; e++) strip.setPixelColor(e, chosenColor(0)); // clear display
  // This array stores the LED strip locations of the number characters from 1-9 and X for 10 guesses
  int scores[] = {30,31,32,33,34,35,36,37,38,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,2,9,10,11,18,21,27,
  29,30,34,38,41,44,49,50,56,57,9,9,9,9,9,9,9,2,8,10,14,18,21,25,29,30,34,38,41,45,49,51,52,53,55,56,57,2,2,
  2,2,1,2,3,4,5,14,25,34,41,42,43,44,45,46,47,48,49,54,2,2,2,2,2,2,1,2,3,4,5,9,10,14,18,21,25,29,30,34,38,41,
  45,49,51,52,53,58,1,1,2,3,4,5,6,7,8,10,14,18,21,25,29,30,34,38,41,45,49,51,52,53,57,2,1,2,18,21,30,31,32,
  38,41,44,45,46,56,57,58,2,2,2,2,2,2,2,2,2,2,3,4,6,7,8,10,14,18,21,25,29,30,34,38,41,45,49,51,52,53,55,56,
  57,2,3,4,8,10,14,18,21,25,29,30,34,38,41,45,49,51,52,53,54,55,56,57,2,1,2,8,9,12,16,24,26,33,35,43,47,50,
  51,57,58,2,2,2,2,2,2,2,2};
  for(int f = 1; f < 6; f++){                   // Cycle through all the color choices
    for(int d = 0; d < 24; d++) strip.setPixelColor(scores[guesses*24+d], chosenColor(f));
    strip.show();                               // Display the number of guesses in the last game
    delay(2000);                                // Wait two seconds before changing the color again
  }
  for(int e = 0; e< 60; e++) strip.setPixelColor(e, chosenColor(0)); // Clear the LEDs for the next game
  guesses = 0;                                  // Clear the guesses count
  guessing = false;                             // Turn off guessing mode
  victory = false;                              // Reset victory mode
  game = false;                                 // Reset game mode to wait for next button press
}
//-----------------------------------------------------------------------------------------------------------
