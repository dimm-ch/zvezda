#include "brd_dev.h"
#include "time_ipc.h"

//-----------------------------------------------------------------------------
using namespace std;
//-----------------------------------------------------------------------------

brd_dev::brd_dev(unsigned lid)
    : m_lid(lid)
{
    m_hFmc = 0;
    m_hReg = 0;
    m_hBrd = BRD_open(m_lid, BRDopen_SHARED, NULL);
    if (m_hBrd <= 0) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_open(%d, BRDopen_SHARED)!\n", __FILE__, __LINE__, __FUNCTION__, m_lid);
    } else {
        //fprintf(stderr, "%s, %d: %s() - BRD_open(%d, BRDopen_SHARED) - OK\n", __FILE__, __LINE__, __FUNCTION__, m_lid);
        m_info.size = sizeof(BRD_Info);
        S32 err = BRD_getInfo(m_lid, &m_info);
        if (err < 0) {
            fprintf(stderr, "%s, %d: %s() - Error in BRD_getInfo()!\n", __FILE__, __LINE__, __FUNCTION__);
        }
        if (enum_services()) {
            lock_services();
        }
        // uint32_t fid_idx = ~0x0;
        for (uint32_t ii = 0; ii < 8; ++ii) {
            uint32_t id = rh(ii, 0);
            //fprintf(stderr, "%02d: ID = 0x%04X\n", ii, id);
            if ((id & 0xff) == 0x19) {
                m_fid = ii;
            }
        }
    }
}

//-----------------------------------------------------------------------------

brd_dev::~brd_dev()
{
    if (m_hBrd) {
        free_services();
        BRD_close(m_hBrd);
        // fprintf(stderr,"%s, %d: %s() - BRD_close(%d) - OK\n", __FILE__, __LINE__, __FUNCTION__, m_lid);
    }
}

//-----------------------------------------------------------------------------

bool brd_dev::enum_services()
{
    const int size = 32;
    U32 servReal;
    BRD_ServList srvList[size];

    S32 err = BRD_serviceList(m_hBrd, 0, srvList, size, &servReal);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_serviceList()!\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    if ((servReal == 0) || (servReal > size)) {
        fprintf(stderr, "%s, %d: %s() - No service on board: servReal == %d\n", __FILE__, __LINE__, __FUNCTION__, servReal);
        return false;
    }

    for (U32 j = 0; j < servReal; j++) {
#ifdef __linux
        m_list.push_back(srvList[j].name);
#else 
        char name[16] = { 0 };
        BRDC_bcstombs(name, srvList[j].name, 16);
        m_list.push_back(name);
#endif
    }
    //fprintf(stderr, "number of services %d\n", m_list.size());
    return !m_list.empty();
}

//-----------------------------------------------------------------------------

void brd_dev::show_services()
{
    for (U32 j = 0; j < m_list.size(); j++) {
        fprintf(stderr, "Service %d: %s\n", (int)j, m_list.at(j).c_str());
    }
}

//-----------------------------------------------------------------------------

bool brd_dev::lock_services()
{
    for (U32 j = 0; j < m_list.size(); j++) {
        U32 mode = BRDcapt_SHARED;
        const std::string& srvName = m_list.at(j).c_str();
        if (srvName == "REG0") {
#ifdef __linux
            m_hReg = BRD_capture(m_hBrd, 0, &mode, srvName.c_str(), 5000);            
#else
            BRDCHAR srv_name[32];
            BRDC_mbstobcs(srv_name, srvName.c_str(), 32);
            m_hReg = BRD_capture(m_hBrd, 0, &mode, srv_name, 5000);            
#endif
            if (m_hReg <= 0) {
                fprintf(stderr, "%s, %d: %s() - Error in BRD_capture(): %s\n", __FILE__, __LINE__, __FUNCTION__, srvName.c_str());
                return false;
            } else {
                //fprintf(stderr, "%s, %d: %s() - BRD_capture(%s) - OK\n", __FILE__, __LINE__, __FUNCTION__, srvName.c_str());
            }
            m_services.push_back(std::make_pair(srvName, m_hReg));
        }
        if (srvName == "BASEFMC0") {
#ifdef __linux
            m_hFmc = BRD_capture(m_hBrd, 0, &mode, srvName.c_str(), 5000);
#else
            BRDCHAR srv_name[32];
            BRDC_mbstobcs(srv_name, srvName.c_str(), 32);
            m_hFmc = BRD_capture(m_hBrd, 0, &mode, srv_name, 5000);
#endif            
            if (m_hFmc <= 0) {
                fprintf(stderr, "%s, %d: %s() - Error in BRD_capture(): %s\n", __FILE__, __LINE__, __FUNCTION__, srvName.c_str());
                return false;
            } else {
                //fprintf(stderr, "%s, %d: %s() - BRD_capture(%s) - OK\n", __FILE__, __LINE__, __FUNCTION__, srvName.c_str());
            }
            m_services.push_back(std::make_pair(srvName, m_hFmc));
        }
    }

    //if (m_hReg > 0) {
    //    fprintf(stderr, "BLK MAIN - ID 0x%X, DID: 0x%X\n", rh(0, 0x0), rh(0, 0x2));
    //    fprintf(stderr, "TRD MAIN - ID 0x%X, MOD: 0x%X\n", ri(0, 0x100), ri(0, 0x101));
    //}

    return true;
}

