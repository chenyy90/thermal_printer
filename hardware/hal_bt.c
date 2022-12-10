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
//byte1-16: uuid
// 49535343-FE78-4AE5-8FA9-9FAFD205E455
uint8_t ble_service_uuid_lsps[] = {0x10,0x55,0xe4,0x05,0xd2,0xaf,0x9f,0xa9,0x8f,0xe5,0x4a,0x7d,0xfe,0x43,0x53,0x53,0x49};

/*
byte0: characterisitic  
byte1:characterisitic uuid length  
byte2-17:characterisitic uuid 
byte18:write/read payload length 
byte19-20:write/read payload
*/
//49535343-1E4D-4BD9-BA61-23C647249616
uint8_t ble_Characteristic_uuid_lsps_tx[] = {0x10,0x10,0x16,0x96,0x24,0x47,0xc6,0x23,0x61,0xba,0xd9,0x4b,0x4d,0x1e,0x43,0x53,0x53,0x49,0x01,0x00};
//49535343-8841-43F4-A8D4-ECBE34729BB3
uint8_t ble_Characteristic_uuid_lsps_rx[] = {0x0c,0x10,0xb3,0x9b,0x72,0x34,0xbe,0xec,0xd4,0xa8,0xf4,0x43,0x41,0x88,0x43,0x53,0x53,0x49,0x01,0x00};
//49535343-aca3-481c-91ec-d85e28a60318
uint8_t ble_Characteristic_uuid_flow_ctrl[] = {0x18,0x10,0x18,0x03,0xa6,0x28,0x5e,0xd8,0xec,0x91,0x1c,0x48,0xa3,0xac,0x43,0x53,0x53,0x49,0x01,0x00};	

	
#define IFLASH_NVRAM_ADDR 0x1000000+((512-4)*1024)
#define NVRAM_LEN 170//Nvram 长度170bytes,每组Nvram长度为34bytes,最多可存储5组配对信息
//#define SPP_FLOWCONTROL
uint16_t ble_send_handle=0x2a;
uint8_t bt_cod[3] = {0x24,0x04,0x04};
uint8_t NvramData[NVRAM_LEN]={0};

	
//write flash 
uint8_t   ReserveNV(uint8_t* nvram);
extern tick SystickCount;
#ifdef UART_TO_IPC
HCI_TypeDef HCI_Tx;
HCI_TypeDef HCI_Rx;
uint8_t ipc_tx_buf[256];
uint8_t ipc_rx_buf[256];
#endif

static uint8_t hex_lookup[16] = { '0', '1', '2', '3', '4', '5', '6', '7','8','9', 'a', 'b', 'c', 'd', 'e', 'f' };
static void array2hex(uint8_t* in, int inlen, uint8_t* out)
{
	int i;
	for (i = 0; i < inlen; ++i)
	{
		out[2 * i] = hex_lookup[in[i] >> 4];
		out[2 * i + 1] = hex_lookup[in[i] & 0x0f];
	}
}

