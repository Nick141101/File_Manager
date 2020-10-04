#include "os_file.h"
#include <cstdlib>
#include <cstring>
#include <cctype>

struct Directory {
    char name[33] = "";
    int size = 0;
    struct Directory *parent = nullptr;
    struct Directory **children = nullptr;
    int children_count = 0;
};

int is_manager_created = 0;
Directory *root = nullptr;
Directory *current_dir = nullptr;
int free_disk_size = 0;

int create(int disk_size) {
    if (is_manager_created != 0)
        return 0;
    root = (Directory *) malloc(sizeof(Directory));
    if (root == nullptr)
        return 0;
    current_dir = root;
    strcpy(root->name, "/");
    root->size = 0;
    root->parent = nullptr;
    root->children = nullptr;
    root->children_count = 0;
    is_manager_created = 1;
    free_disk_size = disk_size;
    return 1;
}

void destroy_dir(Directory *curr_dir) {
    if (curr_dir->children != nullptr) {
        for (int i = 0; i < curr_dir->children_count; i++)
            destroy_dir(curr_dir->children[i]);
        free(curr_dir->children);
    }
    if (current_dir == curr_dir)
        current_dir = root;
    free(curr_dir);
}

int destroy() {
    if (!is_manager_created)
        return 0;
    destroy_dir(root);
    current_dir = nullptr;
    is_manager_created = 0;
    free_disk_size = 0;
    return 1;
}

Directory *find_dir(char *path) {
    Directory *temp_dir;
    if (*path == '/') {
        temp_dir = root;
        path++;
        if (*path == '\0') return temp_dir;
    } else temp_dir = current_dir;
    while (true) {
        int i;
        char name_of_next_dir[33];
        for (i = 0; i < 32 && *path != '/' && *path != '\0'; i++, path++) {
            if (isalnum(*path) || *path == '_' || *path == '.')
                name_of_next_dir[i] = *path;
            else return nullptr;
        }
        if (i == 0) return nullptr;
        name_of_next_dir[i] = '\0';
        switch (*path) {
            case '/': {
                path++;
                if (strcmp(name_of_next_dir, "..") == 0) {
                    temp_dir = temp_dir->parent;
                } else if (strcmp(name_of_next_dir, ".") != 0) {
                    int flag = 0;
                    for (int j = 0; j < temp_dir->children_count; j++)
                        if (strcmp(temp_dir->children[j]->name, name_of_next_dir) == 0) {
                            flag = 1;
                            temp_dir = temp_dir->children[j];
                            break;
                        }
                    if (flag == 0) return nullptr;
                }
                break;
            }
            case '\0': {
                if (strcmp(name_of_next_dir, ".") == 0)
                    return temp_dir;
                else if (strcmp(name_of_next_dir, "..") == 0)
                    return temp_dir->parent;
                for (int j = 0; j < temp_dir->children_count; j++)
                    if (strcmp(temp_dir->children[j]->name, name_of_next_dir) == 0)
                        return temp_dir->children[j];
                return nullptr;
            }
            default:
                return nullptr;
        }
    }
}

int add_dir_or_file(char *path, int size) {
    Directory *temp_dir;
    if (*path == '/') {
        temp_dir = root;
        path++;
    } else temp_dir = current_dir;
    while (true) {
        int i;
        char name_of_next_dir[33];
        for (i = 0; i < 32 && *path != '/' && *path != '\0'; ++i, ++path) {
            if (isalnum(*path) || *path == '_' || *path == '.')
                name_of_next_dir[i] = *path;
            else return 0;
        }
        if (i == 0) return 0;
        name_of_next_dir[i] = '\0';
        switch (*path) {
            case '/': {
                path++;
                if (strcmp(name_of_next_dir, "..") == 0)
                    temp_dir = temp_dir->parent;
                else if (strcmp(name_of_next_dir, ".") != 0) {
                    int flag = 0;
                    for (int j = 0; j < temp_dir->children_count; j++)
                        if (strcmp(temp_dir->children[j]->name, name_of_next_dir) == 0) {
                            flag = 1;
                            temp_dir = temp_dir->children[j];
                            break;
                        }
                    if (flag == 0) return 0;
                }
                break;
            }
            case '\0': {
                if (strcmp(name_of_next_dir, ".") == 0 || strcmp(name_of_next_dir, "..") == 0)
                    return 0;
                if (temp_dir->children != nullptr)
                    for (int j = 0; j < temp_dir->children_count; j++)
                        if (strcmp(temp_dir->children[j]->name, name_of_next_dir) == 0)
                            return 0;
                free_disk_size -= size;
                if (temp_dir->children == nullptr)
                    temp_dir->children = (Directory **) malloc(sizeof(Directory *));
                else
                    temp_dir->children = (Directory **) realloc(temp_dir->children, (temp_dir->children_count + 1) * sizeof(Directory *));
                temp_dir->children[temp_dir->children_count] = (Directory *) malloc(sizeof(Directory));
                strcpy(temp_dir->children[temp_dir->children_count]->name, name_of_next_dir);
                temp_dir->children[temp_dir->children_count]->size = size;
                temp_dir->children[temp_dir->children_count]->children = nullptr;
                temp_dir->children[temp_dir->children_count]->children_count = 0;
                temp_dir->children[temp_dir->children_count]->parent = temp_dir;
                temp_dir->children_count++;
                return 1;
            }
            default:
                return 0;
        }
    }
}

