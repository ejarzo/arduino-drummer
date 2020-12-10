 #include "Servo.h"
#include <Tasker.h>
#include <AceButton.h>
// #include <ArduinoSTL.h>

using namespace ace_button;

const int BEAT_PIN_1 = 4;
const int BEAT_PIN_2 = 3;
const int BEAT_PIN_3 = 2;

const int END_LOOP_PIN = 5;

const int SERVO_PIN_1 = 6;
const int SERVO_PIN_2 = 8;
const int SERVO_PIN_3 = 7;

const int OFF_POS = 0;
const int ON_POS = 10;

// ArduinoSTL::vector<int> v = {7, 5, 16, 8};

bool isRecording = false;
long timer;

float userBeatTimes1[16] = {};
int currentUserBeat1 = 0;
float userBeatTimes2[16] = {};
int currentUserBeat2 = 0;
float userBeatTimes3[16] = {};
int currentUserBeat3 = 0;

int userBeatLoopLength = 0;

int pattern1[] = {0, 0, 0, 0, 0, 0, 0, 0};
int pattern2[] = {0, 0, 0, 0, 0, 0, 0, 0};
int pattern3[] = {0, 0, 0, 0, 0, 0, 0, 0};

int stepDuration = 599;
int stepCount = 8;
int currentStep = 0;
bool isPlaying = false;

Tasker tasker(true);

Servo servoMotor1;
Servo servoMotor2;
Servo servoMotor3;

ButtonConfig tapBeatButton1Config;
AceButton tapBeatButton1(&tapBeatButton1Config);

ButtonConfig tapBeatButton2Config;
AceButton tapBeatButton2(&tapBeatButton2Config);

ButtonConfig tapBeatButton3Config;
AceButton tapBeatButton3(&tapBeatButton3Config);

ButtonConfig endLoopButtonConfig;
AceButton endLoopButton(&endLoopButtonConfig);

Servo motors[] = {servoMotor1, servoMotor2, servoMotor3};

/* ================================== */

void resetPos(int motorIndex)
{
  Serial.println("RESET POS");
  Servo motor = motors[motorIndex];
  motor.write(OFF_POS);
}

void triggerHit(int motorIndex)
{
  Serial.println("TRIGGER HIT");

  Servo motor = motors[motorIndex];
  motor.write(ON_POS);
  tasker.setTimeout(resetPos, 200, motorIndex);
}

void playStep()
{
  // Serial.println("step");
  if (!isPlaying)
    return;

  if (pattern1[currentStep])
    triggerHit(0);

  if (pattern2[currentStep])
    triggerHit(1);

  if (pattern3[currentStep])
    triggerHit(2);

  currentStep++;
  currentStep = currentStep % stepCount;
}

// ============ TRIGGER HIT ============
void handleTapBeatPress(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  uint8_t id = button->getId();
  Serial.print("id");
  Serial.println(id);

  if (eventType == AceButton::kEventPressed)
  {
    isRecording = true;
    if (id == 1)
    {
      triggerHit(0);
      userBeatTimes1[currentUserBeat1] = timer;
      currentUserBeat1++;
    }
    else if (id == 2)
    {
      triggerHit(1);
      userBeatTimes2[currentUserBeat2] = timer;
      currentUserBeat2++;
    }
    else if (id == 3)
    {
      triggerHit(2);
      userBeatTimes3[currentUserBeat3] = timer;
      currentUserBeat3++;
    }
  }
}

boolean isStepActive(float beatToQuantizeTo, float halfBeat, int currentUserBeat, float userBeatTimes[])
{
  for (int i = 0; i < currentUserBeat; i++)
  {
    int userBeatTime = userBeatTimes[i];
    if (beatToQuantizeTo - halfBeat < userBeatTime && beatToQuantizeTo + halfBeat > userBeatTime)
    {
      return true;
    }
  }
  return false;
}

// int holdDownTimer = 0;

// void incrementHoldDownTimer() {
//   holdDownTimer++;
// }

// ============ END LOOP ============
void handleEndLoopPress(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  if (eventType == AceButton::kEventLongPressed)
  {
    Serial.println("Long Press");
    isPlaying = !isPlaying;
  }

  if (eventType == AceButton::kEventReleased)
  {
    Serial.println("End loop");
    userBeatLoopLength = timer;
    isRecording = false;
    timer = 0;
    for (size_t i = 0; i < 8; i++)
    {
      pattern1[i] = 0;
      pattern2[i] = 0;
    }
    int newBeatDuration = userBeatLoopLength / 8;
    int halfBeat = newBeatDuration / 2;

    for (int i = 0; i < 8; i++)
    {
      int beatToQuantizeTo = newBeatDuration * i;

      int isActive1 = isStepActive(beatToQuantizeTo, halfBeat, currentUserBeat1, userBeatTimes1);
      pattern1[i] = isActive1;

      int isActive2 = isStepActive(beatToQuantizeTo, halfBeat, currentUserBeat2, userBeatTimes2);
      pattern2[i] = isActive2;

      int isActive3 = isStepActive(beatToQuantizeTo, halfBeat, currentUserBeat3, userBeatTimes3);
      pattern3[i] = isActive3;

      // Serial.println(pattern1[i]);
    }

    stepDuration = newBeatDuration;
    // Serial.println(newBeatDuration);

    currentStep = 0;
    currentUserBeat1 = 0;
    currentUserBeat2 = 0;
    currentUserBeat3 = 0;

    isPlaying = true;
    playStep();
    tasker.setInterval(playStep, stepDuration);
  }
}

void incrementTimer()
{
  if (isRecording)
  {
    timer++;
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(SERVO_PIN_1, OUTPUT);
  pinMode(SERVO_PIN_2, OUTPUT);
  pinMode(SERVO_PIN_3, OUTPUT);
  pinMode(BEAT_PIN_1, INPUT_PULLUP);
  pinMode(BEAT_PIN_2, INPUT_PULLUP);
  pinMode(BEAT_PIN_3, INPUT_PULLUP);
  pinMode(END_LOOP_PIN, INPUT_PULLUP);

  tapBeatButton1Config.setEventHandler(handleTapBeatPress);
  tapBeatButton2Config.setEventHandler(handleTapBeatPress);
  tapBeatButton3Config.setEventHandler(handleTapBeatPress);

  endLoopButtonConfig.setEventHandler(handleEndLoopPress);
  endLoopButtonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  endLoopButtonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  tapBeatButton1.init(BEAT_PIN_1, HIGH, 1);
  tapBeatButton2.init(BEAT_PIN_2, HIGH, 2);
  tapBeatButton3.init(BEAT_PIN_3, HIGH, 3);

  endLoopButton.init(END_LOOP_PIN, HIGH, 4);

  servoMotor1.attach(SERVO_PIN_1);
  servoMotor2.attach(SERVO_PIN_2);
  servoMotor3.attach(SERVO_PIN_3);

  resetPos(0);
  resetPos(1);
  resetPos(2);

  tasker.setInterval(incrementTimer, 1);
  // tasker.setInterval(playStep, stepDuration);
}

void loop()
{
  tapBeatButton1.check();
  tapBeatButton2.check();
  tapBeatButton3.check();
  endLoopButton.check();

  tasker.loop();
}