int hal_bt_init(void)
{
    uint8_t bt_name[15] = "YC3121bt";
    uint8_t bt_addr[6] =  {0x00,0x00,0x00,0x33,0x22,0x10};
    uint8_t chip_id[6]={0};
    uint16_t temp_handle;

    BT_Init();
    enable_intr(INTR_BT);

    read_chipid(chip_id);
    array2hex(chip_id,3,bt_name+8);
    bt_name[14]='\0';
    memcpy(bt_addr,chip_id,3);

    if(BT_SetBleName(bt_name,sizeof(bt_name)-1)==TRUE) 
        HAL_LOGD("HAL BT", "SetBleName_suc ble name:%s\n",bt_name);
    else 
        HAL_LOGD("HAL BT", "SetBleName_failed\n");

    if(BT_SetBtName(bt_name,sizeof(bt_name)-1) == TRUE)//bt与ble名字地址可以设置成一样
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

    if(BT_SetParingMode(0x03) == TRUE)//设置配对模式为confirmkey
        HAL_LOGD("HAL BT", "set confirmkey mode success\n");
    else
        HAL_LOGD("HAL BT", "set confirmkey mode failed\n");

    if(BT_SetCOD(bt_cod) == TRUE) //设置COD
        HAL_LOGD("HAL BT", "set COD sucess\n");
    else
        HAL_LOGD("HAL BT", "set COD failed\n");


    if(BT_DeleteService() == TRUE) //  删除用户自定义服务
        HAL_LOGD("HAL BT", "delete service sucess\n");
    else
        HAL_LOGD("HAL BT", "delete service failed\n");

    temp_handle = BT_AddBleService(ble_service_uuid_lsps,sizeof(ble_service_uuid_lsps));
    if( temp_handle != 0)  //增加服务 返回handle无需保存
        HAL_LOGD("HAL BT", "add service sucess,handle=%04x\n",temp_handle);
    else
        HAL_LOGD("HAL BT", "add service failed,return=%04x\n",temp_handle);

    ble_send_handle = BT_AddBleCharacteristic(ble_Characteristic_uuid_lsps_tx,sizeof(ble_Characteristic_uuid_lsps_tx));
    if( ble_send_handle != 0)  //增加服务特征  write返回的handle需要保存，发数据使用
        HAL_LOGD("HAL BT", "add Characteristic tx sucess,handle=%04x\n",ble_send_handle);
    else
        HAL_LOGD("HAL BT", "add Characteristic tx failed,return=%04x\n",ble_send_handle);

    temp_handle = BT_AddBleCharacteristic(ble_Characteristic_uuid_lsps_rx,sizeof(ble_Characteristic_uuid_lsps_rx));
    if( temp_handle!= 0)
        HAL_LOGD("HAL BT", "add Characteristic rx sucess;handle=%04x\n",temp_handle);
    else
        HAL_LOGD("HAL BT", "add Characteristic rx failed,return=%04x\n",temp_handle);

    temp_handle = BT_AddBleCharacteristic(ble_Characteristic_uuid_flow_ctrl,sizeof(ble_Characteristic_uuid_flow_ctrl));
    if( temp_handle!= 0)
        HAL_LOGD("HAL BT", "add Characteristic flow_ctrl sucess;handle=%04x\n",temp_handle);
    else
        HAL_LOGD("HAL BT", "add Characteristic flow_ctrl failed,return=%04x\n",temp_handle);

    if(BT_SetVisibility(0x01,0x01,0x01) == TRUE)  //设置可发现
        HAL_LOGD("HAL BT", "SetVisibility sucess\n");
    else
        HAL_LOGD("HAL BT", "SetVisibility failed\n");

    HAL_LOGD("HAL BT", "bt version=%x\n", BT_GetVersion());

    qspi_flash_read(IFLASH_NVRAM_ADDR,NvramData,NVRAM_LEN);
    if(NvramData[34*0] > 0x05||NvramData[34*1] > 0x05||NvramData[34*2] > 0x05||NvramData[34*3] > 0x05||NvramData[34*4] > 0x05)
    {
        memset(NvramData,0x00,NVRAM_LEN);
    }

    int i = 0;
    if(BT_SetNVRAM(NvramData,NVRAM_LEN) == TRUE)
    {
        HAL_LOGD("HAL BT", "set nvram success:\n");
        while(i<NVRAM_LEN)
            HAL_LOGD("HAL BT", "0x%02X ",NvramData[i++]);
        HAL_LOGD("HAL BT", "\n");
    }
    else
    {
        HAL_LOGD("HAL BT", "set nvram failed\n");
    }
    return 0;
}

void BT_IRQHandler()
{
    while (IPC_have_data())
    {
        BT_ParseBTData();
    }

    BT_CONFIG &= (~(1<<BT_INIT_FLAG));
}
