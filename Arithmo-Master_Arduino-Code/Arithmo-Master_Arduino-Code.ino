#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Include Library > LiquidCrystal I2C by Frank De Brabander
#include <Keypad.h>
// Include Library > Keypad by Mark Stanley, Alexander Brevig

#define I2C_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

LiquidCrystal_I2C lcd(I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'}, 
  {'7', '8', '9', '*'},
  {'B', '0', '=', '/'}
};

// //-> Arduino Uno
// byte rowPins[ROWS] = {2, 3, 4, 5}; /* connect to the row pinouts of the keypad */
// byte colPins[COLS] = {6, 7, 8, 9}; /* connect to the column pinouts of the keypad */
// //-> Arduino Nano
byte rowPins[ROWS] = {4, 5, 6, 7}; /* connect to the row pinouts of the keypad */
byte colPins[COLS] = {8, 9, 10, 11}; /* connect to the column pinouts of the keypad */

Keypad keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


enum Mode { OPTIONS, CALCULATOR, QUIZ, TABLE };
Mode currentMode = OPTIONS;

char key;
int num1 = 0;
int num2 = 0;
char operation = '+';
int expectedAnswer = 0;
String userAnswer = "";
bool quizActive = false;

String userInput = ""; // Declare userInput globally
int tableNumber = 0; // Store the table number

int numCorrect = 0;
int numQuestions = 0;
int questionsPerSet = 10; // Number of questions per quiz set

long preLoopTime = 0;
long preDelayLoopTime = 0;

void makeDelay(long stDelay) {
  preDelayLoopTime = millis();
  while (millis() - preDelayLoopTime <= stDelay) {
    checkKeyPress();
    delay(5);
  }
}

void setup() {
  // Serial.begin(115200);
  // Serial.println("\n------ Setup Begin ------");
  lcd.init();
  lcd.backlight();
  // Serial.println("------ Setup Done ------");
  displayModes();
}

void loop() {
  if(currentMode == OPTIONS) {
    displayModes();
  }
  checkKeyPress();
  // makeDelay(5);
}

int displayModesIndex = 0;
String displayModesOptions[] = {
  "    Welcome     ",
  "1. Calculator",
  "2. Quiz",
  "3. Tables",
  "Select Mode[1-3]",
  ""
};

void displayModes() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(displayModesOptions[displayModesIndex]);
  lcd.setCursor(0, 1);
  lcd.print(displayModesOptions[displayModesIndex + 1]);
  makeDelay(1000);
  displayModesIndex += 1;
  if(displayModesIndex == *(&displayModesOptions + 1) - displayModesOptions - 1){ displayModesIndex = 0; makeDelay(500);}
}

void checkKeyPress() {
  key = keypad.getKey();
  // if (key) {  
  //   Serial.print("Pressed -> ");
  //   Serial.println(key);
  //   makeDelay(1000);
  // }
  // long currLoopTime = millis();
  // Serial.println("RunTime [checkKeyPress] -> ["+String((double) (currLoopTime - preLoopTime) / 1000.0, 4) + "]");
  // // Serial.println("RunTime [checkKeyPress] -> ["+String.format("%.4f", (double) (currLoopTime - preLoopTime) / 1000.0)) + "]");
  // preLoopTime = currLoopTime;
  if (key)  handleKeyPress(key);
}

void handleKeyPress(char key) {
    if (key == 'B') {
      currentMode = OPTIONS;
      displayModes(); // Return to mode selection screen
    } else {
      if (currentMode == OPTIONS) {
        if (key == '1') {
          currentMode = CALCULATOR;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Calculator Mode:");
          clearInput();
        } else if (key == '2') {
          currentMode = QUIZ;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Quiz Mode:");
          generateQuizQuestion();
          handleQuiz("\0");
        } else if (key == '3') {
          currentMode = TABLE;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Table Mode:");
          lcd.setCursor(0, 1);
          lcd.print("Number:");
          userInput = "";
        }
      } else if (currentMode == CALCULATOR) {
        handleCalculator(key);
      } else if (currentMode == QUIZ) {
        handleQuiz(key);
      } else if (currentMode == TABLE) {
        handleTable(key);
      }
    }

  }
  
void handleCalculator(char key) {
  if (key) {
    if (key == '=') {
      calculateResult();
    } else if (key == 'C') {
      clearInput();
    } else if (key == 'B') {
      currentMode = OPTIONS;
      setup(); // Return to mode selection screen
    } else {
      userAnswer += key;
      displayInput();
    }
  }
}


