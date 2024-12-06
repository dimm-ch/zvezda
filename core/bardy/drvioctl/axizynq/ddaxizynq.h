
#ifndef __IOTCLS_H__
#define __IOTCLS_H__

//-----------------------------------------------------------------------------

#define ZYNQDEV_DRIVER_NAME             "zynqdev"
#define MAX_ZYNQDEV_DEVICE_SUPPORT       1
#define ZYNQDEV_BAR_NUM                  2
#define ZYNQ_NODE_NAME_LEN               64

//-----------------------------------------------------------------------------
// IP AXI DMA v7.1 Scatter/Gather Registers map
//
#define AXI_SG_DMACR           0x00
#define AXI_SG_DMASR           0x04
#define AXI_SG_CURDESC         0x08
#define AXI_SG_CURDESC_MSB     0x0C
#define AXI_SG_TAILDESC        0x10
#define AXI_SG_TAILDESC_MSB    0x14

// Bits of AXI_SG_DMACR register
#define SG_DMACR_START         0x01
#define SG_DMACR_RESET         0x04
#define SG_DMACR_KEYHOLE       0x08
#define SG_DMACR_CYCLICBD      0x10
#define SG_DMACR_IOC_IRQEN     0x1000
#define SG_DMACR_DLY_IRQEN     0x2000
#define SG_DMACR_ERR_IRQEN     0x4000

// Bits of AXI_SG_DMASR register
#define SG_DMASR_HALTED        0x01
#define SG_DMASR_IDLE          0x02
#define SG_DMASR_SGINCLUDED    0x08
#define SG_DMASR_DMA_INT_ERR   0x10
#define SG_DMASR_DMA_SLV_ERR   0x20
#define SG_DMASR_DMA_DEC_ERR   0x40
#define SG_DMASR_DMA_SGINT_ERR 0x100
#define SG_DMASR_DMA_SGSLV_ERR 0x200
#define SG_DMASR_DMA_SGDEC_ERR 0x400
#define SG_DMASR_IOC_IRQEN     0x1000
#define SG_DMASR_DLY_IRQEN     0x2000
#define SG_DMASR_ERR_IRQEN     0x4000

#define SG_TXEOF (1 << 26)
#define SG_TXSOF (1 << 27)

//-----------------------------------------------------------------------------

#ifdef __linux__

#ifndef __KERNEL__
#include <sys/ioctl.h>
#include <stdint.h>
#endif
#define DEVICE_TYPE             'z'
#define MAKE_IOCTL(c) _IO(DEVICE_TYPE, (c))
#endif

//-----------------------------------------------------------------------------

#define BAR_INFO                    MAKE_IOCTL(0x01)
#define DMA_INFO                    MAKE_IOCTL(0x02)
#define DMA_BUFFER_LOCK             MAKE_IOCTL(0x10)
#define DMA_BUFFER_START            MAKE_IOCTL(0x11)
#define DMA_BUFFER_STOP             MAKE_IOCTL(0x12)
#define DMA_BUFFER_UNLOCK           MAKE_IOCTL(0x13)
#define DMA_BLOCK_WAIT              MAKE_IOCTL(0x14)
#define DMA_BUFFER_WAIT             MAKE_IOCTL(0x15)
#define DMA_STATE                   MAKE_IOCTL(0x16)

//-----------------------------------------------------------------------------
/*!
     \brief Структура ресурсов узла устройства в PL.

      Структура описывает все узлы устройства на шине:
      физический адрес BARx и диапазон занимаемый соответветствующим узлом,
      имя узла. Если, ресурсы не выделены или узел с заданным индексом
      не существует, то поля элементов равны 0.

 */
struct node_info_t {
    size_t        idx;                      //! Индекс узла: 0,1,2...
    size_t        pa;                       //! Физический адрес узла
    size_t        sz;                       //! Размер адресного пространства узла
    size_t        fl;                       //! Флаги адресного пространства узла
    char          name[ZYNQ_NODE_NAME_LEN]; //! Имя узла
};

struct bar_info_t {
    size_t               count;
    struct node_info_t*  nodes;
};

//-----------------------------------------------------------------------------
/*!
    \brief Тип памяти используемой каналом DMA устройства.

    KERNEL_MEMORY_DMA - выделяется драйвером. Физически непрервына. Малое число дескрипторов
    для получения буферов. Большая скорость передачи. Ограниченный размер.
    USER_MEMORY_DMA - выделяется пользователм. Физически разрывна на блоки 4К (PAGE_SIZE).
    Большое число дескрипторов для получения буфера. Меньшая скорость передачи.
    Почти неограниченный размер.
 */
enum dma_memory_type {
    KERNEL_MEMORY_DMA = 1,  //! Каналом используется память выдеоенная драйвером.
    USER_MEMORY_DMA = 2,    //! Физический адрес BARx
    BOOT_MEMORY_DMA = 3,    //! Физический адрес boot time memory allocation
};

//-----------------------------------------------------------------------------

enum dma_chan_status {
        CHANNEL_RUN = 1,
        CHANNEL_STOP = 2,
        CHANNEL_DESTROY = 3,
        CHANNEL_BREAK = 4
};

