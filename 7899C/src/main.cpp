#include "vex.h"
using namespace vex;
competition Competition;
brain Brain;
controller Controller1;
motor LeftFront(PORT7, ratio18_1, true);
motor LeftBack(PORT10, ratio18_1, true);
motor RightFront(PORT9, ratio18_1, false);
motor RightBack(PORT8, ratio18_1, false);
motor_group LeftDrive(LeftFront, LeftBack);
motor_group RightDrive(RightFront, RightBack);
motor LeftArm(PORT5, ratio18_1, true);
motor RightArm(PORT6, ratio18_1, false);
motor Claw(PORT4, ratio18_1, false);
motor Toggle(PORT7, ratio18_1, false);

void flip(void) {
  Toggle.spinFor(forward, 180, degrees, false);
}
void pre_auton(void)
{
  Brain.Screen.clearScreen();
  Brain.Screen.print("Initializing robot...\n");
  LeftDrive.setStopping(brake);
  RightDrive.setStopping(brake);
  LeftArm.setStopping(hold);
  RightArm.setStopping(hold);
  Claw.setStopping(hold);
  Toggle.setStopping(hold);
  LeftFront.setMaxTorque(100, percent);
  LeftBack.setMaxTorque(100, percent);
  RightFront.setMaxTorque(100, percent);
  RightBack.setMaxTorque(100, percent);
  LeftArm.setMaxTorque(60, percent);
  RightArm.setMaxTorque(60, percent);
  Claw.setMaxTorque(50, percent);
  Brain.Screen.clearScreen();
  wait(1, sec);
}
void autonomous(void)
{
  LeftArm.spin(reverse, 40, percent);
  RightArm.spin(reverse, 40, percent);
  wait(1, sec);
  LeftArm.stop(hold);
  RightArm.stop(hold);
  LeftDrive.spin(forward, 22, percent);
  RightDrive.spin(forward, 22, percent);
  wait(1, sec);
  LeftDrive.stop(brake);
  RightDrive.stop(brake);
  wait(0.5, sec);
  LeftDrive.spin(reverse, 11, percent);
  RightDrive.spin(forward, 11, percent);
  wait(1.2, sec);
  LeftDrive.stop(brake);
  RightDrive.stop(brake);
  wait(0.2, sec);
  LeftDrive.spin(forward, 22, percent);
  RightDrive.spin(forward, 22, percent);
  wait(1, sec);
  LeftDrive.stop(brake);
  RightDrive.stop(brake);
  wait(0.2, sec);
  LeftArm.stop(brake);
  RightArm.stop(brake);
  wait(0.1,sec);
  LeftArm.spin(forward, 10, percent);
  RightArm.spin(forward, 10, percent);
  wait(0.5, sec);
  LeftArm.stop(brake);
  RightArm.stop(brake);
}
void usercontrol(void)
{
  Controller1.ButtonA.pressed(flip); 

  while (true)
  {
    float speed = 0.5;
    int forwardVal = Controller1.Axis3.position() * speed;
    int turnVal = Controller1.Axis1.position() * speed;

    if (forwardVal > -5 && forwardVal < 5)
      forwardVal = 0;

    if (turnVal > -5 && turnVal < 5)
      turnVal = 0;

    int leftPower = forwardVal + turnVal;
    int rightPower = forwardVal - turnVal;

    if (turnVal == 0)
    {
      leftPower = (int)(leftPower * 0.85);
    }

    LeftDrive.spin(forward, leftPower, percent);
    RightDrive.spin(forward, rightPower, percent);

    // ARM: L1 raises, L2 lowers
    if (Controller1.ButtonR1.pressing())
    {
      LeftArm.spin(forward, 40, percent);
      RightArm.spin(forward, 40, percent);
    }
    else if (Controller1.ButtonR2.pressing())
    {
      LeftArm.spin(reverse, 40, percent);
      RightArm.spin(reverse, 40, percent);
    }
    else
    {
      LeftArm.stop(hold);
      RightArm.stop(hold);
    }

    // CLAW: L2 closes, L1 opens
    if (Controller1.ButtonL2.pressing())
    {
      Claw.spin(forward, 30, percent);
    }
    else if (Controller1.ButtonL1.pressing())
    {
      Claw.spin(reverse, 30, percent);
    }
    else
    {
      Claw.stop(hold);
    }
    wait(20, msec);
  }
}
int main()
{
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  pre_auton();

  while (true)
  {
    wait(100, msec);
  }
}