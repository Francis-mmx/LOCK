#include "system/includes.h"
#include "eg_http_server.h"

#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/timing.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl_cookie.h"
#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif
#if defined(MBEDTLS_SSL_TICKET_C)
#include "mbedtls/ssl_ticket.h"
#endif

#include "net_config.h"

#ifdef CONFIG_EG_HTTP_SERVER

#define DEBUG_LEVEL 0


#define EG_HTTP_RESPONSE                                  \
        "HTTP/1.0 200 OK\r\n"                             \
        "Accept-Ranges: bytes\r\n"                        \
        "Content-length: %d\r\n"                          \
        "Content-Type: text/html\r\n"                     \
        "Connection: close\r\n\r\n"


/*eg_http_server info*/
struct eg_http_srv_t {
    void *sock_hdl;
    void *priv;
    int (*cb_func)(void *cli, int type, int cmd, char *content, void *priv);
    void *port;
    struct sockaddr_in dest_addr;
    mbedtls_net_context listen_fd;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt srvcert;
    mbedtls_pk_context pkey;
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_context cache;
#endif
#if defined(MBEDTLS_SSL_SESSION_TICKETS)
    mbedtls_ssl_ticket_context ticket_ctx;
#endif
};
/*eg cli info*/
struct eg_http_cli_t {
    mbedtls_ssl_context ssl;
    mbedtls_net_context client_fd;
};



static int eg_http_socket_send(mbedtls_ssl_context *ssl, const unsigned char *buf, size_t len);
static int eg_http_socket_cli_close(mbedtls_ssl_context *ssl);

static struct eg_http_srv_t eg_http_srv;

static int eg_cmd_respond_text_xml(void *cli, void *content, u32 len)
{
    u8 buf[256] = {0};
    int ret = 0;
    struct eg_http_cli_t *__cli = (struct eg_http_cli_t *)cli;
    if (!__cli) {
        ret = -1;
        goto exit;
    }
    sprintf(buf, EG_HTTP_RESPONSE, len);
    ret = eg_http_socket_send(&__cli->ssl, (u8 *)buf, strlen(buf));
    if (ret != strlen(buf)) {
        printf("\n send err \n");
        ret = -1;
        goto exit;
    }
    ret = eg_http_socket_send(&__cli->ssl, (u8 *)content, len);
    if (ret != len) {
        printf("\n send err \n");
        ret = -1;
        goto exit;
    }
exit:
    return ret;
}

static int eg_default_cmd(void *cli, void *content)
{
    //处理完命令，调用回复，仅供参考
    const u8 *buf = "<h2>eg_http_server</h2>\r\n<p> only test </p>\r\n";
    return eg_cmd_respond_text_xml(cli, buf, strlen(buf));

}

int eg_cmd_analysis(int cmd, const char *content, void *cli)
{
    int ret;
    switch (cmd) {
    case EG_DEFAULT_CMD:
        ret = eg_default_cmd(cli, content);
        break;
    default:
        printf("\n>>>Invalid cmd  is %d \n", cmd);
        ret = -1;
        break;
    }
    if (cli) {
        struct eg_http_cli_t *__cli = (struct eg_http_cli_t *)cli;
        eg_http_socket_cli_close(&__cli->ssl);
        mbedtls_ssl_free(&__cli->ssl);
        mbedtls_net_free(&__cli->client_fd);
        free(__cli);
    }
    return ret;
}

static int eg_http_socket_recv(mbedtls_ssl_context *ssl, unsigned char *buf, size_t len)
{
    /*
     * 6. Read the HTTP Request
    */
    int ret;
    do {
        ret = mbedtls_ssl_read(ssl, buf, len);
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            continue;
        }
        if (ret <= 0) {
            switch (ret) {
            case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                mbedtls_printf(" connection was closed gracefully\n");
                break;

            case MBEDTLS_ERR_NET_CONN_RESET:
                mbedtls_printf(" connection was reset by peer\n");
                break;

            default:
                mbedtls_printf(" mbedtls_ssl_read returned -0x%x\n", -ret);
                break;
            }

            break;
        }
        mbedtls_printf(" %d bytes read\n\n%s", ret, (char *) buf);
        if (ret > 0) {
            break;
        }
    } while (1);

    return ret;
}
static int eg_http_socket_send(mbedtls_ssl_context *ssl, const unsigned char *buf, size_t len)
{
    /*
    * 7. Write the 200 Response
    */
    int ret;

    while ((ret = mbedtls_ssl_write(ssl, buf, len)) <= 0) {
        if (ret == MBEDTLS_ERR_NET_CONN_RESET) {
            mbedtls_printf(" failed\n  ! peer closed the connection\n\n");
            break;
        }

        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            mbedtls_printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
            break;
        }
    }
    mbedtls_printf(" %d bytes written\n\n%s\n", len, (char *) buf);
    return ret;
}

