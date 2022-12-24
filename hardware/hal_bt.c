/*******************************************************
 * File Name    : hal_bt.c
 * Description  : hardware adc unit.
 -------------------------------------------------------
 Version         |      Date         |         Author 
 -------------------------------------------------------
 1.0             |      2022-11-23   |         oem
********************************************************/
#include "hal_bt.h"
#include "hal_log.h"

// byte0： uuid length
// byte1-16: uuid
// 49535343-FE78-4AE5-8FA9-9FAFD205E455
const uint8_t ble_service_uuid_lsps[] = {0x10,0x55,0xe4,0x05,0xd2,0xaf,0x9f,0xa9,0x8f,0xe5,0x4a,0x7d,0xfe,0x43,0x53,0x53,0x49};

/*
    byte0: characterisitic  
    byte1:characterisitic uuid length  
    byte2-17:characterisitic uuid 
    byte18:write/read payload length 
    byte19-20:write/read payload
*/
//49535343-1E4D-4BD9-BA61-23C647249616
const uint8_t ble_Characteristic_uuid_lsps_tx[] = {0x10,0x10,0x16,0x96,0x24,0x47,0xc6,0x23,0x61,0xba,0xd9,0x4b,0x4d,0x1e,0x43,0x53,0x53,0x49,0x01,0x00};
//49535343-8841-43F4-A8D4-ECBE34729BB3
const uint8_t ble_Characteristic_uuid_lsps_rx[] = {0x0c,0x10,0xb3,0x9b,0x72,0x34,0xbe,0xec,0xd4,0xa8,0xf4,0x43,0x41,0x88,0x43,0x53,0x53,0x49,0x01,0x00};
//49535343-aca3-481c-91ec-d85e28a60318
const uint8_t ble_Characteristic_uuid_flow_ctrl[] = {0x18,0x10,0x18,0x03,0xa6,0x28,0x5e,0xd8,0xec,0x91,0x1c,0x48,0xa3,0xac,0x43,0x53,0x53,0x49,0x01,0x00};    
//Printer COD : 0x100680
const uint8_t bt_cod[3] = {0x10, 0x06, 0x80};
//ble adv
const uint8_t bt_bleadv[31] = {0x08, 0x4c, 0x59, 0x00,0x00,0x00,0x33,0x22,0x10};

#define IFLASH_NVRAM_ADDR 0x1000000+((512-4)*1024)
#define NVRAM_LEN 170  //Nvram 长度170bytes,每组Nvram长度为34bytes,最多可存储5组配对信息

uint16_t ble_send_handle = 0x2a;

// uint8_t NvramData[NVRAM_LEN] = {0};