struct dma_stub_t {
     int32_t lastBlock;		// Number of Block which was filled last Time
    uint32_t totalCounter;	// Total Counter of all filled Block
    uint32_t offset;		// First Unfilled Byte
    uint32_t state;			// CBUF local state
};

//-----------------------------------------------------------------------------
/*!
     \brief Дескриптор канала DMA.

      Структура описывает канал DMA. Указывается напрвление передачи данных,
      номер канала, число блоков и размер одного блока, для формирования
      составного буфера. После обработки запроса, драйвер возвращает физические
      адреса каждого блока в массиве block_addr[]. За выделение памяти под
      эти адреса отвечает пользователь.
 */
struct dma_alloc_t {
    size_t controller;              //! DMA channel controller
    size_t direction;               //! DMA channel direction H2C or C2H
    size_t channel;                 //! DMA channel number 0, 1, 2, 3
    size_t blocks_type;             //! DMA buffer memory type (kernel/user)
    size_t blocks_count;            //! Number of blocks in DMA buffer
    size_t blocks_size;             //! Size of one block in bytes (must be multiple PAGE_SIZE)
    struct dma_stub_t*  stub[1];    //! Return physical addresses of struct dma_stub_t allocated in kernel
    void*  blocks_addr[1];          //! Return physical addresses of DMA blocks
};

//-----------------------------------------------------------------------------
/*!
     \brief Запсук канала DMA.

      Структура описывает канал DMA который неоходимо запустить в работу.
      Указывается напрвление передачи данных, номер канала и тип работы.
 */
struct dma_start_t {
    size_t controller;      //! DMA channel controller
    size_t direction;       //! DMA channel direction H2C or C2H
    size_t channel;         //! DMA channel number 0, 1, 2, 3
    size_t cycle;           //! DMA channel work mode cycle = 1 - continues mode
};

//-----------------------------------------------------------------------------
/*!
     \brief Остановка канала DMA.

      Структура описывает канал DMA который неоходимо остановить.
      Указывается напрвление передачи данных и номер канала.
 */
struct dma_stop_t {
    size_t controller;      //! DMA channel controller
    size_t direction;       //! DMA channel direction H2C or C2H
    size_t channel;         //! DMA channel number 0, 1, 2, 3
};

//-----------------------------------------------------------------------------
/*!
     \brief Освобождение канала DMA.

      Структура описывает канал DMA ресурсы которого неоходимо освободить.
      Указывается напрвление передачи данных и номер канала. После вызова
      dma_free_t для повторно запуска, необходимо вновь выполнить инициализацию.
 */
struct dma_free_t {
    size_t controller;      //! DMA channel controller
    size_t direction;       //! DMA channel direction H2C or C2H
    size_t channel;         //! DMA channel number 0, 1, 2, 3
};

//-----------------------------------------------------------------------------
/*!
     \brief Ожидание заполнения блока данных каналом DMA.

      После старта, можно ожидать заполнения блока данных, соответствующим
      каналом. При этом поток ожидания не будет потреблять ресурсы процессора.
 */
struct dma_wait_block_t {
    size_t controller;              //! DMA channel controller
    size_t direction;               //! DMA channel direction H2C or C2H
    size_t channel;                 //! DMA channel number 0, 1, 2, 3
    size_t timeout;                 //! Timeout of operation
    size_t completed_block_counter; //! DMA current block
    size_t completed_desc_counter;  //! DMA completed descriptors counter
};

//-----------------------------------------------------------------------------
/*!
     \brief Получение состояния канала DMA.

      В этой структуре возвращается состояние указанного канала DMA.
 */
struct dma_state_t {
    size_t controller;              //! DMA channel controller
    size_t direction;               //! DMA channel direction H2C or C2H
    size_t channel;                 //! DMA channel number 0, 1, 2, 3
    size_t status;                  //! DMA channel status
    size_t completed_block_counter; //! DMA current block
    size_t completed_desc_counter;  //! DMA completed descriptors counter
};

//-----------------------------------------------------------------------------

struct dma_node_t {
    size_t        controller;                   //! DMA channel controller
    size_t        direction;                    //! DMA channel direction H2C or C2H
    size_t        channel;                      //! DMA channel number 0, 1, 2, 3
    char          name[ZYNQ_NODE_NAME_LEN];     //! Имя узла
};

struct dma_info_t {
    size_t              count;
    struct dma_node_t*  nodes;
};

//-----------------------------------------------------------------------------
/*!
     \brief Идентификаторы контроллеров DMA.
 */
enum dma_ctrl_id   {
    DMA_CTRL_0 = 0,
    DMA_CTRL_1,
};

//-----------------------------------------------------------------------------
/*!
     \brief Идентификаторы направления передачи данных каналов DMA.
 */
enum chan_dir   {
    DIR_DMA_MM2S = 0,    //! Direction Host to Card
    DIR_DMA_S2MM,        //! Direction Card to Host
};

//-----------------------------------------------------------------------------
/*!
     \brief Идентификаторы номеров каналов DMA.
 */
enum chan_id    { CHAN0 = 0, CHAN1 = 1, CHAN2 = 2, CHAN3 = 3, };

//-----------------------------------------------------------------------------

#endif //_IOTCLS_H_
