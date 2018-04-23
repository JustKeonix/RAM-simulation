#include <iostream>
#include <ctime>

const unsigned int page_size = 50;

class table;

struct node {
    char *data;
    int num;
    node *next;

    node(char *ptr, int num) {
        this->num = num;
        data = ptr;
        next = nullptr;
    }
};

class memory {
    int fast_size, slow_size;
    char **fast, **slow;
    friend table;
public:
    memory() {} //конструктор по умолчанию

    memory(int fast_size, int slow_size) {            //конструктор
        this->fast_size = fast_size;
        this->slow_size = slow_size;
        fast = new char *[fast_size];
        slow = new char *[slow_size];
        for (int i = 0; i < fast_size; i++) {
            fast[i] = new char[page_size];
            fast[i] = nullptr;
        }
        for (int i = 0; i < slow_size; i++) {
            slow[i] = new char[page_size];
            slow[i] = nullptr;
        }

    }

    int search_free(bool ram) {
        int i;
        if (ram) for (i = 0; i < fast_size && fast[i] != nullptr; i++);
        else for (i = 0; i < slow_size && slow[i] != nullptr; i++);
        return i;
    }

    char *RAM(int num) { return fast[num]; }

    int RAM_size() { return fast_size; }

    void RAM_change(int num, char *c) { fast[num] = c; }

    void RAM_free(char *c) { c = nullptr; }

    char *ROM(int num) { return slow[num]; }

    int ROM_size() { return slow_size; }

    void ROM_change(int num, char *c) { slow[num] = c; }

    void ROM_free(char *c) { c = nullptr; }

    void output() {         //вывод содержимого памяти
        std::cout << "RAM pages: ";
        for (int i = 0; i < fast_size; i++) puts(fast[i]);
        std::cout << std::endl << "ROM pages: ";
        for (int i = 0; i < slow_size; i++) puts(slow[i]);
    }

    void input() {          //ввод содержимого памяти
        std::cout << "Input RAM pages: ";
        for (int i = 0; i < fast_size; i++) fgets(fast[i], page_size, stdin);
        std::cout << std::endl << "input ROM pages: ";
        for (int i = 0; i < slow_size; i++) fgets(slow[i], page_size, stdin);
    }

    /*~memory() {
        delete[] fast;
        delete[] slow;
    }*/
};

class table {
    int place, frequency;          //place: 0 - T1, 1 - B1, 2 - T2, 3 - B2, 4 - ROM
    char *ptr;
    clock_t time;
public:
    table() {
        ptr = new char[page_size];
        ptr = nullptr;
        frequency_reset();
        time = -2;
    };

    int where() { return place; }

    void where_change(int n) { place = n; }

    int frequency_show() { return frequency; }

    void frequency_increase() { frequency++; }

    void frequency_recount() { frequency /= 2; }

    void frequency_reset() { frequency = 0; }

    clock_t clock_show() { return time; }

    void clock_reset() { time = clock(); }

    void clock_set_negative() { time = -2; }

    char *adr() { return ptr; }

    void adr_change(char *p) { ptr = p; }

    void free_cell() { ptr = NULL; }

};

class LFU {
    table *linked_list;
    memory mem;
    int num_of_usage, max_num_of_usage;
public:
    LFU(memory mem) {
        this->mem = mem;
        linked_list = new table[mem.RAM_size() + mem.ROM_size()];
        num_of_usage = 0;
        max_num_of_usage = mem.RAM_size() * 10;
    }

    void frequency_recalculate() {
        for (int i = 0; i < mem.RAM_size() + mem.ROM_size(); i++)
            if (linked_list[i].frequency_show() != 0)
                linked_list[i].frequency_recount();
        num_of_usage = 0;
    }

    int search_min_frequency() {
        int i_min = -1;
        for (int i = 0; i < mem.RAM_size() + mem.ROM_size(); i++)
            if (linked_list[i].frequency_show() != 0 && (linked_list[i].frequency_show() <
                                                         linked_list[i_min].frequency_show()))
                i_min = i;
        return i_min;
    }