int hal_bt_init(void)
{
    uint8_t bt_name[15] = "LP100S";
    uint8_t bt_addr[6] =  {0x00,0x00,0x00,0x33,0x22,0x10};
    uint16_t temp_handle;

    BT_Init();
    enable_intr(INTR_BT);

    if(BT_SetBleName(bt_name,sizeof(bt_name)-1)==TRUE) 
        HAL_LOGD("HAL BT", "SetBleName_suc ble name:%s\n",bt_name);
    else 
        HAL_LOGD("HAL BT", "SetBleName_failed\n");

    if(BT_SetBtName(bt_name,sizeof(bt_name)-1) == TRUE)
        HAL_LOGD("HAL BT", "SetbtName_suc\n");
    else 
        HAL_LOGD("HAL BT", "SetbtName_fail\n");

    if(BT_SetBleAddr(bt_addr) == TRUE) 
        HAL_LOGD("HAL BT", "SetBleAddr_suc\n");
    else 
        HAL_LOGD("HAL BT", "SetBleAddr_fail\n"); 

    if(BT_SetBtAddr(bt_addr) == TRUE)
        HAL_LOGD("HAL BT", "SetBtAddr_suc\n");
    else 
        HAL_LOGD("HAL BT", "SetBtAddr_fail\n");

    /* 设置配对模式为confirmkey */
    if(BT_SetParingMode(0x03) == TRUE)
        HAL_LOGD("HAL BT", "set confirmkey mode success\n");
    else
        HAL_LOGD("HAL BT", "set confirmkey mode failed\n");

    /* 设置COD */
    if(BT_SetCOD((uint8_t *)bt_cod) == TRUE)
        HAL_LOGD("HAL BT", "set COD sucess\n");
    else
        HAL_LOGD("HAL BT", "set COD failed\n");

    /* 删除用户自定义服务 */
    if(BT_DeleteService() == TRUE)
        HAL_LOGD("HAL BT", "delete service sucess\n");
    else
        HAL_LOGD("HAL BT", "delete service failed\n");

    /*  增加服务 返回handle无需保存 */
    temp_handle = BT_AddBleService((uint8_t *)ble_service_uuid_lsps,sizeof(ble_service_uuid_lsps));
    if( temp_handle != 0)
        HAL_LOGD("HAL BT", "add service sucess,handle=%04x\n",temp_handle);
    else
        HAL_LOGD("HAL BT", "add service failed,return=%04x\n",temp_handle);

    /* 增加服务特征  write返回的handle需要保存，发数据使用 */
    ble_send_handle = BT_AddBleCharacteristic((uint8_t *)ble_Characteristic_uuid_lsps_tx,sizeof(ble_Characteristic_uuid_lsps_tx));
    if( ble_send_handle != 0)
        HAL_LOGD("HAL BT", "add Characteristic tx sucess,handle=%04x\n",ble_send_handle);
    else
        HAL_LOGD("HAL BT", "add Characteristic tx failed,return=%04x\n",ble_send_handle);

    /* 增加服务特征  read */
    temp_handle = BT_AddBleCharacteristic((uint8_t *)ble_Characteristic_uuid_lsps_rx,sizeof(ble_Characteristic_uuid_lsps_rx));
    if( temp_handle!= 0)
        HAL_LOGD("HAL BT", "add Characteristic rx sucess;handle=%04x\n",temp_handle);
    else
        HAL_LOGD("HAL BT", "add Characteristic rx failed,return=%04x\n",temp_handle);

    /* 增加服务特征  flowctrl */
    temp_handle = BT_AddBleCharacteristic((uint8_t *)ble_Characteristic_uuid_flow_ctrl,sizeof(ble_Characteristic_uuid_flow_ctrl));
    if( temp_handle!= 0)
        HAL_LOGD("HAL BT", "add Characteristic flow_ctrl sucess;handle=%04x\n",temp_handle);
    else
        HAL_LOGD("HAL BT", "add Characteristic flow_ctrl failed,return=%04x\n",temp_handle);

    /* 增加BLE广播 */
    if(BT_SetLEAdvData((uint8_t *)bt_bleadv, 31) == TRUE)
        HAL_LOGD("HAL BT", "SetLEAdvData sucess\n");
    else
        HAL_LOGD("HAL BT", "SetLEAdvData failed\n");

    /* 设置可发现 */
    if(BT_SetVisibility(0x01,0x01,0x01) == TRUE)
        HAL_LOGD("HAL BT", "SetVisibility sucess\n");
    else
        HAL_LOGD("HAL BT", "SetVisibility failed\n");

    HAL_LOGD("HAL BT", "bt version=%x\n", BT_GetVersion());

    // qspi_flash_read(IFLASH_NVRAM_ADDR,NvramData,NVRAM_LEN);
    // if(NvramData[34*0] > 0x05||NvramData[34*1] > 0x05||NvramData[34*2] > 0x05||NvramData[34*3] > 0x05||NvramData[34*4] > 0x05)
    // {
    //     memset(NvramData,0x00,NVRAM_LEN);
    // }

    // int i = 0;
    // if(BT_SetNVRAM(NvramData,NVRAM_LEN) == TRUE)
    // {
    //     HAL_LOGD("HAL BT", "set nvram success:\n");
    //     while(i<NVRAM_LEN)
    //         printf("0x%02X ",NvramData[i++]);
    //     HAL_LOGD("HAL BT", "\n");
    // }
    // else
    // {
    //     HAL_LOGD("HAL BT", "set nvram failed\n");
    // }

    return 0;
}

void BT_IRQHandler(void)
{
    while (IPC_have_data())
    {
        BT_ParseBTData();
    }
    BT_CONFIG &= (~(1<<BT_INIT_FLAG));
}