//-----------------------------------------------------------------------------

void brd_dev::free_services()
{
    if (m_hReg)
        BRD_release(m_hReg, 0);
    if (m_hFmc)
        BRD_release(m_hFmc, 0);
}

//-----------------------------------------------------------------------------

BRD_Handle brd_dev::lock_service(const std::string& name, U32 mode)
{
    for (U32 j = 0; j < m_list.size(); j++) {
        const std::string& srvName = m_list.at(j).c_str();
        if (srvName == name) {
            BRD_Handle hSrv = BRD_capture(m_hBrd, 0, &mode, (BRDCHAR*)srvName.c_str(), 5000);
            if (hSrv <= 0) {
                fprintf(stderr, "%s, %d: %s() - Error in BRD_capture(): %s\n", __FILE__, __LINE__, __FUNCTION__, srvName.c_str());
                return 0;
            } else {
                //fprintf(stderr, "%s, %d: %s() - BRD_capture(%s) - OK\n", __FILE__, __LINE__, __FUNCTION__, srvName.c_str());
                return hSrv;
            }
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------

void brd_dev::unlock_service(BRD_Handle hSrv)
{
    if (hSrv)
        BRD_release(hSrv, 0);
}

//-----------------------------------------------------------------------------

void brd_dev::wi(S32 trdNo, S32 rgnum, U32 val)
{
    if ((m_hReg <= 0) || (trdNo < 0)) {
        return;
    }

    BRD_Reg params;

    params.bytes = sizeof(U32);
    params.reg = rgnum;
    params.tetr = trdNo;
    params.val = val;

    BRD_ctrl(m_hReg, 0, BRDctrl_REG_WRITEIND, &params);
}

//-----------------------------------------------------------------------------

U32 brd_dev::ri(S32 trdNo, S32 rgnum)
{
    if ((m_hReg <= 0) || (trdNo < 0)) {
        return ~0;
    }

    BRD_Reg params;

    params.bytes = sizeof(U32);
    params.reg = rgnum;
    params.tetr = trdNo;
    params.val = 0;

    BRD_ctrl(m_hReg, 0, BRDctrl_REG_READIND, &params);

    return (params.val & 0xFFFF);
}

//-----------------------------------------------------------------------------

void brd_dev::wd(S32 trdNo, S32 rgnum, U32 val)
{
    if ((m_hReg <= 0) || (trdNo < 0)) {
        return;
    }

    BRD_Reg params;

    params.bytes = sizeof(U32);
    params.reg = rgnum;
    params.tetr = trdNo;
    params.val = val;

    BRD_ctrl(m_hReg, 0, BRDctrl_REG_WRITEDIR, &params);
}

//-----------------------------------------------------------------------------

U32 brd_dev::rd(S32 trdNo, S32 rgnum)
{
    if ((m_hReg <= 0) || (trdNo < 0)) {
        return ~0;
    }

    BRD_Reg params;

    params.bytes = sizeof(U32);
    params.reg = rgnum;
    params.tetr = trdNo;
    params.val = 0;

    BRD_ctrl(m_hReg, 0, BRDctrl_REG_READDIR, &params);

    return params.val;
}

//-----------------------------------------------------------------------------

void brd_dev::wh(S32 blk, S32 rgnum, U32 val)
{
    if (m_hReg <= 0) {
        return;
    }

    BRD_Host host;

    host.blk = blk;
    host.reg = rgnum;
    host.val = val;

    BRD_ctrl(m_hReg, 0, BRDctrl_REG_WRITEHOST, &host);
}

//-----------------------------------------------------------------------------

U32 brd_dev::rh(S32 blk, S32 rgnum)
{
    if (m_hReg <= 0) {
        return ~0;
    }

    BRD_Host host;

    host.blk = blk;
    host.reg = rgnum;

    BRD_ctrl(m_hReg, 0, BRDctrl_REG_READHOST, &host);

    return host.val;
}

//-----------------------------------------------------------------------------

S32 brd_dev::rs(U32 dev, U32 is32bits, U32 num, U32 synchr, S32 trd, S32 reg, U32* pVal)
{
    BRD_Spd ctrl;
    S32 err;

    ctrl.dev = dev;
    ctrl.mode = is32bits;
    ctrl.num = num;
    ctrl.sync = synchr;
    ctrl.tetr = trd;
    ctrl.reg = reg;

    err = BRD_ctrl(m_hReg, 0, BRDctrl_REG_READSPD, &ctrl);
    *pVal = ctrl.val;

    return err;
}

//-----------------------------------------------------------------------------

S32 brd_dev::ws(U32 dev, U32 is32bits, U32 num, U32 synchr, S32 trd, S32 reg, U32 val)
{
    BRD_Spd ctrl;

    ctrl.dev = dev;
    ctrl.mode = is32bits;
    ctrl.num = num;
    ctrl.sync = synchr;
    ctrl.tetr = trd;
    ctrl.reg = reg;
    ctrl.val = val;

    return BRD_ctrl(m_hReg, 0, BRDctrl_REG_WRITESPD, &ctrl);
}

//-----------------------------------------------------------------------------

float brd_dev::board_temperature()
{
    unsigned temp;
    float t = 0;

    RegPokeInd(0, 0x210, 0);
    temp = RegPeekInd(0, 0x211);

    temp >>= 6;
    temp &= 0x3FF;
    t = (temp * 503.975) / 1024 - 273.15;

    return t;
}

//-----------------------------------------------------------------------------

bool brd_dev::get_trd_number(S32 id, S32& number)
{
    bool found = false;
    for (S32 ii = 0; ii < 16; ii++) {
        U32 trd_id = ri(ii, 0x100) & 0xFFFF;
        if ((trd_id == (id & 0xffff)) && (trd_id != 0xFFFF)) {
            found = true;
            number = ii;
            break;
        }
    }
    //if (found)
    //    printf("found tetr%d id=0x%X\n", number, id);
    //else
    //    printf("not found tetr id=0x%X\n", id);
    return found;
}

//-----------------------------------------------------------------------------

bool brd_dev::get_trd_id(S32 number, S32& id)
{
    bool found = false;

    if (number < 16) {

        U32 trd_id = ri(number, 0x100) & 0xFFFF;
        if (trd_id != 0xFFFF) {
            found = true;
            id = trd_id;
        }
    }

    return found;
}

//-----------------------------------------------------------------------------

U32 brd_dev::fid_write(U32 dev, U32 adr_dev, U32 adr_reg, U32 timeout, U32 val)
{
    return 0;
}

//-----------------------------------------------------------------------------

U32 brd_dev::fid_read(U32 dev, U32 adr_dev, U32 adr_reg, U32 timeout, U32& val)
{
    // ipc_time_t t = ipc_get_time
    // uint16_t ctrl = rh(m_fid, 0x16);
    // while (!(ctrl & 0x8000)) {
    // }
    return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

brd_stream::brd_stream(brd_dev_t dev, unsigned id, unsigned dir, unsigned src)
    : m_dev(dev)
    , m_id(id)
    , m_dir(dir)
    , m_srcTrd(src)
{
    std::string srv = "BASESTREAM" + std::to_string(id);
    m_hStrm = m_dev->lock_service(srv, BRDcapt_EXCLUSIVE);
    m_agree = false;
}

//-----------------------------------------------------------------------------

brd_stream::~brd_stream()
{
    cleanup();
    if (m_hStrm)
        m_dev->unlock_service(m_hStrm);
}

//-----------------------------------------------------------------------------

bool brd_stream::prepare(unsigned BlockCount, unsigned BlockSize)
{
    S32 err = 0;

    if (!m_hStrm)
        return false;

    // fprintf(stderr, "%s()\n", __FUNCTION__);

    dma_blocks_t Buffer(BlockCount);

    m_strmBuf.blkNum = BlockCount;
    m_strmBuf.blkSize = BlockSize;
    m_strmBuf.dir = m_dir;
    m_strmBuf.pStub = 0;
    m_strmBuf.isCont = 1;
    m_strmBuf.ppBlk = Buffer.data();

    err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_CBUF_ALLOC, &m_strmBuf);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_STREAM_CBUF_ALLOC)\n", __FILE__, __LINE__, __func__);
        return false;
    }

    m_Stub = m_strmBuf.pStub;

    err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_SETSRC, &m_srcTrd);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_STREAM_SETSRC)\n", __FILE__, __LINE__, __FUNCTION__);
        // return false;
    }

    int x = BRDstrm_DRQ_HALF;
    err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_SETDRQ, &x);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_STREAM_SETDRQ)\n", __FILE__, __LINE__, __FUNCTION__);
        // return false;
    }

    err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_RESETFIFO, NULL);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_STREAM_RESETFIFO)\n", __FILE__, __LINE__, __FUNCTION__);
        // return false;
    }

    fprintf(stderr, "STREAM %d INFO:\n", m_id);
    fprintf(stderr, "blkNum = %d\n", m_strmBuf.blkNum);
    for (unsigned i = 0; i < Buffer.size(); i++) {
        //fprintf(stderr, "%d: %p\n", i, Buffer.at(i));
        memset(Buffer.at(i), 0x77, BlockSize);
    }
    //memset(m_Stub, 0, sizeof(BRDctrl_StreamStub));
    fprintf(stderr, "Stub: %p\n", m_Stub);

    m_blocks = Buffer;

    return true;
}