    void evict() {
        int i, j;
        i = search_min_frequency();
        j = mem.search_free(false);
        mem.ROM_change(j, linked_list[i].adr());
        linked_list[i].free_cell();
        linked_list[i].adr_change(mem.ROM(j));
        linked_list[i].frequency_reset();
    }

    void transfer(int num) {
        int i, j;
        if (linked_list[num].frequency_show() == 0) {
            if ((i = mem.search_free(true)) == mem.RAM_size()) {
                evict();
                transfer(num);
            } else {
                mem.RAM_change(i, linked_list[num].adr());
                linked_list[num].free_cell();
                linked_list[num].adr_change(mem.RAM(i));
                linked_list[num].frequency_increase();
            }
        } else {
            i = mem.search_free(false);
            mem.ROM_change(i, linked_list[num].adr());
            linked_list[num].free_cell();
            linked_list[num].adr_change(mem.ROM(i));
            linked_list[num].frequency_reset();
        }
    }

    void input(int num) {
        if (linked_list[num].adr() == nullptr) {
            num_of_usage++;
            char *s;
            int i;
            s = new char[page_size];
            std::cout << "input page:";
            std::cin.getline(s, page_size);
            if ((i = mem.search_free(true)) == mem.RAM_size()) {
                evict();
                i = mem.search_free(true);
            }
            mem.RAM_change(i, s);
            linked_list[num].adr_change(mem.RAM(i));
            linked_list[num].frequency_increase();
            if (num_of_usage > max_num_of_usage) frequency_recalculate();
        } else std::cout << "Page already exist, try another name";
    }

    void output(int num) {
        if (linked_list[num].adr() == nullptr) std::cout << "Page doesn't exist";
        else {
            num_of_usage++;
            puts("Page:");
            puts(linked_list[num].adr());
            if (linked_list[num].frequency_show() == 0) transfer(num);
            else linked_list[num].frequency_increase();
            if (num_of_usage > max_num_of_usage) frequency_recalculate();
        }
    }
};

class RND {
    table *linked_list;
    memory mem;
public:
    RND(memory mem) {
        this->mem = mem;
        linked_list = new table[mem.RAM_size() + mem.ROM_size()];
        srand(time(NULL));
    }

    void input(int num) {
        if (linked_list[num].adr() == nullptr) {
            char *s;
            int i, random;
            s = new char[page_size];
            std::cout << "input page:";
            std::cin.getline(s, page_size);
            if ((i = mem.search_free(true)) == mem.RAM_size()) {
                random = rand() % mem.RAM_size();
                mem.RAM_change(random, s);
                linked_list[num].adr_change(s);
            } else {
                mem.RAM_change(i, s);
                linked_list[num].adr_change(s);
            }
        } else std::cout << "Page already exist, try another name";
    }

    void output(int num) {
        if (linked_list[num].adr() == nullptr) std::cout << "Page doesn't exist";
        else {
            puts("Page:");
            puts(linked_list[num].adr());
        }
    }
};

class LRU {
    table *linked_list;
    memory mem;
public:
    LRU(memory mem) {
        this->mem = mem;
        linked_list = new table[mem.RAM_size() + mem.ROM_size()];
    }

    int search_min_time() {
        int i, i_min;
        for (i = 0; mem.RAM_size() + mem.ROM_size() && linked_list[i].clock_show() == -2; i++);
        i_min = i;
        while (i < mem.RAM_size() + mem.ROM_size() - 1) {
            i++;
            if (linked_list[i].clock_show() != -2 && linked_list[i].clock_show() < linked_list[i_min].clock_show() &&
                linked_list[i].adr() != nullptr)
                i_min = i;
        }
        return i_min;
    }

    void evict() {
        int i, j;
        i = search_min_time();
        j = mem.search_free(false);
        mem.ROM_change(j, linked_list[i].adr());
        linked_list[i].free_cell();
        linked_list[i].adr_change(mem.ROM(j));
        linked_list[i].clock_set_negative();
    }

