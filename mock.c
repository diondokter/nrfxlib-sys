#include "nrf_socket.h"
#include "nrf_errno.h"
#include "nrf_modem_os.h"
#include "nrf_modem_at.h"
#include "nrf_modem_gnss.h"
#include <string.h>

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

int nrf_modem_at_cmd(void *buf, size_t len, const char *fmt, ...)
{
    strncpy(buf, "OK", len);
    return 0;
}

int nrf_modem_at_printf(const char *fmt, ...)
{
    return 0;
}

int nrf_modem_at_cmd_async(nrf_modem_at_resp_handler_t callback, const char *fmt, ...)
{
    callback("OK");
    return 0;
}

static nrf_modem_at_notif_handler_t at_notif_callback = NULL;

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