#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/kmod.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include "ddriver_L6_ioctl.h"

#define PATH_SZ 512

static dev_t muzik_dev;
static struct cdev muzik_cdev;
static char muzikpth[PATH_SZ] = "/tmp";

#define BUF_SZ 1024
static char muzbuf[BUF_SZ + 1];

enum player_state {
    IDLE = 0,
    PLAYING = 1,
    PAUSED = 2,
};

static enum player_state stts = IDLE;
static const char *playerstts[] = {"Parado\n", "Tocando\n", "Pausado\n"};

static int muzik_open(struct inode *inode, struct file *file);
static int muzik_close(struct inode *inode, struct file *file);
static ssize_t muzik_read(struct file *file, char __user *buf, size_t size, loff_t *ppos);
static ssize_t muzik_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos);
static long muzioctl(struct file *file, unsigned int cmd, unsigned long arg);

const struct file_operations muzik_op = {
    .read = muzik_read,
    .write = muzik_write,
    .open = muzik_open,
    .release = muzik_close,
    .unlocked_ioctl = muzioctl
};

static int __init minit(void){
    int all_ret;
    all_ret = alloc_chrdev_region(&muzik_dev, 0, 1, "muzik");
    if (all_ret){
        pr_err("Falha ao alocar o número do player\n");
    }
    cdev_init(&muzik_cdev, &muzik_op);
    all_ret = cdev_add(&muzik_cdev, muzik_dev, 1);
    if (all_ret){
        pr_err("Falha ao registrar o player\n");
    }
    pr_info("With the biggest request, layin' the wax and spinnin' the sound!\n");
    return 0;
}

static void __exit mexit(void){
    cdev_del(&muzik_cdev);
    unregister_chrdev_region(muzik_dev, 1);
    pr_info("We await your return, warrior\n");
}

static ssize_t muzik_read(struct file *file, char __user *buf, size_t size, loff_t *ppos){
    ssize_t tam = simple_read_from_buffer(buf, size, ppos, playerstts[stts], strlen(playerstts[stts]));
    pr_info("Path carregado: %s", muzikpth);
    return tam;
}

static ssize_t muzik_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos){
    ssize_t tam = (size < BUF_SZ - 1) ? size : BUF_SZ - 1;
    if (copy_from_user(muzbuf, buf, tam)){
        return -EFAULT;
    }
    muzbuf[tam] = '\0';

    while (tam > 0 && (muzbuf[tam - 1] == '\n' || muzbuf[tam - 1] == '\r')) {
        muzbuf[tam - 1] = '\0';
        tam--;
    }

    char *com = muzbuf;
    char *arg = NULL;
    ssize_t i;
    for (i = 0; i < tam; i++){
        if (muzbuf[i] == ' '){
            muzbuf[i] = '\0';
            arg = &muzbuf[i + 1];
        }
    }
    if (!(strcmp(com, "play"))){
        char cam[BUF_SZ + PATH_SZ + 2];
        snprintf(cam, sizeof(cam), "%s/%s", muzikpth, arg);
        char *argv[] = {"/usr/bin/aplay", "-D", cam, NULL};
        char *envp[] = {"PATH=/usr/bin:/bin:/usr/local/bin", NULL};
        call_usermodehelper(argv[0], argv, envp, 0);
        stts = PLAYING;
        pr_info("Tocando agora: %s\n", arg);
    }
    else if (!(strcmp(com, "pause"))){
        struct task_struct *taskaplay;
        rcu_read_lock();
        for_each_process(taskaplay){
            if (!(strcmp(taskaplay->comm, "aplay"))){
                send_sig(SIGSTOP, taskaplay, 0);
                stts = PAUSED;
            }
        }
        rcu_read_unlock();
    }
    else if (!(strcmp(com, "resume"))){
        struct task_struct *taskaplay;
        rcu_read_lock();
        for_each_process(taskaplay){
            if (!(strcmp(taskaplay->comm, "aplay"))){
                send_sig(SIGCONT, taskaplay, 0);
                stts = PLAYING;
            }
        }
        rcu_read_unlock();
    }
    else if (!(strcmp(com, "stop"))){
        char *argv[] = {"/usr/bin/pkill", "-f","aplay", NULL};
        char *envp[] = {"PATH=/usr/bin:/bin", NULL};
        call_usermodehelper(argv[0], argv, envp, 0);
        stts = IDLE;
    }
    else {
        pr_err("Comando inválido\n");
        pr_info("Comandos: play, pause, resume e stop - Formatos aceitos: wav\n");
        pr_info("Lembre-se de configurar o caminho das músicas, ou elas serão buscadas na pasta padrão!\n");
    }
    return size;
}

static int muzik_open(struct inode *inode, struct file *file){
    
    return 0;
}

static int muzik_close(struct inode *inode, struct file *file){
    
    return 0;
}

static long muzioctl(struct file *file, unsigned int cmd, unsigned long arg){
    if (_IOC_TYPE(cmd) != MNUM) return -ENOTTY;
    struct conf_st struct_aux;
    if (cmd == CONF_PATH){
        if (copy_from_user(&struct_aux, (void __user *)arg, sizeof(struct_aux))){
            return -EFAULT;
        }
        struct_aux.path[PATH_SZ - 1] = '\0';
        strncpy(muzikpth, struct_aux.path, PATH_SZ);
    }
    else {
        pr_warn("Config inválida\n");
        return -EINVAL;
    }
    return 0;
}

module_init(minit);
module_exit(mexit);

MODULE_AUTHOR("ra247855 - ra250397 - ra247184");
MODULE_DESCRIPTION("DD para abrir músicas");
MODULE_LICENSE("GPL");