static int eg_http_socket_cli_close(mbedtls_ssl_context *ssl)
{
    int ret;
    mbedtls_printf("  . Closing the connection...");

    while ((ret = mbedtls_ssl_close_notify(ssl)) < 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            mbedtls_printf(" failed\n  ! mbedtls_ssl_close_notify returned %d\n\n", ret);
            break;
        }
    }
    mbedtls_printf(" ok\n");
    return ret;
}
static int eg_http_socket_init(void)
{
    int ret;
    const char *pers = "eg_https_server";

    mbedtls_net_init(&eg_http_srv.listen_fd);


    mbedtls_ssl_config_init(&eg_http_srv.conf);

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_init(&eg_http_srv.cache);
#endif

    mbedtls_x509_crt_init(&eg_http_srv.srvcert);
    mbedtls_pk_init(&eg_http_srv.pkey);
    mbedtls_entropy_init(&eg_http_srv.entropy);
    mbedtls_ctr_drbg_init(&eg_http_srv.ctr_drbg);

#if defined(MBEDTLS_SSL_SESSION_TICKETS)
    mbedtls_ssl_ticket_init(&eg_http_srv.ticket_ctx);
#endif

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

    /*
     * 1. Load the certificates and private RSA key
    */
    mbedtls_printf("\n  . Loading the server cert. and key...");
    /*
     * This demonstration program uses embedded test certificates.
     * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
     * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
     */
    ret = mbedtls_x509_crt_parse(&eg_http_srv.srvcert, (const unsigned char *) mbedtls_test_srv_crt, mbedtls_test_srv_crt_len);
    if (ret != 0) {
        mbedtls_printf(" failed\n  !  mbedtls_x509_crt_parse returned %d line = %d\n\n", ret, __LINE__);
        goto exit;
    }

    ret = mbedtls_x509_crt_parse(&eg_http_srv.srvcert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (ret != 0) {
        mbedtls_printf(" failed\n  !  mbedtls_x509_crt_parse returned %d line = %d\n\n", ret, __LINE__);
        goto exit;
    }

    ret =  mbedtls_pk_parse_key(&eg_http_srv.pkey, (const unsigned char *) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0);
    if (ret != 0) {
        mbedtls_printf(" failed\n  !  mbedtls_pk_parse_key returned %d line = %d\n\n", ret, __LINE__);
        goto exit;
    }
    mbedtls_printf(" ok\n");
    /*
     * 2. Setup the listening TCP socket
     */
//    mbedtls_printf( "  . Bind on https://localhost:4433/ ..." );
    if ((ret = mbedtls_net_bind(&eg_http_srv.listen_fd, NULL, (const char *)eg_http_srv.port, MBEDTLS_NET_PROTO_TCP)) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_net_bind returned %d\n\n", ret);
        goto exit;
    }

    mbedtls_printf(" ok\n");
    /*
     *  3. Seed the RNG
     */
    mbedtls_printf("  . Seeding the random number generator...");

    if ((ret = mbedtls_ctr_drbg_seed(&eg_http_srv.ctr_drbg, mbedtls_entropy_func, &eg_http_srv.entropy, (const unsigned char *) pers, strlen(pers))) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        goto exit;
    }

    mbedtls_printf(" ok\n");
    /*
     * 4. Setup stuff
     */
    mbedtls_printf("  . Setting up the SSL data....");
    if ((ret = mbedtls_ssl_config_defaults(&eg_http_srv.conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
        goto exit;
    }

    mbedtls_ssl_conf_rng(&eg_http_srv.conf, mbedtls_ctr_drbg_random, &eg_http_srv.ctr_drbg);

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_conf_session_cache(&eg_http_srv.conf, &eg_http_srv.cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
#endif

    mbedtls_ssl_conf_ca_chain(&eg_http_srv.conf, eg_http_srv.srvcert.next, NULL);
    if ((ret = mbedtls_ssl_conf_own_cert(&eg_http_srv.conf, &eg_http_srv.srvcert, &eg_http_srv.pkey)) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret);
        goto exit;
    }
    return ret;
exit:

    mbedtls_net_free(&eg_http_srv.listen_fd);
    mbedtls_x509_crt_free(&eg_http_srv.srvcert);
    mbedtls_pk_free(&eg_http_srv.pkey);

    mbedtls_ssl_config_free(&eg_http_srv.conf);
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_free(&eg_http_srv.cache);
#endif
    mbedtls_ctr_drbg_free(&eg_http_srv.ctr_drbg);
    mbedtls_entropy_free(&eg_http_srv.entropy);

    return ret;
}

static void eg_http_recv_thread(void *priv)
{
    unsigned char buf[1024];
    int ret;
    struct eg_http_cli_t *cli = (struct eg_http_cli_t *)priv;
    mbedtls_printf("  < Read from client:");
    memset(buf, 0, sizeof(buf));
    //循环接收直到接收到完整的http请求
    ret = eg_http_socket_recv(&cli->ssl, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        printf("\n[ERROR] recv err\n");
    }
    //解析htpp请求
    //调用回调函数通知上层应用,示例如下
    //对应的参数意思：客户端信息、消息类型、命令号、内容、上层传递的参数
    //注：回调之后不是立即被处理的
    eg_http_srv.cb_func(cli, EG_DEFAULT_SRV_MSG, EG_DEFAULT_CMD, NULL, NULL);
}

static void eg_http_accept_thread(void *priv)
{
    int ret, err;
    struct eg_http_cli_t *cli = NULL;
    /*
     * 3. Wait until a client connects
     */
    while (1) {
        cli = (struct eg_http_cli_t *)malloc(sizeof(struct eg_http_cli_t));
        if (!cli) {
            os_time_dly(1);
            continue;
        }
        mbedtls_ssl_init(&cli->ssl);
        mbedtls_net_init(&cli->client_fd);
        if ((ret = mbedtls_ssl_setup(&cli->ssl, &eg_http_srv.conf)) != 0) {
            mbedtls_printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
            goto exit;
        }
        mbedtls_printf(" ok\n");

        mbedtls_printf("  . Waiting for a remote connection ...");

        if ((ret = mbedtls_net_accept(&eg_http_srv.listen_fd, &cli->client_fd, NULL, 0, NULL)) != 0) {
            mbedtls_printf(" failed\n  ! mbedtls_net_accept returned %d\n\n", ret);
            goto exit;
        }
        mbedtls_ssl_set_bio(&cli->ssl, &cli->client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
        mbedtls_printf(" ok\n");
        /*
         * 5. Handshake
         */
        mbedtls_printf("  . Performing the SSL/TLS handshake...");
        while ((ret = mbedtls_ssl_handshake(&cli->ssl)) != 0) {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
                mbedtls_printf(" failed\n  ! mbedtls_ssl_handshake returned %d\n\n", ret);
                free(cli);
                cli = NULL;
                continue;
            }
        }
        mbedtls_printf(" ok\n");

//        err = thread_fork("EG_HTTP_RECV_THREAD", 14, 0x500, 0, 0, eg_http_recv_thread, (void *)client_fd);
//        if(err != OS_NO_ERR){
//            printf("\n [ERROR] eg_http_accept_thread fail\n");
//            goto exit;
//        }
        eg_http_recv_thread(cli);
        cli = NULL;
    }

exit:

    mbedtls_net_free(&eg_http_srv.listen_fd);

    mbedtls_x509_crt_free(&eg_http_srv.srvcert);
    mbedtls_pk_free(&eg_http_srv.pkey);

    mbedtls_ssl_config_free(&eg_http_srv.conf);

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_free(&eg_http_srv.cache);
#endif

    mbedtls_ctr_drbg_free(&eg_http_srv.ctr_drbg);
#if defined(MBEDTLS_SSL_SESSION_TICKETS)
    mbedtls_ssl_ticket_free(&eg_http_srv.ticket_ctx);
#endif
    mbedtls_entropy_free(&eg_http_srv.entropy);
    if (cli) {
        mbedtls_net_free(&cli->client_fd);
        mbedtls_ssl_free(&cli->ssl);
        free(cli);
    }

}

static int eg_http_srv_init(void *port, (*cb_func)(void *cli, int type, int cmd, char *buf, void *priv), void *priv)
{
    int err;
    eg_http_srv.cb_func = cb_func;
    eg_http_srv.priv = priv;
    eg_http_srv.port = port;
    if (eg_http_socket_init() != 0) {
        printf("\n [ERROR] eg_http_socket_init fail\n");
        goto exit;
    }
    err = thread_fork("EG_HTTP_ACCEPT_THREAD", 14, 0x2000, 0, 0, eg_http_accept_thread, (void *)NULL);
    if (err != OS_NO_ERR) {
        printf("\n [ERROR] eg_http_accept_thread fail\n");
        goto exit;
    }
    return 0;

exit:
    return -1;
}
static int eg_http_ctrl_cb_func(void *cli, int type, int cmd, char *content, void *priv)
{
    struct sys_event e;
    struct eg_event_arg *event_arg = NULL;

    switch (type) {
    case EG_DEFAULT_SRV_MSG:
        event_arg = (struct eg_http_arg *)calloc(1, sizeof(struct eg_event_arg));
        if (!event_arg) {
            goto exit;
        }
        e.type = SYS_NET_EVENT; //以后可改为网络事件
        e.u.net.event = NET_EVENT_CMD;
        event_arg->cli = cli;
        event_arg->cmd = cmd;
        if (content) {
            event_arg->content = calloc(1, strlen(content) + 1);
            if (!event_arg->content) {
                goto exit;
            }
            strcpy(event_arg->content, content);
        }

        printf("\n EG_CMD<<%d>>\n", event_arg->cmd);
        e.arg = (void *)event_arg;
        sys_event_notify(&e);
    default :
        break;
    }
    return 0;

exit:
    if (event_arg) {
        if (!event_arg->content) {
            free(event_arg->content);
        }
        free(event_arg);
    }
    return -1;
}

int eg_protocol_init(void)
{
    eg_http_srv_init("443", eg_http_ctrl_cb_func, NULL);
    return 0;
}
#endif // CONFIG_EG_HTTP_SERVER