    void transfer(int num) {
        int i, j;
        if (linked_list[num].clock_show() == -2) {
            if ((i = mem.search_free(true)) == mem.RAM_size()) {
                evict();
                transfer(num);
            } else {
                mem.RAM_change(i, linked_list[num].adr());
                linked_list[num].free_cell();
                linked_list[num].adr_change(mem.RAM(i));
                linked_list[num].clock_reset();
            }
        } else {
            i = mem.search_free(false);
            mem.ROM_change(i, linked_list[num].adr());
            linked_list[num].free_cell();
            linked_list[num].adr_change(mem.ROM(i));
            linked_list[num].clock_set_negative();
        }
    }

    void input(int num) {
        if (linked_list[num].adr() == nullptr) {
            char *s;
            int i;
            s = new char[page_size];
            std::cout << "input page:";
            std::cin.getline(s, page_size);
            if ((i = mem.search_free(true)) == mem.RAM_size()) {
                evict();
                i = mem.search_free(true);
            }
            mem.RAM_change(i, s);
            linked_list[num].adr_change(mem.RAM(i));
            linked_list[num].clock_reset();
        } else std::cout << "Page already exist, try another name";
    }

    void output(int num) {
        if (linked_list[num].adr() == nullptr) std::cout << "Page doesn't exist";
        else {
            puts("Page:");
            puts(linked_list[num].adr());
            if (linked_list[num].clock_show() == -2) transfer(num);
            else linked_list[num].clock_reset();
        }
    }
};

class MRU {
    table *linked_list;
    memory mem;
public:
    MRU(memory mem) {
        this->mem = mem;
        linked_list = new table[mem.RAM_size() + mem.ROM_size()];
    }

    int search_max_time() {
        int i, i_max;
        for (i = 0; mem.RAM_size() + mem.ROM_size() && linked_list[i].clock_show() == -2; i++);
        i_max = i;
        while (i < mem.RAM_size() + mem.ROM_size() - 1) {
            i++;
            if (linked_list[i].clock_show() != -2 && linked_list[i].clock_show() > linked_list[i_max].clock_show() &&
                linked_list[i].adr() != nullptr)
                i_max = i;
        }
        return i_max;
    }

    void evict() {
        int i, j;
        i = search_max_time();
        j = mem.search_free(false);
        mem.ROM_change(j, linked_list[i].adr());
        linked_list[i].free_cell();
        linked_list[i].adr_change(mem.ROM(j));
        linked_list[i].clock_set_negative();
    }

    void transfer(int num) {
        int i, j;
        if (linked_list[num].clock_show() == -2) {
            if ((i = mem.search_free(true)) == mem.RAM_size()) {
                evict();
                transfer(num);
            } else {
                mem.RAM_change(i, linked_list[num].adr());
                linked_list[num].free_cell();
                linked_list[num].adr_change(mem.RAM(i));
                linked_list[num].clock_reset();
            }
        } else {
            i = mem.search_free(false);
            mem.ROM_change(i, linked_list[num].adr());
            linked_list[num].free_cell();
            linked_list[num].adr_change(mem.ROM(i));
            linked_list[num].clock_set_negative();
        }
    }

    void input(int num) {
        if (linked_list[num].adr() == nullptr) {
            char *s;
            int i;
            s = new char[page_size];
            std::cout << "input page:";
            std::cin.getline(s, page_size);
            if ((i = mem.search_free(true)) == mem.RAM_size()) {
                evict();
                i = mem.search_free(true);
            }
            mem.RAM_change(i, s);
            linked_list[num].adr_change(mem.RAM(i));
            linked_list[num].clock_reset();
        } else std::cout << "Page already exist, try another name";
    }

