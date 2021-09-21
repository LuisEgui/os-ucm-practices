#include <stdio.h>

typedef struct {
    char data[4096];
    int key;
} item;

item array[] = {
    {"bill", 3},
    {"neil", 4},
    {"john", 2},
    {"rick", 5},
    {"alex", 1},
};

void swap(item *fpos, item *spos) {
    item tmp = *fpos;
    *fpos = *spos;
    *spos = tmp;
}

void sort(item *a, int n) {
    int s = 1;

    for(int i = n-1; i > 0 && s == 1; i--) {
        s = 0;
        for(int j = 0; j < i; j++) {
            if(a[j].key > a[j+1].key) {
                swap(&a[j], &a[j+1]);
                s = 1;
            }
        }
    }
}

int main() {
    int i;
    sort(array,5);
    for(i = 0; i < 5; i++)
        printf("array[%d] = {%s, %d}\n",
                i, array[i].data, array[i].key);
    return 0;
}
