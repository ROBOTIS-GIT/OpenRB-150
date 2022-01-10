#include <DynamixelSDK.h>


// Control table address
#define ADDRESS_PRESENT_POSITION       132
#define LENGTH_PRESENT_POSITION         4

// Protocol version
#define PROTOCOL_VERSION                2.0                 // See which protocol version is used in the Dynamixel

// Default setting
#define DXL1_ID  1
#define DXL2_ID  2
#define DXL3_ID  3
#define DXL4_ID  4
#define DXL5_ID  200  // OpenCM9.04
#define BAUDRATE                        2000000
#define DEVICENAME                      "OpenCR_DXL_Port"   // This definition only has a symbolic meaning and does not affect to any functionality

#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque

#define CMD_SERIAL                      Serial

uint16_t tx_trial_count = 0;
uint16_t rx_trial_count = 0;
uint16_t tx_success_count = 0;
uint16_t rx_success_count = 0;
uint16_t loop_count = 0;
unsigned long time_begin = 0;
unsigned long time_temp = 0;
uint8_t param_goal_position[4];
int32_t dxl1_present_position = 0;
int32_t dxl2_present_position = 0;
int32_t dxl3_present_position = 0;
int32_t dxl4_present_position = 0;
int32_t dxl5_present_position = 0;

int getch()
{
  while(1)
  {
    if( CMD_SERIAL.available() > 0 )
    {
      break;
    }
  }

  return CMD_SERIAL.read();
}

int kbhit(void)
{
  return CMD_SERIAL.available();
}

void setup()
{
  Serial.begin(115200);
  while(!Serial);

  Serial.println("Start..");

  // Initialize PortHandler instance
  // Set the port path
  // Get methods and members of PortHandlerLinux or PortHandlerWindows
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);

  // Initialize PacketHandler instance
  // Set the protocol version
  // Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

  dynamixel::GroupSyncWrite groupSyncWrite(portHandler, packetHandler, 116, 4);
  dynamixel::GroupSyncRead groupSyncRead(portHandler, packetHandler, ADDRESS_PRESENT_POSITION, LENGTH_PRESENT_POSITION);

  // Add parameter storage for Dynamixel#1 present position value
  groupSyncRead.addParam(DXL1_ID);
  groupSyncRead.addParam(DXL2_ID);
  groupSyncRead.addParam(DXL3_ID);
  groupSyncRead.addParam(DXL4_ID);
  //groupSyncRead.addParam(DXL5_ID);

  int index = 0;
  int dxl_comm_result = COMM_TX_FAIL;             // Communication result
  int dxl_goal_position = 2048;         // Goal position

  uint8_t dxl_error = 0;                          // Dynamixel error
  int32_t dxl_present_position = 0;               // Present position

  // Open port
  if (portHandler->openPort())
  {
    Serial.print("Succeeded to open the port!\n");
  }
  else
  {
    Serial.print("Failed to open the port!\n");
    return;
  }

  // Set port baudrate
  if (portHandler->setBaudRate(BAUDRATE))
  {
    Serial.print("Succeeded to change the baudrate!\n");
  }
  else
  {
    Serial.print("Failed to change the baudrate!\n");
    return;
  }

  packetHandler->write1ByteTxRx(portHandler, 254, 64, 1, &dxl_error);

  while(1)
  {
    Serial.print("Press any key to continue! (or press q to quit!)\n");
    if (getch() == 'q')
      break;

    param_goal_position[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_position));
    param_goal_position[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_position));
    param_goal_position[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_position));
    param_goal_position[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_position));
    
    tx_trial_count = 0;
    rx_trial_count = 0;
    tx_success_count = 0;
    rx_success_count = 0;
    loop_count = 0;

    time_begin = millis();
    Serial.print("Start at : ");
    Serial.print(time_begin);
    Serial.println(" ");
    
    do
    {
      groupSyncWrite.addParam(DXL1_ID, param_goal_position);
      groupSyncWrite.addParam(DXL2_ID, param_goal_position);
      groupSyncWrite.addParam(DXL3_ID, param_goal_position);
      groupSyncWrite.addParam(DXL4_ID, param_goal_position);
      
      // Syncwrite Goal Position
      dxl_comm_result = groupSyncWrite.txPacket();
      tx_trial_count++;
      if (dxl_comm_result != COMM_SUCCESS)
      {
        packetHandler->getTxRxResult(dxl_comm_result);
      } else {
        tx_success_count++;
      }
      // Clear syncwrite parameter storage
      groupSyncWrite.clearParam();
      
      // Syncread present position
      dxl_comm_result = groupSyncRead.txRxPacket();
      rx_trial_count++;
      if (dxl_comm_result != COMM_SUCCESS)
      {
        packetHandler->getTxRxResult(dxl_comm_result);
      } else {
        rx_success_count++;
      }

      // Get Dynamixel#1 present position value
      dxl1_present_position = groupSyncRead.getData(DXL1_ID, ADDRESS_PRESENT_POSITION, LENGTH_PRESENT_POSITION);
      dxl2_present_position = groupSyncRead.getData(DXL2_ID, ADDRESS_PRESENT_POSITION, LENGTH_PRESENT_POSITION);
      dxl3_present_position = groupSyncRead.getData(DXL3_ID, ADDRESS_PRESENT_POSITION, LENGTH_PRESENT_POSITION);
      dxl4_present_position = groupSyncRead.getData(DXL4_ID, ADDRESS_PRESENT_POSITION, LENGTH_PRESENT_POSITION);
      //dxl5_present_position = groupSyncRead.getData(DXL5_ID, ADDRESS_PRESENT_POSITION, LENGTH_PRESENT_POSITION);
      packetHandler->read4ByteTxRx(portHandler, DXL5_ID, ADDRESS_PRESENT_POSITION, (uint32_t*)&dxl5_present_position, &dxl_error);

      loop_count++;
    }while(loop_count < 1000);

    time_temp = millis();
    Serial.print("End at : ");
    Serial.print(time_temp);
    Serial.println(" ");
    time_begin = time_temp - time_begin;

    Serial.println(" ");
    Serial.print("Total TX / RX Count : ");
    Serial.print(loop_count);
    Serial.println(" ");
    Serial.print("TX [Trial | Successful] : [");
    Serial.print(tx_trial_count);
    Serial.print(" | ");
    Serial.print(tx_success_count);
    Serial.println(" ]");
    Serial.print("Total RX Count : ");
    Serial.print(loop_count);
    Serial.println(" ");
    Serial.print("RX [Trial | Successful] : [");
    Serial.print(rx_trial_count);
    Serial.print(" | ");
    Serial.print(rx_success_count);
    Serial.println(" ]");
    Serial.print("Elapsed Time(ms) : ");
    Serial.print(time_begin);
    Serial.println(" ");

    // Change goal position
    if (index == 0)
    {
      index = 1;
    }
    else
    {
      index = 0;
    }
  }

  // Close port
  portHandler->closePort();
}

void loop()
{
}
