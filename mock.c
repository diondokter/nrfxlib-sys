#include "nrf_socket.h"
#include "nrf_errno.h"
#include "nrf_modem_os.h"
#include "nrf_modem_at.h"
#include "nrf_modem_gnss.h"
#include "nrf_modem.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdatomic.h>

static int current_creg = 1;
static nrf_modem_at_notif_handler_t at_notif_callback = NULL;

void call_at_notif_callback(const void *notif)
{
    at_notif_callback((const char *)notif);
}

extern void nrf_modem_os_log_rust(int level, const char *formatted_string);

typedef void (*WorkItemCallback)(const void *context);

typedef struct
{
    atomic_uint taken; // 0: free, 1: being initialized, 2: active
    unsigned int timeout_us;
    WorkItemCallback callback;
    const void *context;
} WorkItem;

#define WORK_QUEUE_SIZE 16
static WorkItem WORK_QUEUE[WORK_QUEUE_SIZE] = {0};

void run_work_queue(unsigned int delta_time_us)
{
    for (int i = 0; i < WORK_QUEUE_SIZE; i++)
    {
        if (atomic_load(&WORK_QUEUE[i].taken) == 2)
        {
            if (delta_time_us <= WORK_QUEUE[i].timeout_us)
            {
                // Timeout is done
                WORK_QUEUE[i].timeout_us = 0;
                (WORK_QUEUE[i].callback)(WORK_QUEUE[i].context);
                WORK_QUEUE[i].callback = NULL;
                WORK_QUEUE[i].context = NULL;
                // Release the slot
                atomic_store(&WORK_QUEUE[i].taken, 0);
            }
            else
            {
                WORK_QUEUE[i].timeout_us -= delta_time_us;
            }
        }
    }
}

void add_work(unsigned int timeout_us, WorkItemCallback callback, const void *context)
{
    for (int i = 0; i < WORK_QUEUE_SIZE; i++)
    {
        const unsigned int expected = 0;
        // Try take the slot if available
        if (atomic_compare_exchange_strong(&WORK_QUEUE[i].taken, &expected, 1))
        {
            WORK_QUEUE[i].callback = callback;
            WORK_QUEUE[i].context = context;
            WORK_QUEUE[i].timeout_us = timeout_us;
            atomic_store(&WORK_QUEUE[i].taken, 2);
            return;
        }
    }

    nrf_modem_os_log_rust(NRF_MODEM_LOG_LEVEL_ERR, "Out of work spots!");
}

int nrf_fcntl(int fd, int cmd, int flags)
{
    return 0;
}

static nrf_modem_pollcb_t socket_callback = NULL;
void call_socket_callback(const void* context) {
    socket_callback((struct nrf_pollfd *)context);
}

int nrf_setsockopt(int socket, int level, int option_name,
                   const void *option_value, nrf_socklen_t option_len)
{
    if (option_name == NRF_SO_POLLCB) {
        socket_callback = (*(struct nrf_modem_pollcb *)option_value).callback;
    }

    return 0;
}

int nrf_socket(int family, int type, int protocol)
{
    return 0;
}

int socket_state = 0; // 0: unconnected, 1: connected

int nrf_connect(int socket, const struct nrf_sockaddr *address, nrf_socklen_t address_len)
{
    if (socket_state == 0) {
        static struct nrf_pollfd POLLFD = {.fd = 0, .revents = 4, .events = 0 };
        add_work(121185, call_socket_callback, &POLLFD);
        return NRF_EINPROGRESS;
    }
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

void set_cereg_state(const void* value) {
    current_creg = *(const int*)value;

    if (current_creg == 1) {
        at_notif_callback("+CEREG: 1");
    }
    if (current_creg == 2) {
        at_notif_callback("+CEREG: 2");
    }
    if (current_creg == 5) {
        at_notif_callback("+CEREG: 5");
    }
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
    if (!strcmp(buffer, "AT+CFUN=21"))
    {
        static int STATE_2 = 2;
        static int STATE_5 = 5;
        add_work(4314332, set_cereg_state, &STATE_2);
        add_work(6973969, set_cereg_state, &STATE_5);
    }
    if (!strcmp(buffer, "AT+CGSN"))
    {
        return "353785728423590\r\nOK\r\n";
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
        if (current_creg == 1) {
            return "+CEREG: 1,4\r\nOK\r\n";
        }
        if (current_creg == 2) {
            return "+CEREG: 1,2\r\nOK\r\n";
        }
        if (current_creg == 5) {
            return "+CEREG: 1,5\r\nOK\r\n";
        }
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
        add_work(
            3487396, // ~3.5 seconds
            call_at_notif_callback,
            (const void *)"%NCELLMEAS: 0,\"01186F0B\",\"20408\",\"878F\",65535,0,6400,14,42,12,6898,0,0,\"0366FC7A\",\"20416\",\"0203\",65535,0,6200,80,37,13,6907,0,0");
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
