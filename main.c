/*
RobotC EV3 Culminating Project
      "The Boba Bot"
       Version 1.0

Joey Maillette, Ethan Ahn
Yuming He, Karthigan Uthayan
*/

// UI
int selectTable();
float selectDrink();
void EndMessage(bool paid);

// motion/control
void conditions();
bool tooClose();
void configureAllSensors();
void waitButton(TEV3Buttons button);
void motorsOn(int leftpower, int rightpower);
void motorsOff();
void driveDistCM(float dist, int angle);
int accelerate(int motorSpeed, int sliceTime, bool acc);
void gyroCorrection(int angle);
void rotateRobot(int motorSpeed, int angle);
void waitForCupIn();
void waitForCupOut();
void openDoor(bool cupIn);

// navigation
int driveToNode(string nextNode, string pos, int heading);
int driveToTable(string *table, int nodes);
void returnToStart(string *table, int nodes, int last_heading);
float getX(string node);
float getY(string node);

//tables
const string TABLE_1[3] = {"0 0", "0 300", "75 300"};
const string TABLE_2[3] = {"0 0", "0 300", "-75 300"};
const string TABLE_3[3] = {"0 0", "0 200", "75 200"};
const string TABLE_4[3] = {"0 0", "0 200", "-75 200"};
const string TABLE_5[3] = {"0 0", "0 100", "75 100"};
const string TABLE_6[3] = {"0 0", "0 100", "-75 100"};

// database
void initialiseDB();
void printDB();
void createLocal(float *localDB);
void postLocal(float *localDB);
bool readCard(float cost, float *balances);

// global constants
const int MOTOR_SPEED = 30;
const int ROTATE_SPEED = 15;
const int DOOR_SPEED = 20;
const int DOOR_ANGLE = 60;
const float CM_TO_DEG = 180 / (2.8 * PI);
const int TOO_CLOSE = 60;
const int ACCEL_FACTOR = 100;
const int MOTOR_LIMIT = 5;
const int CORRECTION_FACTOR = 1;
const int SLICE_TIME = 5;
const int NUM_ACCOUNTS = 6;
const int NUM_NODES = 3;
const int NUM_TABLES = 6;

// global constants for readability
const int LEFT_MOTOR = motorA;
const int RIGHT_MOTOR = motorD;
const int DOOR_MOTOR = motorB;
const int GYRO = S4;
const int ULTRASONIC = S2;
const int COLOR = S3;
const int CUP_TOUCH = S1;
const TEV3Buttons KILL_SWITCH = buttonUp;

// Main
task main()
{
	waitButton(buttonEnter);
	configureAllSensors();

  printDB();

	float balances[NUM_ACCOUNTS];
	createLocal(balances);

	int table = selectTable();
	float cost = selectDrink();
	string currentTable[NUM_NODES];
	int last_heading;

	openDoor(true);

	// sorry
	if (table==1)
	{
		last_heading = driveToTable(TABLE_1, NUM_NODES);
	}
	else if (table==2)
	{
		last_heading = driveToTable(TABLE_2, NUM_NODES);
	}
	else if (table==3)
	{
		last_heading = driveToTable(TABLE_3, NUM_NODES);
	}
	else if (table==4)
	{
		last_heading = driveToTable(TABLE_4, NUM_NODES);
	}
	else if (table==5)
	{
		last_heading = driveToTable(TABLE_5, NUM_NODES);
	}
	else
	{
		last_heading = driveToTable(TABLE_6, NUM_NODES);
	}

	bool paid = readCard(cost, balances);

	if(paid)
	{
		openDoor(false);
	}

	if (table==1)
	{
		returnToStart(TABLE_1, NUM_NODES, last_heading);
	}
	else if (table==2)
	{
		returnToStart(TABLE_2, NUM_NODES, last_heading);
	}
	else if (table==3)
	{
		returnToStart(TABLE_3, NUM_NODES, last_heading);
	}
	else if (table==4)
	{
		returnToStart(TABLE_4, NUM_NODES, last_heading);
	}
	else if (table==5)
	{
		returnToStart(TABLE_5, NUM_NODES, last_heading);
	}
	else
	{
		returnToStart(TABLE_6, NUM_NODES, last_heading);
	}

  EndMessage(paid);
  if (!paid)
  {
  	openDoor(false);
  }

	postLocal(balances);
	printDB();
}