//-----------------------------------------------------------------------------

bool brd_stream::cleanup()
{
    if (!m_hStrm)
        return false;

    // fprintf(stderr, "%s()\n", __FUNCTION__);

    S32 err = 0;
    err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() STREAM: Error in BRD_ctrl(BRDctrl_STREAM_CBUF_STOP)\n", __FILE__, __LINE__, __FUNCTION__);
        // return false;
    }

    err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_CBUF_FREE, &m_strmBuf);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() STREAM: Error in BRD_ctrl(BRDctrl_STREAM_CBUF_FREE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    m_Stub = 0;

    return true;
}

//-----------------------------------------------------------------------------

bool brd_stream::start(bool cycle)
{
    if (!m_hStrm)
        return false;

    // fprintf(stderr, "%s()\n", __FUNCTION__);

    m_strmStart.isCycle = cycle ? 1 : 0;

    S32 err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_RESETFIFO, NULL);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_STREAM_RESETFIFO)\n", __FILE__, __LINE__, __FUNCTION__);
        // return false;
    }

    err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_CBUF_START, &m_strmStart);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_STREAM_CBUF_START): err = 0x%x\n", __FILE__, __LINE__, __FUNCTION__, err);
        // return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

bool brd_stream::stop()
{
    if (!m_hStrm)
        return false;

    // fprintf(stderr, "%s()\n", __FUNCTION__);

    S32 err = 0;
    err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() STREAM: Error in BRD_ctrl(BRDctrl_STREAM_CBUF_STOP)\n", __FILE__, __LINE__, __FUNCTION__);
        // return false;
    }

    err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_RESETFIFO, NULL);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_STREAM_RESETFIFO)\n", __FILE__, __LINE__, __FUNCTION__);
        // return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_stream::adjust(bool enable)
{
    // fprintf(stderr, "%s()\n", __FUNCTION__);

    BRDctrl_StreamCBufAdjust arg;
    arg.isAdjust = enable ? 1 : 0;
    S32 err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_CBUF_ADJUST, &arg);
    if (!BRD_errcmp(err, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRDctrl_STREAM_CBUF_ADJUST error: 0x%.8X\r\n"), err);
        return false;
    }
    m_agree = enable;
    if (m_agree) {
        BRDC_printf(_BRDC("Handshake mode: ON\r\n"));
    } else {
        BRDC_printf(_BRDC("Handshake mode: OFF\r\n"));
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_stream::done(unsigned block_num)
{
    BRDctrl_StreamCBufDone arg;
    arg.blkNo = block_num;
    S32 err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_CBUF_DONE, &arg);
    if (!BRD_errcmp(err, BRDerr_OK)) {
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_stream::state(unsigned& last_block, unsigned& total_block)
{
    if (!m_hStrm)
        return false;

    if (m_Stub) {
        if (m_Stub->state == BRDstrm_STAT_RUN) {
            last_block = m_Stub->lastBlock;
            total_block = m_Stub->totalCounter;
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------

bool brd_stream::wait_block(unsigned& last_block, unsigned& total_block, unsigned timeout)
{
    if (!m_hStrm)
        return false;

    if (m_Stub) {
        m_wait.timeout = timeout; // ждать окончания сбора данных до msTimeout мсек.
        S32 status = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_CBUF_WAITBLOCK, &m_wait);
        if (BRD_errcmp((U32)status, BRDerr_WAIT_TIMEOUT)) {
            fprintf(stderr,"STREAM %d - timeuot!\n", m_id);
            return false;
        } else {
            last_block = m_Stub->lastBlock;
            total_block = m_Stub->totalCounter;
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------

bool brd_stream::wait_buffer(unsigned& last_block, unsigned& total_block, unsigned timeout)
{
    if (!m_hStrm)
        return false;

    if (m_Stub) {
        m_wait.timeout = timeout; // ждать окончания сбора данных до msTimeout мсек.
        S32 status = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_CBUF_WAITBUFEX, &m_wait);
        if (BRD_errcmp((U32)status, BRDerr_WAIT_TIMEOUT)) {
            // fprintf(stderr,"STREAM %d - timeuot!\n", m_id);
            return false;
        } else {
            last_block = m_Stub->lastBlock;
            total_block = m_Stub->totalCounter;
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------

bool brd_stream::stub(BRDctrl_StreamStub** stub)
{
    if (!m_hStrm)
        return false;

    if (m_Stub) {
        if (stub) {
            *stub = m_Stub;
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------

BRDctrl_StreamStub* brd_stream::stub()
{
    if (!m_hStrm)
        return nullptr;
    return m_Stub;
}

//-----------------------------------------------------------------------------

void* brd_stream::block(unsigned id)
{
    if (!m_hStrm)
        return nullptr;
    if (id >= m_blocks.size())
        return nullptr;
    return m_blocks.at(id);
}

//-----------------------------------------------------------------------------

bool brd_stream::set_source(unsigned src_trd)
{
    S32 err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_SETSRC, &m_srcTrd);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_STREAM_SETSRC)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    m_srcTrd = src_trd;
    return true;
}

//-----------------------------------------------------------------------------

bool brd_stream::set_drq_flag(unsigned flag)
{
    U32 drq_flag = flag;
    S32 err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_SETDRQ, &drq_flag);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_STREAM_SETDRQ)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_stream::reset_fifo()
{
    S32 err = BRD_ctrl(m_hStrm, 0, BRDctrl_STREAM_RESETFIFO, 0);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_STREAM_RESETFIFO)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

brd_sdram::brd_sdram(brd_dev_t dev, unsigned id)
    : m_dev(dev)
    , m_id(id)
{
    std::string srv = "BASESDRAM" + std::to_string(id);
    m_hSDRAM = m_dev->lock_service(srv, BRDcapt_EXCLUSIVE);
    if (m_hSDRAM) {
        config();
        S32 err = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_GETSRCSTREAM, &m_src);
        if (0 > err) {
            fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_SDRAM_GETSRCSTREAM)\n", __FILE__, __LINE__, __FUNCTION__);
            return;
        }
        fprintf(stderr, "%s, %d: %s() - TRD SDRAM: 0x%x\n", __FILE__, __LINE__, __FUNCTION__, m_src);
    }
}

//-----------------------------------------------------------------------------

brd_sdram::~brd_sdram()
{
    if (m_hSDRAM)
        m_dev->unlock_service(m_hSDRAM);
}

//-----------------------------------------------------------------------------

bool brd_sdram::prepare(uint32_t size, U32 mode)
{
    U32 nFifoMode = mode;
    S32 err = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_SETFIFOMODE, &nFifoMode);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_SDRAM_SETFIFOMODE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    // set size active zone (U32)
    uint32_t nSize = size / sizeof(uint32_t);
    err = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_SETMEMSIZE, &nSize);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_SDRAM_SETMEMSIZE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

bool brd_sdram::enable()
{
    U32 enable = 1;
    S32 err = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_ENABLE, &enable);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_SDRAM_ENABLE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_sdram::disable()
{
    U32 enable = 0;
    S32 err = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_ENABLE, &enable);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_SDRAM_DISABLE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_sdram::reset_fifo()
{
    S32 err = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_FIFORESET, 0);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_SDRAM_FIFORESET)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

uint32_t brd_sdram::status()
{
    U32 status = 0;
    S32 err = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_FIFOSTATUS, &status);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_SDRAM_FIFOSTATUS)\n", __FILE__, __LINE__, __FUNCTION__);
        return 0x0;
    }
    return status;
}

//-----------------------------------------------------------------------------

uint32_t brd_sdram::get_acq_size()
{
    U32 size = 0;
    S32 err = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_GETACQSIZE, &size);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_SDRAM_GETACQSIZE)\n", __FILE__, __LINE__, __FUNCTION__);
        return 0x0;
    }
    return size;
}

