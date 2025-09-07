#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/proc_fs.h>


/*
 * Defines
 */


#define PROC_FILENAME "avm_kernel_logger"
#define PROC_MAX_SIZE 1024

/*
 * Prototypes
 */

static void append_to_word_list(char *word);

static char *pop_from_word_list(void);

static void print_word(struct work_struct *work);
static void scheduleNextPrint(struct timer_list *timer);


/*
 * struct definitions
 */
struct word_list {
    struct list_head list;
    char *data;
};

/*
 * Globals
 */
LIST_HEAD(g_word_list);
DEFINE_MUTEX(list_mutex);

DEFINE_TIMER(aWordASecond, scheduleNextPrint);
DECLARE_WORK(print_word_work, print_word);
struct workqueue_struct *work_queue;

static struct proc_dir_entry *proc_file;
static char proc_buffer[1024];
static size_t proc_buffer_size = 0;

/*
 * Local functions
 */

/**
 * Appends a word to the list.
 * The string must not be a local variable.
 * @param word New word to append to the list
 */
static void append_to_word_list(char *word) {
    struct word_list *new_node = kcalloc(1, sizeof(struct word_list), GFP_KERNEL);
    INIT_LIST_HEAD(&new_node->list);
    new_node->data = word;
    mutex_lock(&list_mutex);
    list_add_tail(&new_node->list, &g_word_list);
    mutex_unlock(&list_mutex);
}


/**
 * Removes the first word and removes it from the list.
 * @return the removed word
 */
static char *pop_from_word_list(void) {
    mutex_lock(&list_mutex);
    struct word_list *head = list_first_entry(&g_word_list, struct word_list, list);
    list_del(&head->list);
    mutex_unlock(&list_mutex);
    char *data = head->data;
    kfree(head);
    return data;
}

/**
 * Callback function for print_word_work
 * Removes the oldest entry and prints its contents.
 * @param work work_struct associated with this callback
 */
static void print_word(struct work_struct *work) {
    if (list_empty(&g_word_list))
        return;

    char *data = pop_from_word_list();

    pr_info("[avm_kernel_logger] %s\n", data);
    kfree(data);
}

/**
 * callback function for aWordASecond Timer
 * Schedule a new work every second if the list is not empty
 * @param timer timer_list associated with this callback
 */
static void scheduleNextPrint(struct timer_list *timer) {
    if (list_empty(&g_word_list))
        return;
    mod_timer(timer, timer->expires + msecs_to_jiffies(1000));
    queue_work(work_queue, &print_word_work);
}

/**
 * Deletes the word list
 * used in module cleanup
 */
static void delete_word_list(void) {
    if (list_empty(&g_word_list))
        return;
    struct word_list *cursor;
    struct word_list *tmpForDeletion;
    list_for_each_entry_safe(tmpForDeletion, cursor, &g_word_list, list) {
        list_del(&tmpForDeletion->list);
        kfree(tmpForDeletion->data);
        kfree(tmpForDeletion);
    }
}

/**
 * read callback for the proc file
 * Prints every word in the list
 */
static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    if (*ppos > 0 || count < proc_buffer_size)
        return 0; // Ende der Datei

    struct word_list *ptr;
    int written = 0;
    list_for_each_entry(ptr, &g_word_list, list) {
        strcpy(proc_buffer + written, ptr->data);
        written += strlen(ptr->data);
        proc_buffer[written] = ' ';
        proc_buffer[++written] = '\0';
    }


    if (copy_to_user(buf, proc_buffer, written))
        return -EFAULT;

    *ppos = written;
    return written;
}

/**
 * write callback for the proc file
 * tokenizes the passed string and adds every word to the end of the list.
 */
static ssize_t proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    if (count > PROC_MAX_SIZE)
        proc_buffer_size = PROC_MAX_SIZE;
    else
        proc_buffer_size = count;


    if (copy_from_user(proc_buffer, buf, proc_buffer_size))
        return -EFAULT;

    char *pProcBuffer = proc_buffer;
    char *tmp_char = strsep(&pProcBuffer, "\n ");
    while (tmp_char && *tmp_char != '\0') {

        char* listWord = kcalloc(strlen(tmp_char)+1, 1, GFP_KERNEL);
        strcpy(listWord, tmp_char);

        append_to_word_list(listWord);

        tmp_char = strsep(&pProcBuffer, "\r\n ");
    }

    if (!list_empty(&g_word_list) && !timer_pending(&aWordASecond))
        mod_timer(&aWordASecond, jiffies + msecs_to_jiffies(1000));

    return proc_buffer_size;
}

/*
 * Module Initialization
 */

static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};


MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("AVM Assessment kernel logger driver");

static int avm_kernel_logger_init(void) {
    proc_file = proc_create(PROC_FILENAME, 0666, NULL, &proc_file_ops);
    if (!proc_file) {
        pr_err("[proc_module] Fehler beim Erstellen von /proc/%s\n", PROC_FILENAME);
        return -ENOMEM;
    }

    mutex_init(&list_mutex);
    work_queue = create_singlethread_workqueue("AVM logger word printer");

    printk(KERN_INFO "[avm_kernel_logger] module loaded\n");
    return 0;
}

static void avm_kernel_logger_exit(void) {

    cancel_work(&print_word_work);
    flush_workqueue(work_queue);
    destroy_workqueue(work_queue);
    timer_delete_sync(&aWordASecond);

    proc_remove(proc_file);
    delete_word_list();

    mutex_destroy(&list_mutex);
    printk(KERN_INFO "[avm_kernel_logger] module removed\n");
}

module_init(avm_kernel_logger_init);
module_exit(avm_kernel_logger_exit);
