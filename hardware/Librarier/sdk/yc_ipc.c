#include "yc_ipc.h"

//#define	IPC_DEBUG

IPC_TypeDef *IpcRx = (IPC_TypeDef *)(IPC_RX_START_ADDR);
IPC_TypeDef *IpcTx = (IPC_TypeDef *)(IPC_TX_START_ADDR);

uint8_t ipcrecbuf[IPCREVSIZE] = {0};

Boolean IPC_have_data()
{
    return HR_REG_16BIT(&IpcRx->ReadPtr) != HR_REG_16BIT(&IpcRx->WrtiePtr) ? TRUE : FALSE;
}

uint16_t IPC_get_available_size()
{
    uint16_t readtx = HR_REG_16BIT(&IpcTx->ReadPtr);
    uint16_t wrtptr = HR_REG_16BIT(&IpcTx->WrtiePtr);
    uint16_t ipcendaddr = HR_REG_16BIT(&IpcTx->IpcEndAddr);
    uint16_t ipcstartaddr = HR_REG_16BIT(&IpcTx->IpcStartAddr);

    if (readtx > wrtptr)
    {
        return readtx - wrtptr;
    }
    else
    {
        return ((ipcendaddr - ipcstartaddr + 1) - (readtx - wrtptr));
    }
}

void IPC_TxData(HCI_TypeDef *IpcData)
{
	#ifdef IPC_DEBUG
	PrintHCIPack(IpcData,"tx");
	#endif
    WAKEUP_BT |= (1 << WAKEUP_BT_FLAG);
    IPC_HOLD_BT  = 1;
    delay_ms(10);
    uint16_t Wptr  = HR_REG_16BIT(&IpcTx->WrtiePtr);
    uint16_t Len   = IpcData->DataLen + 3;
    uint8_t *Rptr = (uint8_t *)IpcData;
    while (Len--)
    {
        HW_IPC_REG_8BIT(Wptr, *Rptr++);
        RB_UPDATE_PTR(Wptr, HR_REG_16BIT(&IpcTx->IpcStartAddr),  HR_REG_16BIT(&IpcTx->IpcEndAddr));
        if (Len == IpcData->DataLen)
            Rptr = IpcData->p_data;
    }

    HW_REG_16BIT(&IpcTx->WrtiePtr, (uint32_t)Wptr);
    IPC_HOLD_BT = 0;
    WAKEUP_BT &= ~(1 << WAKEUP_BT_FLAG);
}

Boolean IPC_PutBtData(const void *buf, uint32_t size)
{
    if (size == 0 || size > 258)
    {
        return FALSE;
    }
    WAKEUP_BT |= (1 << WAKEUP_BT_FLAG);
    IPC_HOLD_BT  = 1;
    delay_ms(10);
    uint16_t Wptr  = HR_REG_16BIT(&IpcTx->WrtiePtr);
    uint8_t *Rptr = (uint8_t *)buf;
    while (size--)
    {
        HW_IPC_REG_8BIT(Wptr, *Rptr++);
        RB_UPDATE_PTR(Wptr, HR_REG_16BIT(&IpcTx->IpcStartAddr),  HR_REG_16BIT(&IpcTx->IpcEndAddr));
    }

    HW_REG_16BIT(&IpcTx->WrtiePtr, (uint32_t)Wptr);
    IPC_HOLD_BT = 0;
    WAKEUP_BT &= ~(1 << WAKEUP_BT_FLAG);
    return TRUE;
}