    void output(int num) {
        if (linked_list[num].adr() == nullptr) std::cout << "Page doesn't exist";
        else {
            puts("Page:");
            puts(linked_list[num].adr());
            if (linked_list[num].clock_show() == -2) transfer(num);
            else linked_list[num].clock_reset();
        }
    }
};

class FIFO {
    table *linked_list;
    memory mem;
    node *Head, *Tail;
    int size;
public:
    FIFO(memory mem) {
        this->mem = mem;
        linked_list = new table[mem.RAM_size() + mem.ROM_size()];
        Head = Tail = nullptr;
        size = 0;
    }

    void add(char *p, int num) {
        node *newnode = new node(p, num);
        if (Head == nullptr) Head = newnode;
        else Tail->next = newnode;
        Tail = newnode;
    }

    void del() {
        node *tmp = Head;
        char *p = Head->data;
        int num = Head->num;
        Head = Head->next;
        int i, j;
        i = mem.search_free(false);
        mem.ROM_change(i, p);
        linked_list[num].free_cell();
        linked_list[num].adr_change(mem.ROM(i));
        linked_list[num].where_change(1);
        delete tmp;
    }

    void input(int num) {
        if (linked_list[num].adr() == nullptr) {
            char *s;
            int i, j, k;
            s = new char[page_size];
            std::cout << "input page:";
            std::cin.getline(s, page_size);
            if ((i = mem.search_free(true)) == mem.RAM_size()) {
                del();
                i = mem.search_free(true);
            }
            mem.RAM_change(i, s);
            linked_list[num].adr_change(mem.RAM(i));
            linked_list[num].where_change(0);
            add(mem.RAM(i), num);
        } else std::cout << "Page already exist, try another name";
    }

    void output(int num) {
        if (linked_list[num].adr() == nullptr) std::cout << "Page doesn't exist";
        else {
            int i;
            puts("Page:");
            puts(linked_list[num].adr());
            if (linked_list[num].where() == 1) {
                if ((i = mem.search_free(true)) == mem.RAM_size()) {
                    del();
                    i = mem.search_free(true);
                }
                mem.RAM_change(i, linked_list[num].adr());
                linked_list[num].free_cell();
                linked_list[num].adr_change(mem.RAM(i));
                linked_list[num].where_change(0);
                add(mem.RAM(i), num);
            }
        }
    }
};

class ARC {
    int T1_size, ghost_size, nT1, nT2, nB1, nB2;
    table *linked_list;
    memory mem;
public:
    ARC() {}

    ARC(memory mem) {
        this->mem = mem;
        linked_list = new table[mem.RAM_size() + mem.ROM_size()];
        T1_size = mem.RAM_size() - 2;
        ghost_size = mem.ROM_size() / 3;
        nT1 = nT2 = nB1 = nB2 = 0;
    }

    int search_min_time(int where) {
        int min, i;
        for (i = 0; i < mem.RAM_size() + mem.ROM_size() && linked_list[i].where() != where; i++);
        min = i;
        while (i < mem.RAM_size() + mem.ROM_size() - 1) {
            i++;
            if (linked_list[i].where() == where && linked_list[i].clock_show() < linked_list[min].clock_show() &&
                linked_list[i].adr() != nullptr)
                min = i;
        }
        return min;
    }

