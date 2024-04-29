#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Include Library > LiquidCrystal I2C by Frank De Brabander
#include <Keypad.h>
// Include Library > Keypad by Mark Stanley, Alexander Brevig
#include <ESP8266TimerInterrupt.h>
// Include Library > ESP8266TimerInterrupt by Khoi Hoang

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

byte rowPins[ROWS] = {D1, D2, D3, D4};
byte colPins[COLS] = {D5, D6, D7, D8};

Keypad keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

ESP8266Timer ITimer;

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
    timerInterrupt();
    delay(10);
  }
}

void setup() {
//   Serial.begin(115200);
//   Serial.println("\n------ Setup Begin ------");
  lcd.init();
  lcd.backlight();
  lcd.clear();

  String options[3] = {
    "1. Calculator",
    "2. Quiz",
    "3. Tables Print"
  };

  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  makeDelay(1000);
  for (int i = 0; i < 3; i++) {
    lcd.setCursor(0, 0);
    lcd.print(options[i]);
    makeDelay(1000);
    lcd.clear();
  }
  lcd.setCursor(0, 0);
  lcd.print(options[2]);
  
  ITimer.attachInterruptInterval(10000, timerInterrupt);
  ITimer.enableTimer();
//   Serial.println("------ Setup Done ------");
}

void loop() {
}

void timerInterrupt() {
  key = keypad.getKey();
  if (key) {  
    // Serial.printf("Pressed -> [%c]\n",key); 
    // makeDelay(500);
  }
  long currLoopTime = millis();
//   Serial.printf("RunTime [timerInterrupt] -> [%.6lf]\n",(double) (currLoopTime - preLoopTime) / 1000.0);
  preLoopTime = currLoopTime;
  if (key) {
    handleKeyPress(key);
  }
}

void handleKeyPress(char key) {
    if (key == 'B') {
      currentMode = OPTIONS;
      setup(); // Return to mode selection screen
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
        } else if (key == '3') {
          currentMode = TABLE;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Table Mode:");
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
    checkAnswer();
  } else if (isdigit(key)) {
    userAnswer += key;
    displayInput();
  } else if (key == 'B') {
    currentMode = OPTIONS;
    setup(); // Return to mode selection screen
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
  num1 = random(1, 11);
  num2 = random(1, 11);

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
  lcd.setCursor(12, 1);
  lcd.print("      ");
  lcd.setCursor(12, 1);
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

  String result = evaluateExpression(userAnswer);

  lcd.setCursor(0, 1);
  lcd.print("Result: " + result);

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
  if (userAnswer.toInt() == expectedAnswer) {
    numCorrect++;
    lcd.setCursor(0, 1);
    lcd.print("    Correct!    ");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("    Wrong!    ");
  }
  numQuestions++;
  makeDelay(1000);
  if (numQuestions < questionsPerSet) {
    generateQuizQuestion();
  } else if (key == 'B') {
      currentMode = OPTIONS;
      setup(); 
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
  }

  makeDelay(100);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter a number:");
  userInput = "";
}