Boolean IPC_ReadBtData(HCI_TypeDef *IpcData)
{
    if (HR_REG_16BIT(&IpcRx->ReadPtr) != HR_REG_16BIT(&IpcRx->WrtiePtr))
    {
        uint16_t Rptr = HR_REG_16BIT(&IpcRx->ReadPtr);
        IpcData->type = HR_IPC_REG_8BIT(Rptr);
        RB_UPDATE_PTR(Rptr, HR_REG_16BIT(&IpcRx->IpcStartAddr),  HR_REG_16BIT(&IpcRx->IpcEndAddr));
        IpcData->opcode = HR_IPC_REG_8BIT(Rptr);
        RB_UPDATE_PTR(Rptr, HR_REG_16BIT(&IpcRx->IpcStartAddr),  HR_REG_16BIT(&IpcRx->IpcEndAddr));
        IpcData->DataLen = HR_IPC_REG_8BIT(Rptr);
        RB_UPDATE_PTR(Rptr, HR_REG_16BIT(&IpcRx->IpcStartAddr),  HR_REG_16BIT(&IpcRx->IpcEndAddr));
        for (uint8_t i = 0; i < IpcData->DataLen; i++)
        {
            IpcData->p_data[i] = HR_IPC_REG_8BIT(Rptr);
            RB_UPDATE_PTR(Rptr, HR_REG_16BIT(&IpcRx->IpcStartAddr),  HR_REG_16BIT(&IpcRx->IpcEndAddr));
        }
        HW_REG_16BIT(&IpcRx->ReadPtr, (uint32_t)Rptr);
		#ifdef IPC_DEBUG
		PrintHCIPack(IpcData,"rx");
		#endif
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

Boolean IPC_GetBtData(void *buf, uint32_t *size)
{
    uint32_t i = 0;
    uint16_t Rptr = HR_REG_16BIT(&IpcRx->ReadPtr);
    if (*size == 0)
    {
        *size = 0;
        return FALSE;
    }
    if (TRUE == IPC_have_data())
    {
        while (Rptr != HR_REG_16BIT(&IpcRx->WrtiePtr) && i < (*size))
        {
            ((uint8_t *)buf)[i++] = HR_IPC_REG_8BIT(Rptr);
            RB_UPDATE_PTR(Rptr, HR_REG_16BIT(&IpcRx->IpcStartAddr),  HR_REG_16BIT(&IpcRx->IpcEndAddr));
        }
        HW_REG_16BIT(&IpcRx->ReadPtr, (uint32_t)Rptr);
        *size = i;
        return TRUE;
    }
    else
    {
        *size = 0;
        return FALSE;
    }
}

static void noinline _dma_start(int ch, const void *src, int srclen, void *dest, int dstlen)
{
    DMA_SRC_ADDR(ch)  = (int)src;
    DMA_DEST_ADDR(ch) = (int)dest;
    DMA_LEN(ch)       = srclen << 16 | dstlen;
    DMA_START(ch)     = 0x80;
}

void noinline _dma_wait(int ch)
{
    while (!(DMA_STATUS(ch) & 1));
}

void _dmacopy(void *dest, void *src, int len)
{
    _dma_start(DMACH_MEMCP, src, len, dest, len);
    _dma_wait(DMACH_MEMCP);
}

void _download_btcode(const byte *btcode)
{
    int addr, len;
    btcode += 2;
    if (GETWORD(btcode) != 0x55aa)
        return;
    btcode += 2;
    enable_clock(CLKCLS_BT);
    len = GETWORD(btcode);
    btcode += 2;
    BT_UCODE_LO   = 0;
    BT_UCODE_HI   = 0;
    BT_UCODE_CTRL = 0x80;
    DMA_CONFIG(DMACH_MEMCP) = 8;
    _dmacopy((byte *)&BT_UCODE_DATA, (byte *)btcode, len);
    BT_UCODE_CTRL = 0;
    btcode       += len;
    DMA_CONFIG(DMACH_MEMCP) = 0;

    while (GETWORD(btcode) == 0x55aa)
    {
        btcode += 2;
        len  = GETWORD(btcode);
        btcode += 2;
        addr = GETWORD(btcode) | 0xc0000;
        btcode += 2;
        _dmacopy((byte *)addr, (byte *)btcode, len);
        btcode += len;
    }
}

//extern uint8_t BT_Wake;
extern const unsigned char bt_code[];

void erase_memory(void)
{
    for (uint32_t i = 0; i < 0x1000; i++)
    {
        *(volatile uint8_t *)(0xc0000 + i) = 0;
        *(volatile uint8_t *)(0xc4000 + i) = 0;
    }
}
#if (VERSIONS == NO_BT)
static uint16_t get_dpll_status(void)
{
    uint16_t addr = 0;
    addr  = (*(uint8_t *)(0xc800e));
    addr += (*(uint8_t *)(0xc800f)) << 8;
    return addr;
}

static Boolean is_dpll_opend(void)
{
    uint16_t addr =  get_dpll_status();

    Boolean ret = FALSE;

    if ((0x4c77 == addr) || (0x4c78 == addr) || (0x4c79 == addr))
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }
    MyPrintf("is_dpll_opend=%x-%x", addr, ret);
    return ret;
}
#endif

Boolean ipc_inited = FALSE;
void IpcInit(void)
{
#if (VERSIONS == EXIST_BT)
    if (ipc_inited == FALSE)
    {
        disable_intr(INTR_BT);
        ipc_inited = TRUE;
        SYSCTRL_ROM_SWITCH = 0x94;
        delay(10000);   //wait rom switch ok
        BT_RESET = 1;
        delay(10000);   //wait for reset ok
        while (!(BT_CONFIG & (1 << BT_INIT_FLAG)));
        BT_CONFIG &= (~(1 << BT_INIT_FLAG));
        erase_memory();
        _download_btcode(bt_code);
        WAKEUP_BT = 0x9c;
        delay_ms(100);
    }
#elif (VERSIONS == NO_BT)
    if (ipc_inited == FALSE || is_dpll_opend() == FALSE)
    {
        disable_intr(INTR_BT);
        ipc_inited = TRUE;

        lpm_bt_write(1, 0xd00000);
        delay_us(1000);

        SYSCTRL_ROM_SWITCH = 0x94;
        delay(10000);   //wait rom switch ok
        BT_RESET = 1;
        delay(10000);   //wait for reset ok
        while (!(BT_CONFIG & (1 << BT_INIT_FLAG)));
        BT_CONFIG &= (~(1 << BT_INIT_FLAG));
        *(volatile uint8_t *)0xC4176 = 0x0f;
        *(volatile uint8_t *)0xC4177 = 0x84;
        *(volatile uint8_t *)0xC4178 = 0x30;
        *(volatile uint8_t *)0xC4179 = 0x0a;
        *(volatile uint8_t *)0xC40c3 = 0x00;
        *(volatile uint8_t *)0xC40c4 = 0x4f;
        *(volatile uint8_t *)0xC4f00 = 0xff;
        delay_ms(60);
    }
#endif
}

//打印HCI数据包
void PrintHCIPack(HCI_TypeDef *msg, const char *str)
{
    MyPrintf("M0 %s:\n", str);
    MyPrintf("%s Type:%02x\n", str, msg->type);
    MyPrintf("%s OPCode: %02x\n", str, msg->opcode);
    MyPrintf("%s Length: %02x\n", str, msg->DataLen);
    MyPrintf("%s Data:\n", str);
    for (uint8_t cnt = 0; cnt < msg->DataLen; cnt++)
    {
        MyPrintf("%02x ", msg->p_data[cnt]);
        if ((cnt % 16) == 0&&cnt!=0)
            MyPrintf("\n");
    }
    MyPrintf("\n");
}