// function configures all the sensors
void configureAllSensors()
{
  // reset all motor encoders
  nMotorEncoder[LEFT_MOTOR] = nMotorEncoder[RIGHT_MOTOR] =
  nMotorEncoder[DOOR_MOTOR] = 0;

  // sensor ports
  SensorType[GYRO] = sensorEV3_Gyro;
  SensorType[ULTRASONIC] = sensorEV3_Ultrasonic;
  SensorType[COLOR] = sensorEV3_Color;
  SensorType[CUP_TOUCH] = sensorEV3_Touch;
  wait1Msec(50);

  // color sensor initialization
  SensorMode[COLOR] = modeEV3Color_Color;
  wait1Msec(50);

  // gyro initialization
  SensorMode[GYRO] = modeEV3Gyro_Calibration;
  wait1Msec(100);
  SensorMode[GYRO] = modeEV3Gyro_RateAndAngle;
  wait1Msec(50);
  resetGyro(GYRO);
}

// function checks while-loop stopping conditions
// ie. kill switch or object detection
void conditions()
{
  // checks if the kill switch was pressed, end the program
  if (getButtonPress(KILL_SWITCH))
  {
    stopTask(main);
  }
}

bool tooClose()
{
  // if an object stopped the while-loop
  if (SensorValue(ULTRASONIC) <= TOO_CLOSE)
  {
  	playSound(soundException);
    return true;
  }
  // if the while-loop condition stopped the while-loop
  else
  {
    return false;
  }
}

// function to wait for a button to be pressed and released
void waitButton(TEV3Buttons button)
{
  // wait for it to be pressed
  while (!getButtonPress(button) && !getButtonPress(KILL_SWITCH))
  {}
	conditions();
  // wait for it to be released
  while (getButtonPress(button) && !getButtonPress(KILL_SWITCH))
  {}
	conditions();
}

// function that turns motors on at specific left and right speed
void motorsOn(int leftpower, int rightpower)
{
  motor[LEFT_MOTOR] = leftpower;
  motor[RIGHT_MOTOR] = rightpower;
}

// function that powers both motors off
void motorsOff()
{
	motor[LEFT_MOTOR] = motor[RIGHT_MOTOR] = 0;
}

// function that drives to a specific distance in cm
void driveDistCM(float dist, int angle)
{
	// for (0,0) nodes
	if (dist == 0)
	{
		return;
	}

  // reset the gyro before the distance is travelled to account for minor
  // angular drift
  nMotorEncoder[LEFT_MOTOR] = 0;
  resetGyro(GYRO);
	int start_angle = getGyroDegrees(GYRO)+angle;

  // stopping condition set to false
  bool recall_fnc = false;
  int decelerateDist = 0;

  // accelerate() accelerates the robot and returns the distance in motor
  // encoder ticks that it took to accelerate so we know when to start
  // decelerating at the same rate
  decelerateDist = accelerate(MOTOR_SPEED, SLICE_TIME, true);

  // while conditions: distance travelled so far < desired distance travelled
  // 									 there is
  // no object
  // kill switch isn't pressed
  time1[T1] = 0;
  while (nMotorEncoder[LEFT_MOTOR] < ((dist)*CM_TO_DEG - decelerateDist) && SensorValue(ULTRASONIC) > TOO_CLOSE && !getButtonPress(KILL_SWITCH))
  {
  			 gyroCorrection(-start_angle);
  }

  // stopping condition is set based on conditions()
  recall_fnc = tooClose();
  conditions();

  // to make sure motor values are same before deleceration phase
	motorsOn(MOTOR_SPEED, MOTOR_SPEED);

	// emergency stop
	if(recall_fnc)
	{
		motorsOff();
	}
	else
	{
		// decelerates the bot at the same rate it accelerated
		accelerate(MOTOR_SPEED, SLICE_TIME, false);
	}
  // stops the bot completely
  motorsOff();

  // if the reason it stopped was because there was an object detected
  if (recall_fnc)
  {
    // wait for the object to move
    while (SensorValue(ULTRASONIC) <= TOO_CLOSE && !getButtonPress(KILL_SWITCH))
    {}
    conditions();

    // recursively calls driveDistCM by the amount of distance the bot still
    // needs to travel
    driveDistCM(dist - nMotorEncoder[LEFT_MOTOR] / CM_TO_DEG, getGyroDegrees(GYRO));
  }
}

