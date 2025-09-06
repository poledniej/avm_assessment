#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>

struct word_list {
    struct list_head list;
    char* data;
};

LIST_HEAD(g_word_list);
DEFINE_MUTEX(list_mutex);

/*
 * Prototypes
 */

void append_to_word_list(char* word);
const char* peek_word_list(void);
char *pop_from_word_list(void);

/**
 * Appends a word to the list.
 * The string must not be a local variable.
 * @param word New word to append to the list
 */
void append_to_word_list(char* word) {
    struct word_list* new_node = kcalloc(1, sizeof(struct word_list), GFP_KERNEL);
    INIT_LIST_HEAD(&new_node->list);
    new_node->data = word;
    mutex_lock(&list_mutex);
    list_add_tail(&new_node->list, &g_word_list);
    mutex_unlock(&list_mutex);
}

/**
 * Peek at the first word in the list.
 *
 *
 * @return the first word in the list or NULL if list is empty
 */
const char* peek_word_list() {
    if (list_empty(&g_word_list))
        return NULL;
    return list_first_entry(&g_word_list, struct word_list, list)->data;
}

/**
 * Removes the first word and removes it from the list.
 * @return the removed word
 */
char *pop_from_word_list() {
    mutex_lock(&list_mutex);
    struct word_list* head = list_first_entry(&g_word_list, struct word_list, list);
    list_del(&head->list);
    mutex_unlock(&list_mutex);
    char* data = head->data;
    kfree(head);
    return data;
}


MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("AVM Assessment kernel logger driver");
static int avm_kernel_logger_init(void)
{
    printk(KERN_INFO "avm_kernel_logger module loaded\n");
    mutex_init(&list_mutex);
    for (int i = 0; i < 10; i++) {
        char* word = kmalloc(strlen("Hallo 0"), GFP_KERNEL);
        sprintf(word, "Hallo %d", i);
        append_to_word_list(word);
    }

    while (!list_empty(&g_word_list)) {
        char* word = pop_from_word_list();
        printk(KERN_INFO "%s\n", word);
        kfree(word);
    }

    return 0;
}
static void avm_kernel_logger_exit(void)

{
    printk(KERN_INFO "avm_kernel_logger module removed\n");

}
module_init(avm_kernel_logger_init);
module_exit(avm_kernel_logger_exit);