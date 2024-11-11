#include "system/includes.h"
#include "app_config.h"


#if 0

#define FILE_SIZE   20 * 1024 * 1024

static u8 write_buf[32 * 1024 + 500];

static void fs_write_test()
{

    for(int step = 0; step < 2048; step++)
    {

        int wlen = 1;

        FILE *f = fopen(CONFIG_STORAGE_PATH"/C/CC/wt_001.m", "w+");

        if(!f)
        {
            return;
        }

        memset(write_buf, (0x5A + step) & 0xff, sizeof(write_buf));


        for(int i = 0; i < FILE_SIZE;)
        {
            int len = fwrite(f, write_buf, wlen);
            if(len != wlen)
            {
                log_e("fwrite: %x, %x, %x\n", i, len, wlen);
                return;
            }
            i += wlen;
            if(i == FILE_SIZE)
            {
                break;
            }
            wlen +=  jiffies % sizeof(write_buf);
            if(wlen > sizeof(write_buf))
            {
                wlen = 2;
            }
            if(i + wlen > FILE_SIZE)
            {
                wlen = FILE_SIZE - i;
            }
        }
        printf("write_ok: %x, %x\n", wlen, fpos(f));
        fseek(f, 0, SEEK_SET);

        int rlen = 1024;

        for(int i = 0; i < FILE_SIZE; i += rlen)
        {
            memset(write_buf, 0, rlen);
            int len = fread(f, write_buf, rlen);
            if(len != rlen)
            {
                log_e("fread: %x, %x, %x\n", len, rlen, fpos(f));
                fclose(f);
                return;
            }
            for(int i = 0; i < rlen; i++)
            {
                if(write_buf[i] != ((0x5A + step) & 0xff))
                {
                    log_e("not eq: %x, %x, %x\n", write_buf[i], fpos(f), i);
                    fclose(f);
                    return;
                }
            }
            if(i + rlen >= FILE_SIZE)
            {
                rlen = FILE_SIZE - i;
            }
        }

        log_d("---------------------fs_write_suss: %d\n", step);

        fdelete(f);
    }

}

static u8 buf[115200];

static void fs_write_test2()
{
    memset(buf, 0xaa, sizeof(buf));

    for(int i = 0; i < 400; i++)
    {

        printf("\nfs_write_test: %x\n", i * sizeof(buf));

        FILE *f = fopen(CONFIG_STORAGE_PATH"/C/test.txt", "w+");

        fseek(f, i * sizeof(buf), SEEK_SET);

        puts("---------fwrite\n");
        fwrite(f, buf, sizeof(buf));
#if 0
        fclose(f);
        f = fopen(CONFIG_STORAGE_PATH"/C/test.txt", "r");
#endif
        fseek(f, i * sizeof(buf), SEEK_SET);
        memset(buf, 0x55, sizeof(buf));
        fread(f, buf, sizeof(buf));
        for(int j = 0; j < sizeof(buf); j++)
        {
            if(buf[j] != 0xaa)
            {
                log_e("fs_write_err: %x, %d, %x\n", i * sizeof(buf), j, buf[j]);
                fclose(f);
                return;
            }
        }
        fclose(f);
    }
}

static void exfat_read_test()
{
    u8 name[128];
    struct vfscan *fs;

    fs = fscan(CONFIG_STORAGE_PATH"/C", "-tTXT -sn");
    if(fs == NULL)
    {
        log_e("fscan_faild\n");
        return;
    }

    printf("fscan: file_number = %d\n", fs->file_number);;

    FILE *file = fselect(fs, FSEL_FIRST_FILE, 0);
    if(file)
    {
        fget_name(file, name, sizeof(name));
        printf("file_name: %s\n", name);
    }

}

int storage_device_ready();
static void fs_test_task(void *p)
{
    OS_SEM sem;

    os_sem_create(&sem, 0);

    while(1)
    {
        if(!storage_device_ready())
        {
            os_time_dly(10);
            continue;
        }
        /*fs_write_test();*/
        /*fs_write_test2();*/
        exfat_read_test();
        os_sem_pend(&sem, 0);
    }

}



static int fs_test_init()
{
    task_create(fs_test_task, NULL, "fs_test");
    return 0;
}
late_initcall(fs_test_init);

#endif