//-----------------------------------------------------------------------------

bool brd_sdram::wait_complete()
{
    U32 status = 0;
    while (!status) {
        S32 err = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_ISACQCOMPLETE, &status);
        if (0 > err) {
            fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_SDRAM_ISACQCOMPLETE)\n", __FILE__, __LINE__, __FUNCTION__);
            return false;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_sdram::clear_flag()
{
    S32 err = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_FLAGCLR, 0);
    if (0 > err) {
        fprintf(stderr, "%s, %d: %s() - Error in BRD_ctrl(BRDctrl_SDRAM_FLAGCLR)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_sdram::set_full_rate()
{
    m_dev->wi(m_src, 0xB, 0x22);
    return true;
}

//-----------------------------------------------------------------------------

bool brd_sdram::config()
{
    // проверяем наличие динамической памяти
    m_SdramConfig.Size = sizeof(BRD_SdramCfgEx);
    m_PhysMemSize = 0;

    S32 status = BRD_ctrl(m_hSDRAM, 0, BRDctrl_SDRAM_GETCFGEX, &m_SdramConfig);
    if (status < 0) {
        fprintf(stderr, "%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_SDRAM_GETCFGEX)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    } else {

        if (m_SdramConfig.MemType == 11) {
            m_PhysMemSize = (unsigned)(((((__int64)m_SdramConfig.CapacityMbits * 1024 * 1024) >> 3) * (__int64)m_SdramConfig.PrimWidth / m_SdramConfig.ChipWidth * m_SdramConfig.ModuleBanks * m_SdramConfig.ModuleCnt) >> 2); // в 32-битных словах
        } else {
            m_PhysMemSize = (1 << m_SdramConfig.RowAddrBits) * (1 << m_SdramConfig.ColAddrBits) * m_SdramConfig.ModuleBanks * m_SdramConfig.ChipBanks * m_SdramConfig.ModuleCnt * 2; // в 32-битных словах
        }
    }

    // динамическая память присутствует на модуле
    BRDC_printf(_BRDC("SDRAM Config: Memory size = %d MBytes\n"), (m_PhysMemSize / (1024 * 1024)) * 4);

    return true;
}

//-----------------------------------------------------------------------------
#if 0
bool brd_dev::memory_as_fifo()
{
    // проверяем наличие динамической памяти
    unsigned PhysMemSize = 0;
    if(!memory_config(PhysMemSize)) {
        return false;
    }

    S32 status = 0;

    if(PhysMemSize) {

        // установить параметры SDRAM
        ULONG target = 2; // будем осуществлять сбор данных в память
        status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_SETTARGET, &target);

        ULONG fifo_mode = 1; // память используется как FIFO
        status = BRD_ctrl(m_hAdc, 0, BRDctrl_SDRAM_SETFIFOMODE, &fifo_mode);

        BRDC_printf(_BRDC("SDRAM as a FIFO mode!!!\n"));
    } else {
        return false;
    }

    status = BRD_ctrl(m_hAdc, 0, BRDctrl_SDRAM_GETSRCSTREAM, &m_memTrd);
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_SDRAM_GETSRCSTREAM)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    BRDC_printf(_BRDC("MEM TRDID: %d\n"), m_memTrd);

    return true;
}

//-----------------------------------------------------------------------------

bool brd_dev::start_memory()
{
    fprintf(stderr, "%s()\n", __FUNCTION__);

    S32 status = BRD_ctrl(m_hAdc, 0, BRDctrl_SDRAM_FIFORESET, NULL);  // сброс FIFO SDRAM
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_SDRAM_FIFORESET)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    ULONG Enable = 1;
    status = BRD_ctrl(m_hAdc, 0, BRDctrl_SDRAM_ENABLE, &Enable); // разрешение работы SDRAM
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_SDRAM_ENABLE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_dev::stop_memory()
{
    fprintf(stderr, "%s()\n", __FUNCTION__);

    ULONG Enable = 0;
    S32 status = BRD_ctrl(m_hAdc, 0, BRDctrl_SDRAM_ENABLE, &Enable); // запрещение работы SDRAM
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_SDRAM_ENABLE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    status = BRD_ctrl(m_hAdc, 0, BRDctrl_SDRAM_FIFORESET, NULL);  // сброс FIFO SDRAM
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_SDRAM_FIFORESET)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_dev::start_adc()
{
    fprintf(stderr, "%s()\n", __FUNCTION__);

    S32 status = 0;

    BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_FIFORESET, NULL);  // сброс FIFO АЦП
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_FIFORESET)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    ULONG Enable = 1;

    status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_ENABLE, &Enable); // разрешение работы АЦП
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_ENABLE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_dev::stop_adc()
{
    ULONG Enable = 0;

    fprintf(stderr, "%s()\n", __FUNCTION__);

    S32 err = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_ENABLE, &Enable); // запрещение работы АЦП
    if( 0>err ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_ENABLE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    err = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_FIFORESET, NULL);  // сброс FIFO АЦП
    if( 0>err ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_FIFORESET)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool brd_dev::prepare_adc()
{
    FM212x1GSRV_MU	rSetMu =
    {
        sizeof(FM212x1GSRV_MU),	// size;
        0,          //	subType;
        m_app_params.MasterMode,            //  master;
        m_app_params.ChannelMask,           //	chanMask;
        m_app_params.ClockSource,           //	clockSrc;
        m_app_params.SamplingRate,          //	samplingRate;
        m_app_params.ExternalClockValue,	//	clockValue;

        {0,0},      //	gaindb[BRD_CHANCNT];
        {0,0.0},    //	bias[BRD_CHANCNT];
        {0,0},      //	inpResist[BRD_CHANCNT];
        {0,0},      //	dcCoupling[BRD_CHANCNT];
        0,          // 	format;

        {           //	adcStMode;
                    0,                  //  src
                    0,                  //  inv
                    0.0,                //  level
                    {
                        0,	//  startSrc
                        0,	//  startInv
                        1,	//  trigOn
                        0,	//  trigStopSrc
                        0,	//  stopInv
                        0	//  reStartMode
                    }
        },
        0,          //	stResist;

        0,          //	pretrigMode;
        0,          //	pretrigSamples;

        0,          //	cnt0Value;
        0,          //	cnt1Value;
        0,          //	cnt2Value;
        0,          //	cnt0Enable;
        0,          //	cnt1Enable;
        0,          //  cnt2Enable;

        0,          //	titleEnable;
        0,          //	titleSize;
        0,          //	titleData;

        {           //  dblClk
                    sizeof(FM212x1GSRV_DBLCLK),
                    0,		//  isDblClk
                    0,		//  inpSrc
                    0,		//  valRange
                    0.0,	//  inp0Range
                    0.0,	//  inp1Range
                    0.0,	//  inp0Bias
                    0.0,	//  inp1Bias
                    {
                        0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0
                    }
        }
    };

    memcpy(&m_adcParam, &rSetMu, sizeof(m_adcParam));

    S32 status = BRD_ctrl(m_hAdc, NODE0, BRDctrl_ADC_SETMU, &rSetMu);
    if(0 > status) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_SETMU)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    ULONG format = 0;
    status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_GETFORMAT, &format);
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_GETFORMAT)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_FIFORESET, NULL);  // сброс FIFO SDRAM
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_FIFORESET)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    ULONG sample_size = format ? format : sizeof(short);
    if(format == 0x80) // упакованные 12-разрядные данные
        sample_size = 2;
    m_sample_size = sample_size;

    status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_GETSRCSTREAM, &m_adcTrd);
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_GETSRCSTREAM)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    set_adc_param();

    BRDC_printf(_BRDC("ADC TRDID: %d\n"), m_adcTrd);

    return true;
}