/*
function that controls the speed of the bot while in acceleration
or decceleration phases. (uses trigonometric acceleration)

sliceTime is the time between speed updates
acc is true for accelerating and false for deccelerating
*/
int accelerate(int motorSpeed, int sliceTime, bool acc)
{
  bool recall_fnc = false;
  float current_power = 0;
  float fraction = 0;

  recall_fnc = tooClose();
  if (recall_fnc)
  {
    while (SensorValue(ULTRASONIC) <= TOO_CLOSE && !getButtonPress(KILL_SWITCH))
    {}
    conditions();
  }
  // loops over a constant number of steps (set to 100 right now)
  for (int i = 0; i < ACCEL_FACTOR; i++)
  {
    // calculates the fraction of the motorSpeed the robot needs to be on during
    // this "step"
    fraction = (-1 * (cos(i * PI / ACCEL_FACTOR) + 1) / 2) + 1;

    // if it is accelerating
    if (acc)
    {
      current_power = motorSpeed * fraction;
      motorsOn(current_power,
              current_power+fraction); // +1 to compensate for motor imbalance
    }

    // if its deccelerating
    else
    {
      current_power = motorSpeed - (motorSpeed * fraction);
      motorsOn(current_power,
              current_power+(1-fraction));
    }


    time1[T2] = 0;

    // times each for-loop and checks if the kill switch is pressed
    while (time1[T2] < sliceTime && !getButtonPress(KILL_SWITCH))
    {}
    conditions();
    displayString(5, "%d", getGyroDegrees(GYRO));
  }

  // returns the distance travelled over acceleration phase in motor encoder
  // ticks
  return (nMotorEncoder[LEFT_MOTOR]);
}

void gyroCorrection(int angle)
{
	if (getGyroDegrees(GYRO) == angle)
	{
		motorsOn(MOTOR_SPEED, MOTOR_SPEED);
	}

	else if (getGyroDegrees(GYRO) < angle)
	{
		motorsOn(MOTOR_SPEED, MOTOR_SPEED-CORRECTION_FACTOR*abs(angle-getGyroDegrees(GYRO)));
	}

	else
	{
		motorsOn(MOTOR_SPEED-CORRECTION_FACTOR*abs(angle-getGyroDegrees(GYRO)), MOTOR_SPEED);
	}
}

// function smoothly rotates the robot over an angle from -180 to 180 (90 being
// right)
void rotateRobot(int motorSpeed, int angle)
{
	float current_power = 0;
  float fraction = 0;
  // loops over a constant number of steps (set to 100 right now)
  for (int i = 0; i < ACCEL_FACTOR; i++)
  {
    // calculates the fraction of the motorSpeed the robot needs to be on during
    // this "step"
    fraction = (-1 * (cos(i * PI / ACCEL_FACTOR) + 1) / 2) + 1;
    current_power = motorSpeed * fraction;

    // if turning left, left motor value negative, right motor value positive
    if (angle < 0)
    {
      // note: motor limit is used to ensure no speed below 1 is read as 0
      // (integer)
      motorsOn(current_power - motorSpeed - MOTOR_LIMIT,
               motorSpeed - current_power + MOTOR_LIMIT);
    }
    // if turning right, right motor value negative, left motor value positive
    else
    {
      motorsOn(motorSpeed - current_power + MOTOR_LIMIT,
               current_power - motorSpeed - MOTOR_LIMIT);
    }

    // each loop it turns the desired angle/the number of loops
    while (abs(getGyroDegrees(GYRO)) < (abs((float)angle / ACCEL_FACTOR) * (i + 1)) && !getButtonPress(KILL_SWITCH))
    {}
    conditions();
  }
  motorsOff();
}

// function that waits for the cup to be put in
void waitForCupIn()
{
  while (!SensorValue[CUP_TOUCH] && !getButtonPress(KILL_SWITCH))
  {}
  conditions();

  time1[T1] = 0;

  while (time1[T1] < 2000 && !getButtonPress(KILL_SWITCH))
  {}
  conditions();
}
// function that waits for the cup to be taken out
void waitForCupOut() {
  while (SensorValue[CUP_TOUCH] && !getButtonPress(KILL_SWITCH))
  {}
  conditions();

  time1[T1] = 0;
  while (time1[T1] < 1000 && !getButtonPress(KILL_SWITCH))
  {}
  conditions();
}

// function that controls the door movement based on if the cup is being
// inserted or removed
void openDoor(bool cupIn)
{
  nMotorEncoder[DOOR_MOTOR] = 0;
  motor[DOOR_MOTOR] = DOOR_SPEED;

  // opens the door
  while (nMotorEncoder[DOOR_MOTOR] < DOOR_ANGLE && !getButtonPress(KILL_SWITCH))
  {}
  conditions();
  motor[DOOR_MOTOR] = 0;

  // if a cup is being inserted, wait for it to be inserted
  if (cupIn) {
    waitForCupIn();
  }
  // if a cup is being removed, wait for it to be removed
  else
  {
    waitForCupOut();
  }

  motor[DOOR_MOTOR] = -DOOR_SPEED;

  // close the door
  while (nMotorEncoder[DOOR_MOTOR] > 0 && !getButtonPress(KILL_SWITCH))
  {}
  conditions();
  motor[DOOR_MOTOR] = 0;
}