void handleQuiz(char key) {
  if (!quizActive) {
    generateQuizQuestion();
    quizActive = true;
  } else if (key == '=') {
    // userAnswer += (" " + String(key) + " ");
    displayInput();
    checkAnswer();
  } else if (key == '-') {
    userAnswer += key;
    displayInput();
  } else if (isdigit(key)) {
    userAnswer += key;
    displayInput();
  } else if (key == 'B') {
    currentMode = OPTIONS;
    displayModes(); // Return to mode selection screen
  }
}

void handleTable(char key) {
  if (key) {
    if (key == '=') {
      tableNumber = userInput.toInt();
      printMultiplicationTable(tableNumber);
    } else if (isdigit(key)) {
      userInput += key;
      lcd.setCursor(0, 1);
      lcd.print("Number: " + userInput + "     ");
    } else if (key == 'B') {
      currentMode = OPTIONS;
      setup(); // Return to mode selection screen
    }
  }
}

void generateQuizQuestion() {
  // num1 = random(1, 11);
  // num2 = random(1, 11);
  num1 = (millis() / random(1, 11)) % 10;
  num2 = (millis() / random(1, 11)) % 10;
  int op = random(4);
  switch (op) {
    case 0:
      operation = '+';
      expectedAnswer = num1 + num2;
      break;
    case 1:
      operation = '-';
      expectedAnswer = num1 - num2;
      break;
    case 2:
      operation = '*';
      expectedAnswer = num1 * num2;
      break;
    case 3:
      operation = '/';
      expectedAnswer = num1 / num2;
      break;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(num1);
  lcd.print(" ");
  lcd.print(operation);
  lcd.print(" ");
  lcd.print(num2);
  lcd.setCursor(0, 1);
  lcd.print("Your answer:");

  userAnswer = "";
  displayInput();
}

void displayInput() {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(userAnswer);
}

void calculateResult() {
  if (userAnswer.length() == 0) {
    lcd.setCursor(0, 1);
    lcd.print("Error");
    makeDelay(1000);
    clearInput();
    return;
  }

  String result = userAnswer + "=" + evaluateExpression(userAnswer);

  lcd.setCursor(0, 1);
  lcd.print(result);

  makeDelay(2000);
  clearInput();
}

void clearInput() {
  userAnswer = "";
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

String evaluateExpression(String expr) {
  int result = 0;
  char operatorChar = '+';

  for (int i = 0; i < expr.length(); i++) {
    char currentChar = expr.charAt(i);

    if (isdigit(currentChar)) {
      int operand = currentChar - '0';
      if (operatorChar == '+') {
        result += operand;
      } else if (operatorChar == '-') {
        result -= operand;
      } else if (operatorChar == '*') {
        result *= operand;
      } else if (operatorChar == '/') {
        if (operand != 0) {
          result /= operand;
        } else {
          return "Error";
        }
      }
    } else if (currentChar == '+' || currentChar == '-' || currentChar == '*' || currentChar == '/') {
      operatorChar = currentChar;
    }
  }

  return String(result);
}

void checkAnswer() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(num1);
  lcd.print(" ");
  lcd.print(operation);
  lcd.print(" ");
  lcd.print(num2);
  lcd.print(" = ");
  lcd.print(expectedAnswer);
  lcd.setCursor(0, 1);
  lcd.print(userAnswer);
  lcd.print(" is ");
  if (userAnswer.toInt() == expectedAnswer) {
    numCorrect++;
    lcd.print("Correct!");
  } else {
    lcd.print("Wrong!    ");
  }
  numQuestions++;
  makeDelay(1000);
  if (numQuestions < questionsPerSet) {
    generateQuizQuestion();
  } else if (key == 'B') {
      currentMode = OPTIONS;
      displayModes();
  } else {
    // Display the score
    makeDelay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Your score: " + String(numCorrect) + "/" + String(questionsPerSet));
    makeDelay(2000);

    // Reset the quiz counters
    numCorrect = 0;
    numQuestions = 0;
    generateQuizQuestion();//reset the quiz questions
  }
}

void printMultiplicationTable(int numValue) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Table of " + String(numValue) + ":");

  for (int i = 1; i <= 10; i++) {
    lcd.setCursor(0, 1);
    lcd.print(String(numValue) + " x " + String(i) + " = " + String(numValue * i));
    makeDelay(600);
    if(currentMode == OPTIONS) return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter a number:");
  userInput = "";
}
