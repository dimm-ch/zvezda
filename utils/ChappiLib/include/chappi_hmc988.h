///
/// \file hmc988.h
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 04.07.2019
///
/// \copyright InSys Copyright (c) 2019
///

#pragma once

#include "chappi_base.h"
#include "chappi_register.h"

namespace chappi {

enum class hmc988_divide_ratio : uint16_t {
    no_div,
    div2,
    div4,
    div8,
    div16,
    div32
};

enum class hmc988_tx_buffer_swing : uint16_t {
    single_ended_600mVpp,
    single_ended_700mVpp,
    single_ended_800mVpp,
    single_ended_900mVpp
};

enum class hmc988_gpio_select : uint16_t {
    _0,
    _1,
    slip_req,
    __0,
    sync_req,
    sync_delayed,
    post_sync,
    spare
};

enum class hmc988_output_launch_phase : uint16_t {
    falling_edge,
    rising_edge
};

enum class hmc988_gpo_force_mode : uint16_t {
    on_gpo_only,
    on_sdo_only,
    to_hiz
};

struct hmc988_gpo_force {
    hmc988_gpo_force_mode mode;
    bool enabled;
};

namespace hmc988_registers {

    using register_type = uint16_t;
    using register_addr_type = uint16_t;
    template <typename register_bits_type, register_addr_type register_addr>
    using register_abstract = ::chappi::register_abstract<register_type, register_bits_type, register_addr_type, register_addr>;

#pragma pack(push, 1)

    union register_bits_00h {
        struct {
            register_type read_control : 4;
            register_type soft_reset : 1;
        };
        register_type chip_id;
    };

    struct register_bits_01h {
        bool master_chip_enable : 1;
        bool rx_buffer_enable : 1;
        bool divider_core_enable : 1;
        bool output_buffer_enable : 1;
    };

    struct register_bits_02h {
        hmc988_divide_ratio divide_ratio_select : 3;
    };

    struct register_bits_03h {
        register_type : 4;
        hmc988_tx_buffer_swing tx_buffer_swing_select : 2;
        register_type sync_delay_adj : 3;
    };

    struct register_bits_04h {
        bool broadcast_mode : 1;
        bool external_sync_pin_en : 1;
        bool external_slip_pin_en : 1;
        bool rx_buffer_dc_bias_select : 1;
        bool delay_line_enable : 1;
        bool on_chip_regulator_bypass : 1;
    };

    struct register_bits_05h {
        hmc988_gpio_select gpio_select : 3;
        register_type force_gpo_on_gpo : 1;
        register_type force_gpo_on_sdo : 1;
        register_type force_gpo_hiz : 1;
    };

    struct register_bits_06h {
        register_type spi_sync_signal : 1;
        register_type spi_slip_signal : 1;
        hmc988_output_launch_phase output_launch_phase : 1;
    };

    struct register_bits_07h {
        register_type delay_line_setpoint : 6;
    };

#pragma pack(pop)

    using register_00h = register_abstract<register_bits_00h, 0x00>;
    using register_01h = register_abstract<register_bits_01h, 0x01>;
    using register_02h = register_abstract<register_bits_02h, 0x02>;
    using register_03h = register_abstract<register_bits_03h, 0x03>;
    using register_04h = register_abstract<register_bits_04h, 0x04>;
    using register_05h = register_abstract<register_bits_05h, 0x05>;
    using register_06h = register_abstract<register_bits_06h, 0x06>;
    using register_07h = register_abstract<register_bits_07h, 0x07>;

}

namespace detail {
    struct hmc988_counter : chips_counter<hmc988_counter> {
    };
} // namespace detail

template <typename ErrorType = int, ErrorType NoerrorValue = 0,
    typename DevAddrType = uint16_t, typename AddrType = uint16_t, typename ValueType = uint16_t>
class hmc988 final : public chip_base<ErrorType, NoerrorValue, DevAddrType, AddrType, ValueType> {
    static constexpr auto _chip_name = "HMC988";
    detail::hmc988_counter _counter;