// helper function to initialise the database in the EV3 brick's memory
void initialiseDB()
{
  const int MAX_SIZE = 7;
  float iBalances[MAX_SIZE] = {100.0, 97.5, 24.5, 37.6, 56.9, 45.6, 85.3};
  long fout = fileOpenWrite("db.txt");
  for (int i = 0; i < MAX_SIZE; i++)
  {
    fileWriteFloat(fout, iBalances[i]);
  }

  fileClose(fout);
}

// helper function to see what values are in the txt file
void printDB()
{
  const int MAX_SIZE = NUM_ACCOUNTS;
  long fin = fileOpenRead("db.txt");
  float balance = 0;
  for (int i = 0; i < MAX_SIZE; i++)
  {
    fileReadFloat(fin, &balance);
    displayString(i, "Account#: %d, Balance: %.2f", i, balance);
  }

  waitButton(buttonEnter);

  fileClose(fin);
  eraseDisplay();
}

// create local version of database by reading from file and writing to the
// array
void createLocal(float *localDB)
{
  const int SIZE = NUM_ACCOUNTS;
  long fin = fileOpenRead("db.txt");
  float balance = 0;
  for (int i = 0; i < SIZE; i++)
  {
    fileReadFloat(fin, &balance);
    localDB[i] = balance;
  }

  fileClose(fin);
}

// writes to the file from the local array
void postLocal(float *localDB)
{
  const int SIZE = NUM_ACCOUNTS;
  long fin = fileOpenWrite("db.txt");
  for (int i = 0; i < SIZE; i++)
  {
    fileWriteFloat(fin, localDB[i]);
  }

  fileClose(fin);
}

// reads card and processes payment
bool readCard(float cost, float *balances)
{
  displayString(4, "Price: %.2f", cost);
  displayBigTextLine(7, "INSERT CARD");
  while (SensorValue[COLOR] == (int)colorWhite && !getButtonPress(KILL_SWITCH))
  {}
	conditions();

  eraseDisplay();
  time1[T1] = 0;
  displayBigTextLine(5, "PROCESSING...");
  while (time1[T1] < 1000 && !getButtonPress(KILL_SWITCH))
  {}
	conditions();

  // read color value and check if account has sufficient funds to pay for drink
  int accountNum = SensorValue[COLOR] - 1;
  if (SensorValue[COLOR] == 7)
  {
  	accountNum = 5;
  }
  bool payed = true;

  eraseDisplay();
  if (balances[accountNum] - cost >= 0)
  {
    // charge account based on cost
    balances[accountNum] -= cost;

    displayString(2, "Account #%d:", accountNum);
    displayString(4, "New Balance: %.2f", balances[accountNum]);
    playSound(soundUpwardTones);
  }
  else
  {
    payed = false;
    displayBigTextLine(2, "No Funds");
    playSound(soundDownwardTones);
  }

  displayBigTextLine(6, "REMOVE CARD");
  while (SensorValue[COLOR] != (int)colorWhite && !getButtonPress(KILL_SWITCH))
  {}
	conditions();
  eraseDisplay();

  return payed;
}

int getX(string node)
{
	string new_node = node;
	int split1 = stringFind(new_node, " ");
	stringDelete(new_node, split1, strlen(new_node) - split1);
	return atoi(new_node);
}

int getY(string node)
{
	string new_node = node;
	int split1 = stringFind(new_node, " ");
	stringDelete(new_node, 0, split1);
	return atoi(new_node);
}

// drives to the a given node from a position and current heading
int driveToNode(string nextNode, string pos, int heading)
{
	// calculate drive distances and angle change
	int delta_x = getX(nextNode) - getX(pos);
	int delta_y = getY(nextNode) - getY(pos);
	int delta_angle = 0;
	int next_heading = 0;
	if (heading >= 0)
	{
		delta_angle = (180/PI)*(atan2(delta_x, delta_y)) - heading;
		next_heading = heading + delta_angle;
	} else {
		delta_angle = (180/PI)*(atan2(delta_x, abs(delta_y))) - heading;
		delta_angle *= -1;
		next_heading = heading + (-delta_angle);
	}

	if (delta_angle != 0)
	{
		rotateRobot(ROTATE_SPEED, delta_angle);
	}
	driveDistCM(abs(delta_x + delta_y), 0);

	return next_heading;
}

