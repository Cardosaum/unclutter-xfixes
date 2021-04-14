// vim:ts=4:sw=4:expandtab
#include "all.h"

void bail(char *message) {
    ELOG("Received error: %s", message);
    errx(EXIT_FAILURE, "%s", message);
}

long parse_int(char *str) {
    char *endptr = NULL;
    long parsed = strtol(str, &endptr, 10);
    if (parsed == LONG_MIN || parsed == LONG_MAX ||
            parsed < 0 || endptr == str) {

        return -1;
    }

    return parsed;
}

double parse_double(char *str) {
    char *endptr = NULL;
    double parsed = strtod(str, &endptr);
    if (parsed == -DBL_MAX || parsed == DBL_MAX ||
            parsed < 0 || endptr == str) {

        return -1;
    }

    return parsed;
}

void parse_buttons_numbers(char *str, ignore_buttons_t *ignore_buttons) {
    char *button = strtok(str, ",");
    while (button != NULL) {
        long number = parse_int(button);
        button = strtok(NULL, ",");
        if (number == -1) {
            continue;
        }

        ignore_buttons->count++;
        unsigned int *buttons = (unsigned int *)realloc(ignore_buttons->buttons,
            ignore_buttons->count * sizeof(unsigned int));
        if (buttons == NULL) {
            free(ignore_buttons->buttons);
            bail("Cannot reallocate memory for ignore-buttons");
        } else {
            ignore_buttons->buttons = buttons;
        }

        ignore_buttons->buttons[ignore_buttons->count - 1] = number;
    }

    if (ignore_buttons->count == 0) {
        bail("--ignore-buttons was specified, but no button could be parsed.");
    }
}

window_jitter_t *parse_window_jitter(char* str) {
    window_jitter_t *window_jitter;
    char* name;
    long jitter;

    window_jitter = calloc(sizeof(window_jitter_t), 1);
    if (window_jitter == NULL)
        bail("Cannot allocate memory for new widow_jitter");

    if (!strstr(str, WINDOW_JITTER_SEP)) {
        window_jitter->jitter = -1;
        return window_jitter;
    }

    name = strtok(str, WINDOW_JITTER_SEP);
    jitter = parse_int(strtok(NULL, WINDOW_JITTER_SEP));

    window_jitter->name = name;
    window_jitter->len = name ? strlen(name) : 0;
    window_jitter->jitter = jitter;
    window_jitter->next = NULL;

    return window_jitter;
}

void add_window_jitter(window_jitter_t **head, window_jitter_t *window_jitter) {
    if (*head == NULL) {
        *head = window_jitter;
    } else {
        window_jitter_t *last_node = *head;

        while (last_node->next != NULL) {
            last_node = last_node->next;
        }
        last_node->next = window_jitter;
    }
    return;
}

void print_window_jitter(window_jitter_t **head) {
    if (*head == NULL) {
        return;
    } else {
        window_jitter_t *last_node = *head;

        printf("\n===\n");
        while (last_node->next != NULL) {
            printf("pointer: %p\n", &last_node);
            printf("name: %s\n", last_node->name);
            printf("len: %i\n", last_node->len);
            printf("jitter: %li\n", last_node->jitter);
            printf("next: %p\n", last_node->next);
            printf("---\n");
            last_node = last_node->next;
        }
        printf("pointer: %p\n", &last_node);
        printf("name: %s\n", last_node->name);
        printf("len: %i\n", last_node->len);
        printf("jitter: %li\n", last_node->jitter);
        printf("next: %p\n", last_node->next);
        printf("===\n\n");
    }
    return;
}

long jitter_window_match(Window win) {
    window_jitter_t *window_jitter;
    XClassHint hint;
    char *name;
    long jitter = 0;
    int xfetchname;
    int xgetclasshint;

    if (config.window_jitter_list == NULL)
        return jitter;
    else
        window_jitter = config.window_jitter_list;

    xfetchname = XFetchName(display, win, &name);
    xgetclasshint = XGetClassHint(display, win, &hint);

    if (xfetchname || xgetclasshint) {
        while (window_jitter != NULL) {
            if (xfetchname) {
                if (strncasecmp(window_jitter->name, name, window_jitter->len) == 0) {
                    jitter = window_jitter->jitter;
                    break;
                }
            }
            if (xgetclasshint) {
                if (strncasecmp(window_jitter->name, hint.res_name, window_jitter->len) == 0 ||
                    strncasecmp(window_jitter->name, hint.res_class, window_jitter->len) == 0) {
                    jitter = window_jitter->jitter;
                    break;
                }
            }
            window_jitter = window_jitter->next;
        }
        XFree(name);
        XFree(hint.res_name);
        XFree(hint.res_class);
        if (jitter) return jitter;
    }

    return jitter;
}

bool is_on_window_jitter_list(Window win) {
    Window child_in;
    Window win_in;
    Window win_dummy;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int mask;

    child_in = win_in = win;

    do {
        win_in = child_in;
        if (jitter_window_match(win_in)) {
            return true;
        }
    } while (XQueryPointer(display, win_in, &win_dummy, &child_in, &root_x, &root_y, &win_x, &win_y, &mask)
             && child_in != None);

    return false;
}

long get_window_jitter(Window win) {
    Window child_in;
    Window win_in;
    Window win_dummy;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int mask;
    long jitter = 0;

    child_in = win_in = win;

    do {
        win_in = child_in;
        if ((jitter = jitter_window_match(win_in))) {
            return jitter;
        }
    } while (XQueryPointer(display, win_in, &win_dummy, &child_in, &root_x, &root_y, &win_x, &win_y, &mask)
             && child_in != None);

    return jitter;
}
