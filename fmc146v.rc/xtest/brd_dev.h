#ifndef BRD_DEV_H
#define BRD_DEV_H

#include "gipcy.h"
#include "brd.h"
#include "ctrlreg.h"
#include "exceptinfo.h"
#include "ctrlstrm.h"
#include "ctrlsdram.h"

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>


//-----------------------------------------------------------------------------

class brd_dev
{

public:
    explicit brd_dev(unsigned lid);
    virtual ~brd_dev();
    bool enum_services();
    void show_services();
    bool lock_services();
    void free_services();
    BRD_Handle lock_service(const std::string& name, U32 mode);
    void unlock_service(BRD_Handle hSrv);

    void wi(S32 trdNo, S32 rgnum, U32 val);
    U32 ri(S32 trdNo, S32 rgnum);
    void wd(S32 trdNo, S32 rgnum, U32 val);
    U32 rd(S32 trdNo, S32 rgnum);
    void wh(S32 blk, S32 rgnum, U32 val);
    U32 rh(S32 blk, S32 rgnum);
    S32	rs(U32 dev, U32 is32bits, U32 num, U32 synchr, S32 trd, S32 reg, U32 *pVal);
    S32	ws(U32 dev, U32 is32bits, U32 num, U32 synchr, S32 trd, S32 reg, U32 val);

    inline void RegPokeInd(S32 trdNo, S32 rgnum, U32 val) { return wi(trdNo, rgnum, val); }
    inline U32 RegPeekInd(S32 trdNo, S32 rgnum) { return ri(trdNo, rgnum); }
    inline void RegPokeDir(S32 trdNo, S32 rgnum, U32 val) { return wd(trdNo, rgnum, val); }
    inline U32 RegPeekDir(S32 trdNo, S32 rgnum) { return rd(trdNo, rgnum); }

    U32 fid_write(U32 dev, U32 adr_dev, U32 adr_reg, U32 timeout, U32 val);
    U32 fid_read(U32 dev, U32 adr_dev, U32 adr_reg, U32 timeout, U32& val);

    const BRD_Info& board_info() { return m_info; }
    unsigned board_bus() { return m_info.bus; }
    unsigned board_lid() { return m_lid; }
    float board_temperature();
    bool is_valid() {
        if(m_hBrd != 0)
            return true;
        return false;
    }
    bool get_trd_number(S32 id, S32& number);
    bool get_trd_id(S32 number, S32& id);
    BRD_Handle get_brd_handle() { return m_hBrd; }

private:
    unsigned m_lid = {0};
    std::vector<std::string> m_list;
    BRD_Handle  m_hBrd = {0};
    BRD_Handle  m_hReg;
    BRD_Info    m_info;
    uint32_t    m_fid = {0};
    brd_dev();
};

//-----------------------------------------------------------------------------

using brd_dev_t = std::shared_ptr<brd_dev>;
inline brd_dev_t get_device(unsigned lid) { return std::make_shared<brd_dev>(lid); }

//#############################################################################
//#############################################################################
//#############################################################################

typedef std::vector<void*> dma_blocks_t;

//-----------------------------------------------------------------------------

class brd_stream {
public:
    brd_stream(brd_dev_t dev, unsigned id, unsigned dir, unsigned src);
    virtual ~brd_stream();

    bool prepare(unsigned Count, unsigned  BlockSize);
    bool cleanup();
    bool start(bool cycle);
    bool stop();
    bool adjust(bool enable);
    bool done(unsigned block_num);
    bool state(unsigned& last_block, unsigned& total_block);
    bool wait_block(unsigned& last_block, unsigned& total_block, unsigned timeout);
    bool wait_buffer(unsigned& last_block, unsigned& total_block, unsigned timeout);
    bool stub(BRDctrl_StreamStub** stub);
    BRDctrl_StreamStub* stub();
    void* block(unsigned id);
    bool set_source(unsigned src_trd);
    bool set_drq_flag(unsigned flag);
    bool reset_fifo();
    unsigned id() { return m_id; }

private:
    brd_stream();
    brd_dev_t                   m_dev;
    unsigned                    m_id;
    unsigned                    m_dir;
    unsigned                    m_srcTrd;
    BRD_Handle                  m_hStrm;
    BRDctrl_StreamCBufAlloc     m_strmBuf;
    BRDctrl_StreamCBufStart     m_strmStart;
    BRDctrl_StreamStub*         m_Stub;
    BRDctrl_StreamCBufWaitBlock m_wait;
    dma_blocks_t                m_blocks;
    bool                        m_agree;
};

//-----------------------------------------------------------------------------

using stream_t = std::shared_ptr<brd_stream>;
inline stream_t get_stream(brd_dev_t dev, unsigned id, unsigned dir, unsigned src) { return std::make_shared<brd_stream>(dev, id, dir, src); }

//#############################################################################
//#############################################################################
//#############################################################################

class brd_sdram {
public:
    brd_sdram(brd_dev_t dev, unsigned id);
    virtual ~brd_sdram();
    bool prepare(uint32_t size);
    bool info();
    bool enable();
    bool disable();
    bool reset_fifo();
    bool wait_complete();
    bool clear_flag();
    uint32_t status();
    uint32_t source_id() { return m_src; }
    uint32_t get_acq_size();
    bool set_full_rate();
    bool config();

private:
    brd_sdram() {}
    brd_dev_t                   m_dev;
    BRD_Handle                  m_hSDRAM;
    unsigned					m_id;
    unsigned                    m_src;
    unsigned 					m_PhysMemSize;
    BRD_SdramCfgEx 				m_SdramConfig;
};

//-----------------------------------------------------------------------------

using sdram_t = std::shared_ptr<brd_sdram>;
inline sdram_t get_sdram(brd_dev_t dev, unsigned id) { return std::make_shared<brd_sdram>(dev, id); }

//-----------------------------------------------------------------------------


#endif // BRD_DEV_H
