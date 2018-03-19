#include <iostream>
#include <ctime>

using namespace std;

const unsigned int page_size = 50;

class cell_memory {
    char data[page_size];
    bool free;
    int where, table_num;  //where: 0 - T1, 1 - B1, 2 - T2, 3 - B2, 4 - ROM
    clock_t time_use;
public:
    cell_memory() {
        free = true;
        time_use = -1;
        where = -1;
        table_num = -1;
        for(int i = 0; i < page_size; i++) data[i] = 0;
    }

    char *data_output() { return data; };

    int table_num_output() { return table_num; };

    bool free_output() { return free; };

    int where_output() { return where; };

    clock_t time_use_output() { return time_use; };

    void data_change(char s[]) {
        for (int i = 0; i < page_size; i++) data[i] = s[i];
    }

    void table_num_change(int num) { table_num = num; }

    void free_change(bool f) { free = f; }

    void where_change(int destination) { where = destination; };

    void time_use_change(time_t t) { time_use = t; };

    void time_use_reset() { time_use = clock(); };


};

class memory {
public:
    int RAM_size, ROM_size;
    cell_memory *RAM, *ROM;

    memory() {          //конструктор по умолчанию
        RAM_size = 5;
        ROM_size = 10;
        RAM = new cell_memory[RAM_size];
        ROM = new cell_memory[ROM_size];
    }

    memory(int RAM_size, int ROM_size) {            //конструктор
        this->RAM_size = RAM_size;
        this->ROM_size = ROM_size;
        RAM = new cell_memory[RAM_size];
        ROM = new cell_memory[ROM_size];

    }


    void output() {         //вывод содержимого памяти
        cout << "RAM pages: ";
        for (int i = 0; i < RAM_size; i++) puts(RAM[i].data_output());
        cout << endl << "ROM pages: ";
        for (int i = 0; i < ROM_size; i++) puts(ROM[i].data_output());
    }

    void input() {          //ввод содержимого памяти
        auto *s = new char[page_size];
        cout << "Input RAM pages: ";
        for (int i = 0; i < RAM_size; i++) {
            fgets(s, page_size, stdin);
            RAM[i].data_change(s);
            RAM[i].free_change(true);
        }
        cout << endl << "input ROM pages: ";
        for (int i = 0; i < ROM_size; i++) {
            fgets(s, page_size, stdin);
            ROM[i].data_change(s);
            ROM[i].free_change(true);
        }
    }

    /*~memory() {
        delete [] RAM;
        delete [] ROM;
    }*/
};

class ARC {
    int T1_size, ghost_size, nT1, nT2, nB1, nB2, i, n, min_clock_i, min_clock_j;
    cell_memory **pages;
    bool flag;
    char s[page_size];
    memory mem;
public:
    ARC(memory mem) {
        cell_memory **pages = new cell_memory * [mem.RAM_size + mem.ROM_size];
        this->pages = pages;
        this->mem = mem;
        T1_size = mem.RAM_size - 1;
        ghost_size = mem.ROM_size / 3;
        nT1 = nT2 = nB1 = nB2 = 0;
        for (i = 0; i < mem.RAM_size + mem.ROM_size; i++) pages[i] = nullptr;
    }

    int search_min_time(int where) {
        int min, i;
        if (where == 0 || where == 2) {
            for (i = 0; i < mem.RAM_size && mem.RAM[i].where_output() != where; i++);
            min = i;
            while (i < mem.RAM_size - 1) {
                i++;
                if (mem.RAM[i].where_output() == where &&
                    (mem.RAM[i].time_use_output() < mem.RAM[min].time_use_output()))
                    min = i;
            }
        } else {
            for (i = 0; i < mem.ROM_size && mem.ROM[i].where_output() != where; i++);
            min = i;
            while (i < mem.ROM_size - 1) {
                i++;
                if (mem.ROM[i].where_output() == where &&
                    (mem.ROM[i].time_use_output() < mem.ROM[min].time_use_output()))
                    min = i;
            }
        }
        return min;
    }

