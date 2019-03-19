#include "test.h"
#include "server.h"
extern int cmd_status_fd;

void cmd_and_status_channel_rw_loop_test()
{

    cmd_status_fd = open_cmd_status_device(CmdStatusDevicePath);
    printc((cmd_status_fd > 0) ? MSG_INFO : MSG_ERROR, "open device %s %s\n", CmdStatusDevicePath, (cmd_status_fd > 0) ? "success" : "failed");

    pthread_t thread_id = 0;
    if(pthread_create(&thread_id, NULL, (void *)cmd_and_status_channel_loop_read, NULL) != 0)
    {
        printc(MSG_ERROR, "create cmd_and_status_channel_loop_read failed!. exit\n");
        goto Exit;
    }
    if(pthread_create(&thread_id, NULL, (void *)cmd_and_status_channel_loop_write, NULL) != 0)
    {
        printc(MSG_ERROR, "create cmd_and_status_channel_loop_write failed!. exit\n");
        goto Exit;
    }
    Exit:
    return;
}

void cmd_and_status_channel_loop_write()
{
    char ip_write_cmd[1024];
    int count = 100;
    int sum = 0;
    while((cmd_status_fd > 0) && (count-- > 0))
    {
        if(writen_to_cmd_status_dev(cmd_status_fd, ip_write_cmd, sizeof(ip_write_cmd)) != sizeof(ip_write_cmd))
        {
            printc(MSG_ERROR, "cmd_and_status_channel_loop_write. exit\n", cmd_status_fd);
            sum += sizeof(ip_write_cmd);
        }
    }
     printc(MSG_INFO, "=====================cmd_and_status_channel_loop_write. sum is \n", sum);
}

void cmd_and_status_channel_loop_read()
{
    const int max_read_len_once = 0x1000;
    char *status_buf = (char *)malloc(sizeof(char) * max_read_len_once);
    if(status_buf == NULL)
    {
        printc(MSG_ERROR, "send_status_data_thread applicate mem from system failed, thread exit\n");
        return;
    }
    int read_ret = 0;
    int sum = 0;
    while(1)
    {
        printc(MSG_INFO, "start read....\n");
        read_ret = read(cmd_status_fd, status_buf, max_read_len_once);
        if(read_ret < 0)
        {
            printc(MSG_ERROR, "cmd_and_status_channel_loop_read, thread exit\n");
            return;
        }

        sum += read_ret;
        printc(MSG_INFO, "===================== read status, len is %d , sum is %d\n", read_ret,sum);
    }
}



void cmd_status_trans_rw_test()
{
    char write_cmd[] = {0x05, 0xff,0x00,0x00,0x00,0xff};

    if(cmd_status_fd = open_cmd_status_device(CmdStatusDevicePath)< 0)
    {
       perror("cannot open device file");
       exit(0);
    }

    writen_to_cmd_status_dev(cmd_status_fd, write_cmd, sizeof(write_cmd));

    const int max_read_len_once = 0x1000;
    char *status_buf = (char *)malloc(sizeof(char) * max_read_len_once);

    if(status_buf == NULL)
    {
        printc(MSG_ERROR, "send_status_data_thread applicate mem from system failed, thread exit\n");
        return;
    }
    int read_ret = 0;
    int err_cnt_sum = 0;

    int is_first_read = 1;
    char start_val = 0;
    int sum = 0;

    int print_control_cnt = 10 * 1024 * 1024; //10M

    while(1)
    {
        printc(MSG_INFO, "start read....\n");
        read_ret = read(cmd_status_fd, status_buf, max_read_len_once);
        if(read_ret < 0)
        {
            printc(MSG_ERROR, "cmd_and_status_channel_loop_read, thread exit\n");
            return;
        }

        sum += read_ret;

        if(read_ret < 0)
        {
            printc(MSG_ERROR, "cmd_and_status_channel_loop_read, thread exit\n");
            return;
        }

        if(is_first_read == 1)
        {
            err_cnt_sum += check_data_inc(start_val, 0, status_buf, read_ret);

            is_first_read = !is_first_read;

        }else{
            err_cnt_sum += check_data_inc(start_val, 1, status_buf, read_ret);
        }
        start_val = status_buf[read_ret - 1];

        printc(MSG_INFO, "the CAS err_cnt_sum is %d\n",err_cnt_sum);
        if(sum > print_control_cnt)
        {
            printc(MSG_INFO, "===================== read status, sum is %d\n", sum);
            print_control_cnt *= 2;

        }

    }
    free(status_buf);
}






int check_data_inc(char start_val, int start_val_vaild, char *data, int length)
{
    int err_cnt = 0;
    if(start_val_vaild == 1)
    {
        start_val = data[0] - 1;
    }
    for(int i = 0; i < length; i++)
    {
        start_val++;
        if(start_val != data[i])
        {
            start_val = data[i];
            err_cnt++;
        }
    }
    return err_cnt;
}
