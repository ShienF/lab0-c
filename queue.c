#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*test*/

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head;
    head = malloc(sizeof(struct list_head));
    if (!head) {
        free(head);
        return NULL;
    } else {
        INIT_LIST_HEAD(head);
        return head;
    }
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *node = NULL, *safe = NULL;
    list_for_each_entry_safe (node, safe, l, list) {
        q_release_element(node);
    }

    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;

    int len = strlen(s);
    element->value = malloc(sizeof(char) * len + 1);
    if (!element->value) {
        free(element);
        return false;
    }

    strncpy(element->value, s, len + 1);

    list_add(&element->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;

    int len = strlen(s);
    element->value = malloc(sizeof(char) * len + 1);
    if (!element->value) {
        free(element);
        return false;
    }

    strncpy(element->value, s, len + 1);

    list_add_tail(&element->list, head);

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *target = list_first_entry(head, element_t, list);

    list_del(head->next);

    if (sp) {
        strncpy(sp, target->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    return target;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *target = list_last_entry(head, element_t, list);

    list_del(head->prev);

    if (sp) {
        strncpy(sp, target->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    return target;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 * // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
 */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head **indir = &head->next;

    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next)
        indir = &(*indir)->next;

    struct list_head *del = *indir;
    list_del(del);

    element_t *del_element = list_entry(del, element_t, list);
    q_release_element(del_element);

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 * // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
 */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;

    element_t *element, *next_element;
    bool check = 0;

    list_for_each_entry_safe (element, next_element, head, list) {
        if (&next_element->list != head &&
            strcmp(next_element->value, element->value) == 0) {
            list_del(&element->list);
            q_release_element(element);
            check = 1;
        } else if (check) {
            list_del(&element->list);
            q_release_element(element);
            check = 0;
        }
    }

    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 // https://leetcode.com/problems/swap-nodes-in-pairs/
 */
void q_swap(struct list_head *head)
{
    if (!head)
        return;

    struct list_head *cur;
    for (cur = head->next; cur != head && cur->next != head; cur = cur->next)
        list_move(cur, cur->next);
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *next;
    list_for_each_safe (node, next, head) {
        node->next = node->prev;
        node->prev = next;
    }

    head->next = head->prev;
    head->prev = next;

    // list_for_each_safe (node, next, head) {
    //     list_move (node, head);
    // }
}

struct list_head *merge_two_lists(struct list_head *L1, struct list_head *L2)
{
    struct list_head *head = NULL;
    struct list_head **indir = &head, **temp = NULL;

    for (; L1 && L2; *temp = (*temp)->next) {
        element_t *L1_ele = list_entry(L1, element_t, list);
        element_t *L2_ele = list_entry(L2, element_t, list);

        temp = strcmp(L1_ele->value, L2_ele->value) < 0 ? &L1 : &L2;
        *indir = *temp;
        indir = &(*indir)->next;
    }

    *indir = (struct list_head *) ((uintptr_t) L1 | (uintptr_t) L2);

    return head;
}

struct list_head *merge_sort(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    struct list_head *slow = head, *fast;

    for (fast = head->next; fast && fast->next; fast = fast->next->next)
        slow = slow->next;

    struct list_head *left, *right;

    right = slow->next;
    slow->next = NULL;  // no prev

    left = merge_sort(head);
    right = merge_sort(right);

    return merge_two_lists(left, right);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }

    head->prev->next = NULL;
    head->next = merge_sort(head->next);

    struct list_head *tmp = NULL, *prev = head;
    for (tmp = head->next; tmp->next != NULL; tmp = tmp->next) {
        tmp->prev = prev;
        prev = prev->next;
    }
    tmp->prev = prev;
    tmp->next = head;
    head->prev = tmp;
}