    void transfer(int num, int to) {
        int i, n;
        switch (to) {
            case 0: {
                if (nT1 == T1_size) evict(0);
                else nT1++;
                pages[num]->free_change(true);
                for (i = 0; i < mem.RAM_size && !mem.RAM[i].free_output(); i++);
                mem.RAM[i].table_num_change(num);
                mem.RAM[i].free_change(false);
                mem.RAM[i].where_change(to);
                mem.RAM[i].time_use_reset();
                mem.RAM[i].data_change(pages[num]->data_output());
                pages[num] = &mem.RAM[i];
                break;
            }
            case 1: {
                nT1--;
                if (nB1 == ghost_size) evict(1);
                else nB1++;
                pages[num]->free_change(true);
                for (i = 0; i < mem.ROM_size && !mem.ROM[i].free_output(); i++);
                mem.ROM[i].table_num_change(num);
                mem.ROM[i].free_change(false);
                mem.ROM[i].where_change(to);
                mem.ROM[i].time_use_reset();
                mem.ROM[i].data_change(pages[num]->data_output());
                pages[num] = &mem.ROM[i];
                break;
            }
            case 2: {
                switch (pages[num]->where_output()) {
                    case 0:
                        nT1--;
                        break;
                    case 1:
                        nB1--;
                        if (T1_size < mem.RAM_size - 1) {
                            if (nT2 == mem.RAM_size - T1_size) {
                                evict(2);
                                nT2--;
                            }
                            T1_size++;
                        }
                        break;
                    case 3:
                        nB2--;
                        if (T1_size > 1) {
                            if (nT1 == T1_size) {
                                evict(0);
                                nT1--;
                            }
                            T1_size--;
                        }
                        break;
                }
                if (nT2 == mem.RAM_size - T1_size) evict(2);
                else nT2++;
                pages[num]->free_change(true);
                for (i = 0; i < mem.RAM_size && !mem.RAM[i].free_output(); i++);
                mem.RAM[i].table_num_change(num);
                mem.RAM[i].free_change(false);
                mem.RAM[i].where_change(to);
                mem.RAM[i].time_use_reset();
                mem.RAM[i].data_change(pages[num]->data_output());
                pages[num] = &mem.RAM[i];
                break;
            }
            case 3: {
                nT2--;
                if (nB2 == ghost_size) evict(3);
                else nB2++;
                pages[num]->free_change(true);
                for (i = 0; i < mem.ROM_size && !mem.ROM[i].free_output(); i++);
                mem.ROM[i].table_num_change(num);
                mem.ROM[i].free_change(false);
                mem.ROM[i].where_change(to);
                mem.ROM[i].time_use_reset();
                mem.ROM[i].data_change(pages[num]->data_output());
                pages[num] = &mem.ROM[i];
                break;
            }
            case 4: {
                if (pages[num]->where_output() == 1) nB1--;
                else nB2--;
                pages[num]->free_change(true);
                for (i = 0; i < mem.ROM_size && !mem.ROM[i].free_output(); i++);
                mem.ROM[i].table_num_change(num);
                mem.ROM[i].free_change(false);
                mem.ROM[i].where_change(to);
                mem.ROM[i].data_change(pages[num]->data_output());
                pages[num] = &mem.ROM[i];
                break;
            }
        }
    }

    void evict(int from) {
        int i, min_clock_i;
        min_clock_i = search_min_time(from);
        switch (from) {
            case 0: {
                if (nB1 == ghost_size) {
                    evict(1);
                    evict(0);
                    nT1--;
                } else transfer(mem.RAM[min_clock_i].table_num_output(), 1);
                break;
            }
            case 1:
                transfer(mem.ROM[min_clock_i].table_num_output(), 4);
                break;
            case 2: {
                if (nB2 == ghost_size) {
                    evict(3);
                    evict(2);
                } else transfer(mem.RAM[min_clock_i].table_num_output(), 3);
                break;
            }
            case 3:
                transfer(mem.ROM[min_clock_i].table_num_output(), 4);
                break;
        }
    }

    void input(int num) {
        if (pages[num] == nullptr) {
            cout << endl << "Input page: ";
            fgets(s, page_size, stdin);
            if (nT1 == T1_size) evict(0);
            for (i = 0; i < mem.RAM_size && !mem.RAM[i].free_output(); i++);
            mem.RAM[i].data_change(s);
            mem.RAM[i].time_use_reset();
            mem.RAM[i].where_change(0);
            mem.RAM[i].free_change(false);
            mem.RAM[i].table_num_change(num);
            pages[num] = &mem.RAM[i];
            nT1++;
        } else cout << "page with this name already exist";
    }

    void output(int num) {
        if (pages[num] == nullptr) cout << "page does not exist";
        else {
            puts(pages[num]->data_output());
            switch (pages[num]->where_output()) {
                case 2: {
                    pages[num]->time_use_reset();
                    break;
                }
                case 4: {
                    transfer(num, 0);
                    break;
                }
                default:
                    transfer(num, 2);
            }
        }
    }

    void interface() { //WIP
        char c;
        int num;
        cout << "i - input, o - output, e - exit";
        c = getchar();
        while(c != 'e') {
            for(int i = 0; i < mem.RAM_size + mem.ROM_size; i++) {
                cout << "Page N" << i << ":    ";
                if(pages[i] != nullptr) cout << pages[i]->data_output() << endl;
                cout << endl;

            }
            if (c == 'i') {
                cout << "input page number to input: ";
                cin >> num;
                input(num);
            }
            if (c == 'o') {
                cout << "input page number to output: ";
                cin >> num;
                output(num);
            }
            cout << "i - input, o - output, e - exit";
            c = getchar();
        };


    }
};

int main() {
    ARC alg(memory (5, 10));
    alg.input(1);
    alg.output(1);
    alg.output(2);

    return 0;
}
