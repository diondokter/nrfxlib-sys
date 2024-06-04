#include "nrf_socket.h"
#include "nrf_errno.h"
#include "nrf_modem_os.h"
#include "nrf_modem_at.h"
#include "nrf_modem_gnss.h"
#include "nrf_modem.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static nrf_modem_at_notif_handler_t at_notif_callback = NULL;
extern void nrf_modem_os_log_rust(int level, const char *formatted_string);

int nrf_fcntl(int fd, int cmd, int flags)
{
    return 0;
}

int nrf_setsockopt(int socket, int level, int option_name,
                   const void *option_value, nrf_socklen_t option_len)
{
    return 0;
}

int nrf_socket(int family, int type, int protocol)
{
    return 0;
}

int nrf_connect(int socket, const struct nrf_sockaddr *address, nrf_socklen_t address_len)
{
    return 0;
}

ssize_t nrf_send(int socket, const void *buffer, size_t length, int flags)
{
    nrf_modem_os_log(1, "Send! %i, 0x%X, %i, %i", socket, buffer, length, flags);

    if (buffer == NULL || length == 0)
    {
        nrf_modem_os_errno_set(NRF_EINVAL);
        return -1;
    }

    nrf_modem_os_errno_set(NRF_EWOULDBLOCK);
    return -1;
}

ssize_t nrf_recv(int socket, void *buffer, size_t length, int flags)
{
    return 0;
}

int nrf_close(int fildes)
{
    return 0;
}


const char *process_at(const char *fmt, va_list args)
{
    char buffer[256] = {0};

    vsnprintf(buffer, 256, fmt, args);

    nrf_modem_os_log_rust(2, buffer);

    if (!strcmp(buffer, "AT+CFUN?"))
    {
        return "+CFUN: 0\r\nOK\r\n";
    }
    if (!strcmp(buffer, "AT+CGSN"))
    {
        return "352656100367872\r\nOK\r\n";
    }
    if (!strcmp(buffer, "AT+CGMR"))
    {
        return "mfw_nrf9160_1.1.1\r\nOK\r\n";
    }
    if (!strcmp(buffer, "AT%XICCID"))
    {
        return "%XICCID: 8901234567012345678F\r\nOK\r\n";
    }
    if (!strcmp(buffer, "AT+CEREG?"))
    {
        return "+CEREG: 2,5\r\nOK\r\n";
    }
    if (!strcmp(buffer, "AT%XTEMP?"))
    {
        return "%XTEMP: 50\r\nOK\r\n";
    }
    if (!strcmp(buffer, "AT%XMONITOR"))
    {
        return "%XMONITOR: 1,\"EDAV\",\"EDAV\",\"26295\",\"00B7\",7,4,\"00011B07\",7,2300,63,39,\"\", \"11100000\",\"11100000\",\"11100000\"\r\nOK\r\n";
    }

    if (!strcmp(buffer, "AT%NCELLMEAS=4,6\r\n"))
    {
        at_notif_callback("%NCELLMEAS: 0,\"002B670D\",\"20408\",\"8729\",62,11447,6400,424,52,23,10002,1,1,6400,408,47,13,0,\"002B670D\",\"20408\",\"8729\",62,11447,6400,424,52,23,10002,1,1,6400,408,47,13,0");
    }

    return "OK\r\n";
}

int nrf_modem_at_cmd(void *buf, size_t len, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    strncpy(buf, process_at(fmt, args), len);
    va_end(args);
    return 0;
}

int nrf_modem_at_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    process_at(fmt, args);
    va_end(args);
    return 0;
}

int nrf_modem_at_cmd_async(nrf_modem_at_resp_handler_t callback, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    callback(process_at(fmt, args));
    va_end(args);
    return 0;
}

int nrf_modem_at_notif_handler_set(nrf_modem_at_notif_handler_t callback)
{
    at_notif_callback = callback;
    return 0;
}

static nrf_modem_gnss_event_handler_type_t gnss_event_handler = NULL;

int32_t nrf_modem_gnss_event_handler_set(nrf_modem_gnss_event_handler_type_t handler)
{
    gnss_event_handler = handler;
    return 0;
}

int32_t nrf_modem_gnss_fix_interval_set(uint16_t fix_interval)
{
    return 0;
}

int32_t nrf_modem_gnss_fix_retry_set(uint16_t fix_retry)
{
    return 0;
}

int32_t nrf_modem_gnss_start(void)
{
    return 0;
}

int32_t nrf_modem_gnss_stop(void)
{
    return 0;
}

int32_t nrf_modem_gnss_elevation_threshold_set(uint8_t angle)
{
    return 0;
}

int32_t nrf_modem_gnss_use_case_set(uint8_t use_case)
{
    return 0;
}

int32_t nrf_modem_gnss_nmea_mask_set(uint16_t nmea_mask)
{
    return 0;
}

int32_t nrf_modem_gnss_power_mode_set(uint8_t power_mode)
{
    return 0;
}

int32_t nrf_modem_gnss_timing_source_set(uint8_t timing_source)
{
    return 0;
}

int32_t nrf_modem_gnss_read(void *buf, int32_t buf_len, int type)
{
    return -NRF_ENOMSG;
}

int nrf_getaddrinfo(const char *restrict nodename,
                    const char *restrict servname,
                    const struct nrf_addrinfo *restrict hints,
                    struct nrf_addrinfo **restrict res)
{
    return NRF_ENOTSUP;
}

void nrf_freeaddrinfo(struct nrf_addrinfo *ai)
{
}

int nrf_modem_init(const struct nrf_modem_init_params *init_params)
{
    return 0;
}

bool nrf_modem_is_initialized(void)
{
    return true;
}