    static constexpr int _addr_offset { 3 };
    mutable uint8_t _delay_line_setpoint { 4 }; // FIXME: микросхема возвращает значение с 111 в младших разрядах

public:
    CHIP_BASE_RESOLVE
    hmc988(bool log_enable)
        : hmc988 { (log_enable) ? std::clog.rdbuf() : nullptr }
    {
    }
    hmc988(std::streambuf* buf_ptr = {}, reg_read_fn reg_read = {}, reg_write_fn reg_write = {}, dev_addr_type dev_addr = {})
        : chip_base<error_type, NoerrorValue, dev_addr_type, addr_type, value_type> { buf_ptr, reg_read, reg_write, dev_addr }
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    ~hmc988() noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    int get_num() const noexcept final { return _counter.get_num(); }
    int get_counts() const noexcept final { return _counter.get_counts(); }
    std::string get_name() const noexcept final { return get_name(_chip_name, get_num()); }
    void reset() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        hmc988_registers::register_00h reg {};
        reg.data.bits.soft_reset = true;
        _write(reg);
    }
    void reset(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<hmc988, error_type, NoerrorValue, &hmc988::reset>(this, error);
    }
    void read_id(value_type& id) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        hmc988_registers::register_00h reg {};
        _write(reg);
        _read(reg);
        id = reg.data.bits.chip_id;
    }
    value_type read_id() const
    {
        value_type id {};
        read_id(id);
        return id;
    }
    value_type read_id(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, value_type, &hmc988::read_id>(this, error);
    }
    void chip_enable(bool enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_01h reg_01h {};
        reg_00h.data.bits.read_control = reg_01h.addr;
        _write(reg_00h);
        _read(reg_01h);
        reg_01h.data.bits.master_chip_enable = enabled;
        _write(reg_01h);
    }
    void chip_enable(bool enabled, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc988, error_type, NoerrorValue, bool, &hmc988::chip_enable>(this, enabled, error);
    }
    void is_enabled(bool& enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_01h reg_01h {};
        reg_00h.data.bits.read_control = reg_01h.addr;
        _write(reg_00h);
        _read(reg_01h);
        enabled = reg_01h.data.bits.master_chip_enable;
    }
    bool is_enabled() const
    {
        bool enabled {};
        is_enabled(enabled);
        return enabled;
    }
    bool is_enabled(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, bool, &hmc988::is_enabled>(this, error);
    }
    void rx_buffer_enable(bool enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_01h reg_01h {};
        reg_00h.data.bits.read_control = reg_01h.addr;
        _write(reg_00h);
        _read(reg_01h);
        reg_01h.data.bits.rx_buffer_enable = enabled;
        _write(reg_01h);
    }
    void rx_buffer_enable(bool enabled, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc988, error_type, NoerrorValue, bool, &hmc988::rx_buffer_enable>(this, enabled, error);
    }
    void is_rx_buffer_enabled(bool& enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_01h reg_01h {};
        reg_00h.data.bits.read_control = reg_01h.addr;
        _write(reg_00h);
        _read(reg_01h);
        enabled = reg_01h.data.bits.rx_buffer_enable;
    }
    bool is_rx_buffer_enabled() const
    {
        bool enabled {};
        is_rx_buffer_enabled(enabled);
        return enabled;
    }
    bool is_rx_buffer_enabled(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, bool, &hmc988::is_rx_buffer_enabled>(this, error);
    }
    void output_buffer_enable(bool enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_01h reg_01h {};
        reg_00h.data.bits.read_control = reg_01h.addr;
        _write(reg_00h);
        _read(reg_01h);
        reg_01h.data.bits.output_buffer_enable = enabled;
        _write(reg_01h);
    }
    void output_buffer_enable(bool enabled, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc988, error_type, NoerrorValue, bool, &hmc988::output_buffer_enable>(this, enabled, error);
    }
    void is_output_buffer_enabled(bool& enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_01h reg_01h {};
        reg_00h.data.bits.read_control = reg_01h.addr;
        _write(reg_00h);
        _read(reg_01h);
        enabled = reg_01h.data.bits.output_buffer_enable;
    }
    bool is_output_buffer_enabled() const
    {
        bool enabled {};
        is_output_buffer_enabled(enabled);
        return enabled;
    }
    bool is_output_buffer_enabled(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, bool, &hmc988::is_output_buffer_enabled>(this, error);
    }
    void divider_core_enable(bool enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_01h reg_01h {};
        reg_00h.data.bits.read_control = reg_01h.addr;
        _write(reg_00h);
        _read(reg_01h);
        reg_01h.data.bits.divider_core_enable = enabled;
        _write(reg_01h);
    }
    void divider_core_enable(bool enabled, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc988, error_type, NoerrorValue, bool, &hmc988::divider_core_enable>(this, enabled, error);
    }
    void is_divider_core_enabled(bool& enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_01h reg_01h {};
        reg_00h.data.bits.read_control = reg_01h.addr;
        _write(reg_00h);
        _read(reg_01h);
        enabled = reg_01h.data.bits.divider_core_enable;
    }
    bool is_divider_core_enabled() const
    {
        bool enabled {};
        is_divider_core_enabled(enabled);
        return enabled;
    }
    bool is_divider_core_enabled(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, bool, &hmc988::is_divider_core_enabled>(this, error);
    }
    void delay_line_enable(bool enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_04h reg_04h {};
        reg_00h.data.bits.read_control = reg_04h.addr;
        _write(reg_00h);
        _read(reg_04h);
        reg_04h.data.bits.delay_line_enable = enabled;
        _write(reg_04h);
    }
    void delay_line_enable(bool enabled, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc988, error_type, NoerrorValue, bool, &hmc988::delay_line_enable>(this, enabled, error);
    }
    void is_delay_line_enable(bool& enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_04h reg_04h {};
        reg_00h.data.bits.read_control = reg_04h.addr;
        _write(reg_00h);
        _read(reg_04h);
        enabled = reg_04h.data.bits.delay_line_enable;
    }
    bool is_delay_line_enable() const
    {
        bool enabled {};
        is_delay_line_enable(enabled);
        return enabled;
    }
    bool is_delay_line_enable(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, bool, &hmc988::is_delay_line_enable>(this, error);
    }
    void regulator_bypass(bool bypassed) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_04h reg_04h {};
        reg_00h.data.bits.read_control = reg_04h.addr;
        _write(reg_00h);
        _read(reg_04h);
        reg_04h.data.bits.on_chip_regulator_bypass = bypassed;
        _write(reg_04h);
    }
    void regulator_bypass(bool bypassed, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc988, error_type, NoerrorValue, bool, &hmc988::regulator_bypass>(this, bypassed, error);
    }
    void is_regulator_bypass(bool& bypassed) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_04h reg_04h {};
        reg_00h.data.bits.read_control = reg_04h.addr;
        _write(reg_00h);
        _read(reg_04h);
        bypassed = reg_04h.data.bits.on_chip_regulator_bypass;
    }
    bool is_regulator_bypass() const
    {
        bool bypassed {};
        is_regulator_bypass(bypassed);
        return bypassed;
    }
    bool is_regulator_bypass(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, bool, &hmc988::is_regulator_bypass>(this, error);
    }
    void set_divide_ratio(hmc988_divide_ratio ratio) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_02h reg_02h {};
        reg_00h.data.bits.read_control = reg_02h.addr;
        _write(reg_00h);
        _read(reg_02h);
        reg_02h.data.bits.divide_ratio_select = ratio;
        _write(reg_02h);
    }
    void set_divide_ratio(hmc988_divide_ratio ratio, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc988, error_type, NoerrorValue, hmc988_divide_ratio, &hmc988::set_divide_ratio>(this, ratio, error);
    }
    void get_divide_ratio(hmc988_divide_ratio& ratio) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_02h reg_02h {};
        reg_00h.data.bits.read_control = reg_02h.addr;
        _write(reg_00h);
        _read(reg_02h);
        ratio = reg_02h.data.bits.divide_ratio_select;
    }
    hmc988_divide_ratio get_divide_ratio() const
    {
        hmc988_divide_ratio ratio {};
        get_divide_ratio(ratio);
        return ratio;
    }
    hmc988_divide_ratio get_divide_ratio(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, hmc988_divide_ratio, &hmc988::get_divide_ratio>(this, error);
    }
    void set_tx_buffer_swing(hmc988_tx_buffer_swing swing) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_03h reg_03h {};
        reg_00h.data.bits.read_control = reg_03h.addr;
        _write(reg_00h);
        _read(reg_03h);
        reg_03h.data.bits.tx_buffer_swing_select = swing;
        _write(reg_03h);
    }
    void set_tx_buffer_swing(hmc988_tx_buffer_swing swing, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc988, error_type, NoerrorValue, hmc988_tx_buffer_swing, &hmc988::set_tx_buffer_swing>(this, swing, error);
    }
    void get_tx_buffer_swing(hmc988_tx_buffer_swing& swing) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_03h reg_03h {};
        reg_00h.data.bits.read_control = reg_03h.addr;
        _write(reg_00h);
        _read(reg_03h);
        swing = reg_03h.data.bits.tx_buffer_swing_select;
    }
    hmc988_tx_buffer_swing get_tx_buffer_swing() const
    {
        hmc988_tx_buffer_swing swing {};
        get_tx_buffer_swing(swing);
        return swing;
    }
    hmc988_tx_buffer_swing get_tx_buffer_swing(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, hmc988_tx_buffer_swing, &hmc988::get_tx_buffer_swing>(this, error);
    }
    void force_gpo(const hmc988_gpo_force& force) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_05h reg_05h {};
        reg_00h.data.bits.read_control = reg_05h.addr;
        _write(reg_00h);
        _read(reg_05h);
        switch (force.mode) {
        case hmc988_gpo_force_mode::on_gpo_only:
            reg_05h.data.bits.force_gpo_on_gpo = force.enabled;
            break;
        case hmc988_gpo_force_mode::on_sdo_only:
            reg_05h.data.bits.force_gpo_on_sdo = force.enabled;
            break;
        case hmc988_gpo_force_mode::to_hiz:
            reg_05h.data.bits.force_gpo_hiz = force.enabled;
            break;
        default:
            throw std::invalid_argument("hmc988::force_gpo: invalid argument");
            break;
        }
        _write(reg_05h);
    }
    void force_gpo(const hmc988_gpo_force& force, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc988, error_type, NoerrorValue, hmc988_gpo_force, &hmc988::force_gpo>(this, force, error);
    }
    void is_gpo_forced(hmc988_gpo_force_mode force_mode, bool& enabled) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_05h reg_05h {};
        reg_00h.data.bits.read_control = reg_05h.addr;
        _write(reg_00h);
        _read(reg_05h);
        switch (force_mode) {
        case hmc988_gpo_force_mode::on_gpo_only:
            enabled = reg_05h.data.bits.force_gpo_on_gpo;
            break;
        case hmc988_gpo_force_mode::on_sdo_only:
            enabled = reg_05h.data.bits.force_gpo_on_sdo;
            break;
        case hmc988_gpo_force_mode::to_hiz:
            enabled = reg_05h.data.bits.force_gpo_hiz;
            break;
        default:
            throw std::invalid_argument("hmc988::is_gpo_forced: invalid argument");
            break;
        }
    }
    bool is_gpo_forced(hmc988_gpo_force_mode force_mode) const
    {
        bool enabled {};
        is_gpo_forced(force_mode, enabled);
        return enabled;
    }
    bool is_gpo_forced(hmc988_gpo_force_mode force_mode, error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, bool, &hmc988::is_gpo_forced>(this, force_mode, error);
    }
    void set_delay_line_setpoint(uint8_t setpoint) const
    {
        if (setpoint > 60) {
            throw std::invalid_argument("hmc988::set_delay_line_setpoint: invalid argument");
        }
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_07h reg_07h {};
        reg_07h.data.bits.delay_line_setpoint = setpoint;
        _write(reg_07h);
        _delay_line_setpoint = setpoint; // FIXME: микросхема возвращает значение с 111 в младших разрядах поэтому сохраняем значение в переменную.
    }
    void set_delay_line_setpoint(uint8_t setpoint, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc988, error_type, NoerrorValue, uint8_t, &hmc988::set_delay_line_setpoint>(this, setpoint, error);
    }
    void get_delay_line_setpoint(uint8_t& setpoint) const
    {
        using namespace hmc988_registers;
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        register_00h reg_00h {};
        register_07h reg_07h {};
        reg_00h.data.bits.read_control = reg_07h.addr;
        _write(reg_00h);
        _read(reg_07h);
        setpoint = reg_07h.data.bits.delay_line_setpoint;
        setpoint = _delay_line_setpoint; // FIXME: микросхема возвращает значение с 111 в младших разрядах поэтому возвращаем ранее сохраненное значение.
    }
    uint8_t get_delay_line_setpoint() const
    {
        uint8_t setpoint {};
        get_delay_line_setpoint(setpoint);
        return setpoint;
    }
    uint8_t get_delay_line_setpoint(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc988, error_type, NoerrorValue, uint8_t, &hmc988::get_delay_line_setpoint>(this, error);
    }

private:
    template <typename register_bits_type, hmc988_registers::register_addr_type register_addr>
    void _read(hmc988_registers::register_abstract<register_bits_type, register_addr>& reg) const
    {
        read(get_dev_addr(), reg.data.value);
    }
    template <typename register_bits_type, hmc988_registers::register_addr_type register_addr>
    void _write(const hmc988_registers::register_abstract<register_bits_type, register_addr>& reg) const
    {
        write((reg.addr << _addr_offset) | get_dev_addr(), reg.data.value);
    }
};

} // namespace chappi
