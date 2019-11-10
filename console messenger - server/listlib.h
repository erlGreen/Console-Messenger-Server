 //LIST_LISTLIB_H
struct list{
    struct list *next, *previous;
    void *value;
};
struct list* newList();
void addToList(struct list*, void*);
void remFromList(struct list*, void*);
void deleteList(struct list*);