// drive to a given table
int driveToTable(string *table, int nodes)
{
	// initialise position and heading
	string pos = "0 0";
	int heading = 0;
	string nextNode;

	// navigate to each node in the path
	for (int i = 1; i < nodes; i++)
	{
		nextNode = *(table + i);
		heading = driveToNode(nextNode, pos, heading);
		pos = *(table + i);
	}

	return heading;
}

// drives to home from a given table
void returnToStart(string *table, int nodes, int last_heading)
{
	// initialises the start position as the last node in the table array
	int last_element = nodes - 1;
	string pos = *(table + last_element);

	int heading = last_heading;
	string nextNode;

	// navigates through the paths list backwards to return to home
	for (int i = last_element - 1; i >= 0; i--)
	{
		nextNode = *(table + i);
		heading = driveToNode(nextNode, pos, heading);
		pos = *(table + i);
	}

	rotateRobot(ROTATE_SPEED, 180);
}

int selectTable()
{
  // counter for table number (starts at 1)
  int table_num = 1;
  int MAX_TABLE = NUM_TABLES; // max amount of tables

  // displays string while the enter button is not pressed
  while (!getButtonPress(buttonEnter) && !getButtonPress(KILL_SWITCH))
  {
    // decreases table number
    if (getButtonPress(buttonLeft)) {
      // waits for user to let go of button and lowers table num
      while (getButtonPress(buttonLeft) && !getButtonPress(KILL_SWITCH))
      {}
      table_num--;
      eraseDisplay(); // erases previous num on display

      // goes to highest if they go less than 1
      // so table numbers are in range 1-6
      if (table_num < 1)
      {
        table_num = MAX_TABLE;
      }
    }

    // increases table number
    if (getButtonPress(buttonRight))
    {
      // waits for user to let go of button and increases num
      while (getButtonPress(buttonRight) &&  !getButtonPress(KILL_SWITCH))
      {}
      table_num++;
      eraseDisplay();

      // loops back to 1 if they go over the max amount of tables
      if (table_num > MAX_TABLE)
      {
        table_num = 1;
      }
    }

    // displays the the desired table number on Ev3 screen
    displayBigTextLine(3, "Table number %d", table_num);
  }

  // breaks loop if enter button is pressed
  // does nothing until released
  while (getButtonPress(buttonEnter) && !getButtonPress(KILL_SWITCH))
  {}

  eraseDisplay();

  // returns table num for payment
  return table_num;
}

/*allows user to select drink size
 returns float based on associated
  price of drink size */

float selectDrink()
{
  // assumes these are the only items in the menu
  string drinkMenu[] = {"small", "medium", "large"};
  float prices[] = {5.00, 5.50, 6.50}; // corresponding prices
  int menu_num = 0;                    // lowest index of array
  const int MAX_ITEM = 2;              // amount of menu items - 1

  while (!getButtonPress(buttonEnter) && !getButtonPress(KILL_SWITCH))
  {
    // decrements menu_num when right button pressed
    if (getButtonPress(buttonLeft))
    {
      while (getButtonPress(buttonLeft) && !getButtonPress(KILL_SWITCH))
      {}
      menu_num--;
      eraseDisplay();
      // if you go over the menu options
      if (menu_num < 0) {
        menu_num = MAX_ITEM; // goes to the last option
      }
    }

    // increments menu_num when right button pressed
    if (getButtonPress(buttonRight))
    {
      while (getButtonPress(buttonRight) && !getButtonPress(KILL_SWITCH))
      {}
      menu_num++;
      if (menu_num > MAX_ITEM) // if you go over the menu options
      {
        menu_num = 0; // goes back to the first option
      }
      eraseDisplay();
    }

    // displays the menu item on the screen
    displayBigTextLine(7, "Size: %s", drinkMenu[menu_num]);
    displayBigTextLine(10, "$%.2f", prices[menu_num]);
  }

  while (getButtonPress(buttonEnter) &&!getButtonPress(KILL_SWITCH))
  {}

  eraseDisplay();

  // returns the price of menu item
  return prices[menu_num];
}

void EndMessage(bool paid) //saved when paid or not
{
	if(paid == 0) //if not paid
	{
		displayString(5, "Not enouugh money");
	}
	else //paid
	{
		displayString(5, "Drink has been delivered");
	}
  waitButton(buttonEnter);
  eraseDisplay();
}