int hal_bt_progress(void)
{
    int eventCmd = BT_GetEventOpcode();

    switch(eventCmd)
    {
        case BT_DATA:
        {
            uint8_t btData[MAX_BLUETOOTH_DATA_LEN];
            int BtDataLen=0;
            BtDataLen=BT_ReadBTData(btData);
            if(BtDataLen>0)
            {
                HAL_LOGD("HAL BT", "received %d bytes bt data:\n",BtDataLen);
                for(int i=0;i<BtDataLen;i++) printf("%02x ",btData[i]);
                printf("\n");	

                BT_SendSppData(btData,BtDataLen);
                HAL_LOGD("HAL BT", "send %d bytes bt data:\n",BtDataLen);
                for(int i=0;i<BtDataLen;i++) printf("%02x ",btData[i]);
                printf("\n");	
                
                #if 1 // def SPP_FLOWCONTROL
                if(BT_SetSppFlowcontrol(1) != TRUE)	HAL_LOGD("HAL BT", "set Flowcontrol failed\n");
                #endif
            }
            else
            {
                HAL_LOGD("HAL BT", "read bt data error\r\n");
            }
        }
        break;
        case BLE_DATA:
        {
            uint8_t bleData[MAX_BLUETOOTH_DATA_LEN]={0};
            int BleDataLen=0;
            BleDataLen=BT_ReadBTData(bleData+2);
            if(BleDataLen>0)
            {
                HAL_LOGD("HAL BT", "received %d bytes ble data:\n",BleDataLen);
                for(int i=0;i<BleDataLen;i++) printf("%02x ",bleData[2+i]);
                printf("\n");
                
                //add handle
                bleData[0]=(ble_send_handle&0xff);
                bleData[1]=((ble_send_handle>>8)&0xff);
                BT_SendBleData(bleData,BleDataLen+2);
                HAL_LOGD("HAL BT", "send %d bytes ble data[%02x %02x]:\n",BleDataLen,bleData[0],bleData[1]);
                for(int i=0;i<BleDataLen;i++) printf("%02x ",bleData[2+i]);
                printf("\n");
            }
            else
            {
                HAL_LOGD("HAL BT", "read ble data error\r\n");
            }
        }
        break;
        case BT_CONNECTED:
            HAL_LOGD("HAL BT", "bt connected\n");
        break;
        case BLE_CONNECTED:
            HAL_LOGD("HAL BT", "ble connected\n");
        break;
        case BT_DISCONNECTED:
            HAL_LOGD("HAL BT", "bt disconnected\n");
        break;
        case BLE_DISCONNECTED:
            HAL_LOGD("HAL BT", "ble disconnected\n");
        break;
        case NVRAM_DATA:
        {
            uint8_t NvramData[MAX_BLUETOOTH_DATA_LEN];
            int NvramDataLen=0;
            NvramDataLen=BT_ReadBTData(NvramData);
            if(NvramDataLen>0)
            {
                HAL_LOGD("HAL BT", "receive %dbytes Nvramdata:",NvramDataLen);
                for(int i=0;i<NvramDataLen;i++) printf("%02X ",NvramData[i]);
                printf("\n");
                // ReserveNV(NvramData);//将最新的5个设备信息（NVRAM） 写入flash ,
            }
            else
            {
                HAL_LOGD("HAL BT", "read Nvram data error\r\n");
            }
        }
        break;
        case CONFIRM_GKEY:
        {
            uint8_t GkeyData[MAX_BLUETOOTH_DATA_LEN];
            int GkeyDataLen=0;
            GkeyDataLen=BT_ReadBTData(GkeyData);
            if(GkeyDataLen>0)
            {
                HAL_LOGD("HAL BT", "receive %dbytes CONFIRM_GKEY:",GkeyDataLen);
                for(int i=0;i<GkeyDataLen;i++) printf("%02X ",GkeyData[i]);
                printf( "\n");
            }
            else
            {
                HAL_LOGD("HAL BT", "read CONFIRM_GKEY data error\r\n");
            }

            if(BT_ConfirmGkey(0) == TRUE)//BT_ConfirmGkey参数为0则继续配对，1则取消配对。
                HAL_LOGD("HAL BT", "set confirmkey  success");
            else
                HAL_LOGD("HAL BT", "set confirmkey  failed\n");
        }
        break;
        case PASSKEY:
            HAL_LOGD("HAL BT", "request Passkey");
        break;
        case PAIRING_COMPLETED:
        {
            uint8_t PairingData[MAX_BLUETOOTH_DATA_LEN];
            int PairingDataLen=0;
            PairingDataLen=BT_ReadBTData(PairingData);
            if(PairingDataLen>0)
            {
                HAL_LOGD("HAL BT", "receive %dbytes paring data:",PairingDataLen);
                for(int i=0;i<PairingDataLen;i++) printf("%02X ",PairingData[i]);
                printf("\n");
            }
            else
            {
                HAL_LOGD("HAL BT", "read PAIRING_COMPLETED data error\r\n");
            }
        }
        break;
        case PAIRING_STATE:
        {
            uint8_t PairingData[MAX_BLUETOOTH_DATA_LEN];
            int PairingDataLen=0;
            uint16_t pairing_status=0;
            PairingDataLen=BT_ReadBTData(PairingData);
            if(PairingDataLen==2)
            {
                pairing_status |=PairingData[0];
                pairing_status |=(PairingData[1]<<8);
                switch(pairing_status)
                {
                    case 0x0001:
                        HAL_LOGD("HAL BT", "BT Pairing Success\r\n");
                    break;
                    case 0x0101:
                        HAL_LOGD("HAL BT", "BT Pairing fail\r\n");
                    break;
                    case 0x0080:
                        HAL_LOGD("HAL BT", "BLE Pairing Success\r\n");
                    break;
                    case 0x0180:
                        HAL_LOGD("HAL BT", "BLE Pairing fail\r\n");
                    break;
                    default:
                        HAL_LOGD("HAL BT", "unknown pairing_status[%04x]\r\n",pairing_status);
                    break;
                }
            }
            else
            {
                HAL_LOGD("HAL BT", "read PAIRING_STATE data error\r\n");
            }
        }
        break;
        case -1:
            /* no event */
        return -1;
        default:
        {
            HAL_LOGD("HAL BT", "unknown event:%02x\r\n",eventCmd);
            uint8_t unknownData[MAX_BLUETOOTH_DATA_LEN];
            int unknownDataLen=0;
            unknownDataLen=BT_ReadBTData(unknownData);
            if(unknownDataLen>0)
            {
                HAL_LOGD("HAL BT", "datalen:%d\r\n",unknownDataLen);
                for(int i=0;i<unknownDataLen;i++) printf("%02X ",unknownData[i]);
                printf("\n");
            }
        }
        break;
    }

    return 0;
}
