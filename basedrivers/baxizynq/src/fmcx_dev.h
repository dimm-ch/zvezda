#ifndef FMCX_DEV_H
#define FMCX_DEV_H

#include "mapper.h"
#include "ddaxizynq.h"

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>

//-----------------------------------------------------------------------------

struct user_node_t {
    std::string name;
    size_t idx;
    size_t *bar;
    size_t sz;
    user_node_t() {
        name = "empty";
        idx = 0;
        bar = nullptr;
        sz = 0;
    }
};

//-----------------------------------------------------------------------------

class fmcx_dev
{
public:
    fmcx_dev(const std::string& name);
    virtual ~fmcx_dev();

    //bool write_node(uint32_t node, uint32_t offset, uint32_t val);
    //bool read_node(uint32_t node, uint32_t offset, uint32_t& val);

    uint32_t blk_nodes_total() {return _blk_list.size(); }
    const user_node_t& get_blk_node(uint32_t idx);

    uint32_t trd_nodes_total() {return _trd_list.size(); }
    const user_node_t& get_trd_node(uint32_t idx);

    const std::string fmcx_name() { return _name; }
    static void delay(int ms);
    static bool device_exist(unsigned id);
    static bool device_exist(const std::string& name);
    inline int fd() { return _fd; }

protected:
    int _fd = -1;
    unsigned _page_size = {4096};
    std::string _name;
    Mapper _mm;
    std::vector<node_info_t> _nodes_info;   //! Вектор узлов, обнаруженных в драйвере
    std::vector<user_node_t> _blk_list;     //! Ресурсы узлов относящихся к блокам
    std::vector<user_node_t> _trd_list;     //! Ресурсы узлов относящихся к тетрадам
    std::vector<user_node_t> _xln_list;     //! Ресурсы узлов относящихся к компонентам xilinx
    struct user_node_t       _zero_node;    //! Вернем нулевой узел, чтобы не генерировать исключения
    bool _verbose = true;

    void msg(const char *fmt, ...);
    void cleanup();
};

//-----------------------------------------------------------------------------

typedef std::shared_ptr<fmcx_dev> fmcx_dev_t;

inline fmcx_dev_t get_device(const std::string& name)
{
    return std::make_shared<fmcx_dev>(name.c_str());
}

//-----------------------------------------------------------------------------

#endif // FMCX_DEV_H