//-----------------------------------------------------------------------------

bool brd_dev::set_adc_start_mode(bool external)
{
    BRD_AdcStartMode adcStart;

    adcStart.stndStart.reStartMode = m_app_params.ReStart;
    adcStart.stndStart.startInv = m_app_params.StartBaseInverting;
    adcStart.stndStart.startSrc = m_app_params.StartBaseSource;
    adcStart.stndStart.stopInv = m_app_params.StopInverting;
    adcStart.stndStart.trigOn = m_app_params.StartMode;
    adcStart.stndStart.trigStopSrc = m_app_params.StopSource;

    adcStart.src = m_app_params.StartSource;
    adcStart.inv = m_app_params.StartInverting;
    adcStart.level = m_app_params.StartLevel;

    S32 status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_SETSTARTMODE, &adcStart);
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_SETSTARTMODE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    IPC_delay(100);

    return true;
}

//-----------------------------------------------------------------------------

bool brd_dev::set_adc_param()
{
    double gain[2] = { m_app_params.GainDb0, m_app_params.GainDb1 };

    for(unsigned i=0; i<2; i++) {

        BRD_ValChan valChan;
        valChan.chan = i;
        valChan.value = gain[i];

        S32 status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_SETGAIN, &valChan);
        if( 0>status ) {
            fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_SETGAIN)\n", __FILE__, __LINE__, __FUNCTION__);
            return false;
        }
    }

    u8 bias[2] = { m_app_params.Bias0, m_app_params.Bias1 };

    for(unsigned i=0; i<2; i++) {

        BRD_ValChan valChan;
        valChan.chan = i;
        valChan.value = bias[i];

        S32 status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_SETBIAS, &valChan);
        if( 0>status ) {
            fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_SETBIAS)\n", __FILE__, __LINE__, __FUNCTION__);
            return false;
        }
    }

    U32 mask = m_app_params.ChannelMask;
    S32 status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_SETCHANMASK, &mask);
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_SETCHANMASK)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

