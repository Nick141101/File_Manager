#include "os_file.h"
#include <cstdio>

int main() {
    file_manager_t fm;
    setup_file_manager(&fm);
    fm.create(1000);
    fm.destroy();
    return 0;
}