    void transfer(int num, int to) {
        int i;
        switch (to) {
            case 0: {
                if (nT1 == T1_size) evict(to);
                nT1++;
                i = mem.search_free(true);
                mem.RAM_change(i, linked_list[num].adr());
                linked_list[num].free_cell();
                linked_list[num].adr_change(mem.RAM(i));
                linked_list[num].where_change(to);
                linked_list[num].clock_reset();
                break;
            }
            case 1: {
                if (nB1 == ghost_size) evict(to);
                nB1++;
                i = mem.search_free(false);
                mem.ROM_change(i, linked_list[num].adr());
                linked_list[num].free_cell();
                linked_list[num].adr_change(mem.ROM(i));
                linked_list[num].where_change(to);
                linked_list[num].clock_reset();
                nT1--;
                break;
            }
            case 2: {
                switch (linked_list[num].where()) {
                    case 0: {
                        if (nT2 == mem.RAM_size() - T1_size) evict(to);
                        nT2++;
                        linked_list[num].where_change(to);
                        linked_list[num].clock_reset();
                        nT1--;
                        break;
                    }
                    case 1: {
                        if (mem.RAM_size() - T1_size > 2) {
                            if (nT2 == mem.RAM_size() - T1_size) evict(to);
                            T1_size++;
                        }
                        if (nT2 == mem.RAM_size() - T1_size) evict(to);
                        nT2++;
                        i = mem.search_free(true);
                        mem.RAM_change(i, linked_list[num].adr());
                        linked_list[num].free_cell();
                        linked_list[num].adr_change(mem.RAM(i));
                        linked_list[num].where_change(to);
                        linked_list[num].clock_reset();
                        nB1--;
                        break;
                    }
                    case 3: {
                        if (T1_size > 2) {
                            if (nT1 == T1_size) evict(0);
                            T1_size--;
                        } else if (nT2 == mem.RAM_size() - T1_size) evict(to);
                        nT2++;
                        i = mem.search_free(true);
                        mem.RAM_change(i, linked_list[num].adr());
                        linked_list[num].free_cell();
                        linked_list[num].adr_change(mem.RAM(i));
                        linked_list[num].where_change(to);
                        linked_list[num].clock_reset();
                        nB2--;
                        break;
                    }
                }
                break;
            }
            case 3: {
                if (nB2 == ghost_size) evict(to);
                nB2++;
                i = mem.search_free(false);
                mem.ROM_change(i, linked_list[num].adr());
                linked_list[num].free_cell();
                linked_list[num].adr_change(mem.ROM(i));
                linked_list[num].where_change(to);
                linked_list[num].clock_reset();
                nT2--;
                break;
            }
            case 4: {
                switch (linked_list[num].where()) {
                    case 1: {
                        nB1--;
                        i = mem.search_free(false);
                        mem.ROM_change(i, linked_list[num].adr());
                        linked_list[num].free_cell();
                        linked_list[num].adr_change(mem.ROM(i));
                        linked_list[num].where_change(to);
                        break;
                    }
                    case 3: {
                        nB2--;
                        i = mem.search_free(false);
                        mem.ROM_change(i, linked_list[num].adr());
                        linked_list[num].free_cell();
                        linked_list[num].adr_change(mem.ROM(i));
                        linked_list[num].where_change(to);
                        break;
                    }
                }
                break;
            }
        }
    }

    void evict(int from) {
        int i, min_clock_i = search_min_time(from);
        switch (from) {
            case 0: {
                if (nB1 == ghost_size) {
                    evict(1);
                    evict(0);
                } else transfer(min_clock_i, 1);
                break;
            }
            case 2: {
                if (nB2 == ghost_size) {
                    evict(3);
                    evict(2);
                } else transfer(min_clock_i, 3);
                break;
            }
            default:
                transfer(min_clock_i, 4);
                break;
        }
    }

    void input(int num) {
        if (linked_list[num].adr() == nullptr) {
            char *s;
            int i;
            std::cout << "input page:";
            s = new char[page_size];
            std::cin.getline(s, page_size);
            if (nT1 == T1_size) evict(0);
            nT1++;
            i = mem.search_free(true);
            mem.RAM_change(i, s);
            linked_list[num].adr_change(mem.RAM(i));
            linked_list[num].where_change(0);
            linked_list[num].clock_reset();
        } else std::cout << "Page already exist, try another name";
    }

    void output(int num) {
        if (linked_list[num].adr() == nullptr) std::cout << "Page doesn't exist";
        else {
            puts("Page:");
            puts(linked_list[num].adr());
        }
    }
};

int main() {
    /*ARC alg(memory(4, 8));
    alg.input(1);
    alg.input(2);
    alg.input(1);
    alg.output(1);
    alg.output(2);*/
    return 0;
}