bool brd_dev::set_adc_frequency(unsigned freq, unsigned stdelay)
{
    /*
    BRD_ClkMode clk;
    clk.src = 0x82;
    clk.value = freq;
*/
    BRD_SyncMode sync;
    sync.clkSrc = m_app_params.ClockSource;
    sync.clkValue = (m_app_params.ClockSource > 4) ? m_app_params.ExternalClockValue : ((m_app_params.ClockSource == 2) ? m_app_params.SubClockValue : m_app_params.BaseClockValue);
    sync.rate = freq;

    fprintf(stderr, "%s()\n", __FUNCTION__);

    //S32 status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_SETCLKMODE, &clk);
    S32 status = BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_SETSYNCMODE, &sync);
    if( 0>status ) {
        fprintf(stderr,"%s, %d: %s() - Erorr in BRD_ctrl(BRDctrl_ADC_SETSTARTMODE)\n", __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

    set_delay(8, stdelay);

    return true;
}

//-----------------------------------------------------------------------------

void brd_dev::program_counters(uint32_t data0, uint32_t data1, uint32_t cnt0)
{
    bool single_zone = (data0 == 512) ? true : false;

    // align on FPGA samples
    data0 >>= 5;
    data0 -=  2;
    data1 >>= 5;

    if(single_zone) {
        RegPokeInd(m_adcTrd, 0x2E1, ((0x1 << 9) | (0x1 << 7)));  // enable CNT1, and HEADERS (bit 7)
    } else {
        RegPokeInd(m_adcTrd, 0x2E1, ((0x3 << 9) | (0x1 << 7)));  // enable CNT1, CNT2, and HEADERS (bit 7)
    }

    // CNT0
    unsigned cnt0_val = cnt0 < 2 ? 2 : cnt0;

    if(single_zone) {

        RegPokeInd(m_adcTrd, 0x2E2, cnt0_val);
        RegPokeInd(m_adcTrd, 0x2E3, cnt0_val >> 16);

        // program CNT1 data
        data1 += (data0+1);
        RegPokeInd(m_adcTrd, 0x2E4, data1);
        RegPokeInd(m_adcTrd, 0x2E5, data1 >> 16);

        // program CNT1 skip
        RegPokeInd(m_adcTrd, 0x2E6, 2);
        RegPokeInd(m_adcTrd, 0x2E7, 0);

    } else {

        // program CNT0 delay
        RegPokeInd(m_adcTrd, 0x2E2, 2);
        RegPokeInd(m_adcTrd, 0x2E3, 0);

        // program CNT1 data
        RegPokeInd(m_adcTrd, 0x2E4, data0);
        RegPokeInd(m_adcTrd, 0x2E5, data0 >> 16);

        // program CNT1 skip
        RegPokeInd(m_adcTrd, 0x2E6, cnt0_val);
        RegPokeInd(m_adcTrd, 0x2E7, cnt0_val >> 16);

        // program CNT2 data
        RegPokeInd(m_adcTrd, 0x2E8, data1);
        RegPokeInd(m_adcTrd, 0x2E9, data1 >> 16);

        // program CNT2 skip
        RegPokeInd(m_adcTrd, 0x2EA, 2);
        RegPokeInd(m_adcTrd, 0x2EB, 0);
    }

    BRD_ctrl(m_hAdc, 0, BRDctrl_ADC_FIFORESET, NULL);  // сброс FIFO АЦП
}

//-----------------------------------------------------------------------------

void brd_dev::enable_psp(bool enable)
{
    if(enable)
        RegPokeInd(m_adcTrd, 0xC, 0x100);
    else
        RegPokeInd(m_adcTrd, 0xC, 0x0);

}

//-----------------------------------------------------------------------------

void brd_dev::set_delay(unsigned id, unsigned delay)
{
    U32 val = (id << 8) | delay;
    RegPokeInd(m_adcTrd, 0x2F4, val);
    val |= 0x4000;
    RegPokeInd(m_adcTrd, 0x2F4, val);
    U32 rval = RegPeekInd(m_adcTrd, 0x212);
    fprintf(stderr,"%s() ID: %d, WRITE: 0x%x READ: 0x%x\n", __FUNCTION__, id, val, rval);
}

//-----------------------------------------------------------------------------

void brd_dev::set_params(struct app_params_t *params)
{
    if(params) {
        m_app_params = *params;
    }
}

//-----------------------------------------------------------------------------
#endif