int create_dir(const char *path) {
    if (is_manager_created == 0)
        return 0;
    char *str_point = (char *) malloc((strlen(path) + 1) * sizeof(char));
    strcpy(str_point, path);
    int flag = add_dir_or_file(str_point, 0);
    free(str_point);
    return flag;
}

int create_file(const char *path, int file_size) {
    if (is_manager_created == 0 || file_size <= 0 || free_disk_size < file_size)
        return 0;
    char *str_point = (char *) malloc((strlen(path) + 1) * sizeof(char));
    strcpy(str_point, path);
    int flag = add_dir_or_file(str_point, file_size);
    free(str_point);
    return flag;
}

void count_size(Directory *temp_dir, int *size) {
    *size += temp_dir->size;
    for (int i = 0; i < temp_dir->children_count; i++)
        count_size(temp_dir->children[i], size);
}

int size(const char *path) {
    if (is_manager_created == 0)
        return -1;
    char *str_point = (char *) malloc((strlen(path) + 1) * sizeof(char));
    strcpy(str_point, path);
    Directory *temp_dir = find_dir(str_point);
    free(str_point);
    if (temp_dir == nullptr)
        return -1;
    int size_of_dir = 0;
    count_size(temp_dir, &size_of_dir);
    return size_of_dir;
}

int change_dir(const char *path) {
    if (is_manager_created == 0)
        return 0;
    char *str_point = (char *) malloc((strlen(path) + 1) * sizeof(char));
    strcpy(str_point, path);
    Directory *temp = find_dir(str_point);
    free(str_point);
    if (temp == nullptr || temp->size != 0)
        return 0;
    current_dir = temp;
    return 1;
}

void delete_from_parent(Directory *child) {
    Directory *parent = child->parent;
    for (int i = 0; i < parent->children_count; i++) {
        if (parent->children[i] == child) {
            Directory *t = parent->children[i];
            parent->children[i] = parent->children[parent->children_count - 1];
            parent->children[parent->children_count - 1] = t;
            parent->children = (Directory**)realloc(parent->children, (parent->children_count - 1) * sizeof(Directory*));
            parent->children_count--;
            return;
        }
    }
}

int remove(const char *path, int recursive) {
    if (is_manager_created == 0)
        return 0;
    char *str_point = (char *) malloc((strlen(path) + 1) * sizeof(char));
    if (str_point == nullptr) return 0;
    strcpy(str_point, path);
    Directory *temp_dir = find_dir(str_point);
    free(str_point);
    if (temp_dir == nullptr)
        return 0;
    if (temp_dir->children_count != 0 && recursive == 0)
        return 0;
    int size_of_dir = size(path);
    delete_from_parent(temp_dir);
    destroy_dir(temp_dir);
    free_disk_size += size_of_dir;
    return 1;
}

void get_cur_dir(char *dst) {
    if (is_manager_created == 0) return;
    Directory *dir = current_dir;
    int length = 0;
    while (dir->parent != nullptr) {
        char letter[33];
        strcpy(letter, dir->name);
        for (int i = strlen(letter) - 1; i >= 0; length++, i--)
            dst[length] = letter[i];
        dst[length++] = '/';
        dir = dir->parent;
    }
    for (int i = 0; i < length / 2; i++){
        char t = dst[i];
        dst[i] = dst[length - 1 - i];
        dst[length - 1 - i] = t;
    }
    if (current_dir == root) dst[length++] = '/';
    dst[length] = '\0';
}

void setup_file_manager(file_manager_t *fm) {
    fm->create = create;
    fm->destroy = destroy;
    fm->create_dir = create_dir;
    fm->create_file = create_file;
    fm->size = size;
    fm->change_dir = change_dir;
    fm->remove = remove;
    fm->get_cur_dir = get_cur_dir;
}