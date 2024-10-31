#include "net_config.h"
#include "os/os_compat.h"

#ifdef CONFIG_IPERF_ENABLE
static int iperf_test_pid;

extern int iperf_main(int argc, char **argv);

static void iperf_test_thread(void *priv)
{
    int argc = 0;
    char *argv[32] = {"iperf3", 0}; //p:f:i:D1VJvsc:ub:t:n:k:l:P:Rw:B:M:N46S:L:ZO:F:A:T:C:dI:hX:

    argv[++argc] = "-i";
    argv[++argc] = "60";
    argv[++argc] = "-s";
    argv[++argc] = "-p";
    argv[++argc] = "5001";
//    argv[++argc] = "-V";
//    argv[++argc] = "-d";


    iperf_main(++argc, argv);

}
#endif

void iperf_test(void)
{
#ifdef CONFIG_IPERF_ENABLE
    if (iperf_test_pid == 0) {
        thread_fork("iperf_test", 5, 768, 0, &iperf_test_pid, iperf_test_thread, 0);
    }
#endif
}

