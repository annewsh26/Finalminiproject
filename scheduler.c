#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME 50

typedef struct Event {
    int id;
    char name[MAX_NAME];
    char date[11];
    char time[6];
    int seats;
    struct Event* next;
    struct Event* prev;
} Event;

typedef struct {
    Event* head;
    Event* tail;
    int count;
} Scheduler;

Scheduler* create() {
    Scheduler* s = (Scheduler*)malloc(sizeof(Scheduler));
    s->head = s->tail = NULL;
    s->count = 0;
    return s;
}

int rand_id() {
    return rand() % 90000 + 10000;
}

Event* create_event(int id, char* name, char* date, char* time, int seats) {
    Event* e = (Event*)malloc(sizeof(Event));
    e->id = id ? id : rand_id();
    strcpy(e->name, name);
    strcpy(e->date, date);
    strcpy(e->time, time);
    e->seats = seats;
    e->next = e->prev = NULL;
    return e;
}

void insert(Scheduler* s, char* name, char* date, char* time, int seats) {
    Event* e = create_event(0, name, date, time, seats);
    if (!s->head) {
        s->head = s->tail = e;
    } else {
        s->tail->next = e;
        e->prev = s->tail;
        s->tail = e;
    }
    s->count++;
}

Event* search(Scheduler* s, int id) {
    Event* cur = s->head;
    while (cur) {
        if (cur->id == id) return cur;
        cur = cur->next;
    }
    return NULL;
}

int modify(Scheduler* s, int id, char* name, char* date, char* time, int seats) {
    Event* e = search(s, id);
    if (!e) return 0;
    if (strlen(name) > 0) strcpy(e->name, name);
    if (strlen(date) > 0) strcpy(e->date, date);
    if (strlen(time) > 0) strcpy(e->time, time);
    if (seats > 0) e->seats = seats;
    return 1;
}

int delete(Scheduler* s, int id) {
    Event* cur = s->head;
    while (cur) {
        if (cur->id == id) {
            if (cur->prev) cur->prev->next = cur->next;
            else s->head = cur->next;
            if (cur->next) cur->next->prev = cur->prev;
            else s->tail = cur->prev;
            free(cur);
            s->count--;
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

void display_forward(Scheduler* s, char* output) {
    if (!s->head) {
        sprintf(output, "No events.");
        return;
    }
    char temp[200];
    Event* cur = s->head;
    while (cur) {
        sprintf(temp, "%d|%s|%s|%s|%d\n", cur->id, cur->name, cur->date, cur->time, cur->seats);
        strcat(output, temp);
        cur = cur->next;
    }
}

void display_reverse(Scheduler* s, char* output) {
    if (!s->tail) {
        sprintf(output, "No events.");
        return;
    }
    char temp[200];
    Event* cur = s->tail;
    while (cur) {
        sprintf(temp, "%d|%s|%s|%s|%d\n", cur->id, cur->name, cur->date, cur->time, cur->seats);
        strcat(output, temp);
        cur = cur->prev;
    }
}

void save(Scheduler* s, char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;
    Event* cur = s->head;
    while (cur) {
        fprintf(f, "%d,%s,%s,%s,%d\n", cur->id, cur->name, cur->date, cur->time, cur->seats);
        cur = cur->next;
    }
    fclose(f);
}

void load(Scheduler* s, char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return;
    int id, seats;
    char name[MAX_NAME], date[11], time[6];
    while (fscanf(f, "%d,%[^,],%[^,],%[^,],%d\n", &id, name, date, time, &seats) == 5) {
        Event* e = create_event(id, name, date, time, seats);
        if (!s->head) {
            s->head = s->tail = e;
        } else {
            s->tail->next = e;
            e->prev = s->tail;
            s->tail = e;
        }
        s->count++;
    }
    fclose(f);
}

void generate_random(Scheduler* s, int n) {
    char* names[] = {"Meeting", "Workshop", "Conference", "Training", "Seminar",
                     "Webinar", "Party", "Lunch", "Dinner", "Interview"};
    char* dates[] = {"01/12/2024","05/12/2024","10/12/2024","15/12/2024","20/12/2024"};
    char* times[] = {"09:00","10:30","12:00","14:00","15:30"};
    for (int i = 0; i < n; i++) {
        char name[100];
        sprintf(name, "%s %d", names[rand()%10], rand()%100+1);
        insert(s, name, dates[rand()%5], times[rand()%5], rand()%100+10);
    }
}

void free_scheduler(Scheduler* s) {
    Event* cur = s->head;
    while (cur) {
        Event* tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    free(s);
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    Scheduler* s = create();
    load(s, "events.log");

    if (argc > 1) {
        if (strcmp(argv[1], "insert") == 0 && argc == 6) {
            insert(s, argv[2], argv[3], argv[4], atoi(argv[5]));
            save(s, "events.log");
            printf("%d\n", s->tail->id);
        }
        else if (strcmp(argv[1], "search") == 0 && argc == 3) {
            Event* e = search(s, atoi(argv[2]));
            if (e) printf("%d|%s|%s|%s|%d\n", e->id, e->name, e->date, e->time, e->seats);
            else printf("NOTFOUND\n");
        }
        else if (strcmp(argv[1], "modify") == 0 && argc == 7) {
            int ok = modify(s, atoi(argv[2]), argv[3], argv[4], argv[5], atoi(argv[6]));
            if (ok) { save(s, "events.log"); printf("OK\n"); }
            else printf("FAIL\n");
        }
        else if (strcmp(argv[1], "delete") == 0 && argc == 3) {
            int ok = delete(s, atoi(argv[2]));
            if (ok) { save(s, "events.log"); printf("OK\n"); }
            else printf("FAIL\n");
        }
        else if (strcmp(argv[1], "display_forward") == 0) {
            char out[10000] = "";
            display_forward(s, out);
            printf("%s", out);
        }
        else if (strcmp(argv[1], "display_reverse") == 0) {
            char out[10000] = "";
            display_reverse(s, out);
            printf("%s", out);
        }
        else if (strcmp(argv[1], "generate") == 0 && argc == 3) {
            generate_random(s, atoi(argv[2]));
            save(s, "events.log");
            printf("OK\n");
        }
        else if (strcmp(argv[1], "count") == 0) {
            printf("%d\n", s->count);
        }
    }

    free_scheduler(s);
    return 0;
}