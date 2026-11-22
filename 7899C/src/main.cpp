#include "vex.h"
using namespace vex;

competition Competition;
brain Brain;
controller Controller1;

// ---------------- PORTS ----------------
// If you change a port here, change the number in diagRows[] below too.
motor LeftFront(PORT7, ratio18_1, true);
motor LeftBack(PORT10, ratio18_1, true);
motor RightFront(PORT9, ratio18_1, false);
motor RightBack(PORT8, ratio18_1, false);
motor_group LeftDrive(LeftFront, LeftBack);
motor_group RightDrive(RightFront, RightBack);
motor LeftArm(PORT5, ratio18_1, true);
motor RightArm(PORT6, ratio18_1, false);
motor Claw(PORT4, ratio18_1, false);

// TODO: PORT3 IS A GUESS. The old code used PORT7, which is LeftFront.
// Set this to the port that the Toggle motor is connected to.
motor Toggle(PORT3, ratio18_1, false);

// ---------------- COMMAND TRACKING ----------------
// These hold the last command sent to each mechanism (percent).
// The diagnostic screen shows them.
int cmdLeft = 0;
int cmdRight = 0;
int cmdArm = 0;
int cmdClaw = 0;
int cmdToggle = 0;

// Send a command to the drive. 0 and 0 stops the drive.
void driveCmd(int leftPct, int rightPct)
{
  cmdLeft = leftPct;
  cmdRight = rightPct;
  if (leftPct == 0)
    LeftDrive.stop(brake);
  else
    LeftDrive.spin(forward, leftPct, percent);
  if (rightPct == 0)
    RightDrive.stop(brake);
  else
    RightDrive.spin(forward, rightPct, percent);
}

// Send a command to the arm. Positive raises. 0 holds the position.
void armCmd(int pct)
{
  cmdArm = pct;
  if (pct == 0)
  {
    LeftArm.stop(hold);
    RightArm.stop(hold);
  }
  else
  {
    LeftArm.spin(forward, pct, percent);
    RightArm.spin(forward, pct, percent);
  }
}

// Send a command to the claw. Positive closes. 0 holds the position.
void clawCmd(int pct)
{
  cmdClaw = pct;
  if (pct == 0)
    Claw.stop(hold);
  else
    Claw.spin(forward, pct, percent);
}

void flip(void)
{
  Toggle.spinFor(forward, 180, degrees, false);
}

// ---------------- DIAGNOSTIC SCREEN ----------------
// One row for each motor:
//   name, port, plug status, control, command (c), measured speed (v)
// "OK" = the Brain sees a motor on that port. "NONE" = no motor found.
struct DiagRow
{
  const char *name;
  int port;
  const char *control;
  motor *m;
  int *cmd;
};

DiagRow diagRows[] = {
    {"LeftFront", 7, "Axis3/1", &LeftFront, &cmdLeft},
    {"LeftBack", 10, "Axis3/1", &LeftBack, &cmdLeft},
    {"RightFront", 9, "Axis3/1", &RightFront, &cmdRight},
    {"RightBack", 8, "Axis3/1", &RightBack, &cmdRight},
    {"LeftArm", 5, "R1/R2", &LeftArm, &cmdArm},
    {"RightArm", 6, "R1/R2", &RightArm, &cmdArm},
    {"Claw", 4, "L1/L2", &Claw, &cmdClaw},
    {"Toggle", 3, "A", &Toggle, &cmdToggle}, // TODO: match the Toggle port
};
const int DIAG_COUNT = sizeof(diagRows) / sizeof(diagRows[0]);

int diagTask()
{
  Brain.Screen.clearScreen();
  while (true)
  {
    // Toggle uses spinFor, so read its state from the motor.
    cmdToggle = Toggle.isSpinning() ? 100 : 0;

    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("MOTOR      PORT PLUG CONTROL   CMD  VEL");

    for (int i = 0; i < DIAG_COUNT; i++)
    {
      DiagRow &r = diagRows[i];
      Brain.Screen.setCursor(i + 2, 1);
      Brain.Screen.print("%-10s P%-2d  %-4s %-7s c%4d v%4d",
                         r.name,
                         r.port,
                         r.m->installed() ? "OK" : "NONE",
                         r.control,
                         *r.cmd,
                         (int)r.m->velocity(percent));
    }

    // Raw controller inputs, so you can see that the controller sends data.
    Brain.Screen.setCursor(DIAG_COUNT + 3, 1);
    Brain.Screen.print("Ax3%4d Ax1%4d R1%d R2%d L1%d L2%d A%d",
                       (int)Controller1.Axis3.position(),
                       (int)Controller1.Axis1.position(),
                       (int)Controller1.ButtonR1.pressing(),
                       (int)Controller1.ButtonR2.pressing(),
                       (int)Controller1.ButtonL1.pressing(),
                       (int)Controller1.ButtonL2.pressing(),
                       (int)Controller1.ButtonA.pressing());

    wait(100, msec);
  }
  return 0;
}

// ---------------- COMPETITION ----------------
void pre_auton(void)
{
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
}

void autonomous(void)
{
  // NOTE: negative lowers the arm (same direction as R2).
  // If the arm must go up here, change -40 to 40.
  armCmd(-40);
  wait(1, sec);
  armCmd(0);

  // Drive forward
  driveCmd(22, 22);
  wait(1, sec);
  driveCmd(0, 0);
  wait(0.5, sec);

  // Turn left
  driveCmd(-11, 11);
  wait(1.2, sec);
  driveCmd(0, 0);
  wait(0.2, sec);

  // Drive forward
  driveCmd(22, 22);
  wait(1, sec);
  driveCmd(0, 0);
  wait(0.3, sec);

  // Raise the arm slowly, then hold
  armCmd(10);
  wait(0.5, sec);
  armCmd(0);
}

void usercontrol(void)
{
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

    // Drift correction when the robot drives straight
    if (turnVal == 0)
    {
      leftPower = (int)(leftPower * 0.85);
    }

    driveCmd(leftPower, rightPower);

    // ARM: R1 raises, R2 lowers
    if (Controller1.ButtonR1.pressing())
      armCmd(40);
    else if (Controller1.ButtonR2.pressing())
      armCmd(-40);
    else
      armCmd(0);

    // CLAW: L2 closes, L1 opens
    if (Controller1.ButtonL2.pressing())
      clawCmd(30);
    else if (Controller1.ButtonL1.pressing())
      clawCmd(-30);
    else
      clawCmd(0);

    wait(20, msec);
  }
}

int main()
{
  // Register the button one time only.
  Controller1.ButtonA.pressed(flip);

  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  pre_auton();

  // Start the diagnostic screen.
  task diag(diagTask);

  while (true)
  {
    wait(100, msec);
  }
}
