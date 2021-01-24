#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <regex.h>


#define LINE_LENGTH 1024
const char *builtins[] = {"cd", "help", "exit", "head", "grep", "fancy_time", NULL};
bool enter = false;
char prev[LINE_LENGTH];

void zero() {
    printf("  ___\n"
           " / _ \\\n"
           "| | | |\n"
           "| |_| |\n"
           " \\___/\n");
}

void one() {
    printf(" _\n"
           "/ |\n"
           "| |\n"
           "| |\n"
           "|_|\n");
}

void two() {
    printf(" ____\n"
           "|___ \\\n"
           "  __) |\n"
           " / __/\n"
           "|_____|\n");

}

void three() {
    printf(" _____\n"
           "|___ /\n"
           "  |_ \\\n"
           " ___) |\n"
           "|____/\n");
}

void four() {
    printf(" _  _\n"
           "| || |\n"
           "| || |_\n"
           "|__   _|\n"
           "   |_|\n");
}

void five() {
    printf(" ____\n"
           "| ___|\n"
           "|___ \\\n"
           " ___) |\n"
           "|____/\n");
}

void six() {
    printf("  __\n"
           " / /_\n"
           "| '_ \\\n"
           "| (_) |\n"
           " \\___/\n");
}

void seven() {
    printf(" _____\n"
           "|___  |\n"
           "   / /\n"
           "  / /\n"
           " /_/\n");
}

void eight() {
    printf("  ___\n"
           " ( _ )\n"
           " / _ \\\n"
           "| (_) |\n"
           " \\___/\n");
}

void nine() {
    printf("  ___\n"
           " / _ \\\n"
           "| (_) |\n"
           " \\__, |\n"
           "   /_/\n");
}

void display_digit(int digit) {
    if (digit == 0) zero();
    else if (digit == 1) one();
    else if (digit == 2) two();
    else if (digit == 3) three();
    else if (digit == 4) four();
    else if (digit == 5) five();
    else if (digit == 6) six();
    else if (digit == 7) seven();
    else if (digit == 8) eight();
    else if (digit == 9) nine();
}

void builtin_fancy_time() {
    time_t now;
    struct tm *now_tm;
    int hour, minute;

    now = time(NULL);
    now_tm = localtime(&now);
    hour = now_tm->tm_hour;
    minute = now_tm->tm_min;
    if (hour > 9) {
        display_digit(hour / 10);
        display_digit(hour % 10);
    } else {
        display_digit(hour);
    }

    printf(" _   _\n"
          "(_) (_)\n");

    if (minute < 9) {
        display_digit(0);
        display_digit(minute);
    } else {
        display_digit(minute / 10);
        display_digit(minute % 10);
    }
}

bool is_special_char(char letter) {
    if (letter == '[' || letter == ']' || letter == '{' || letter == '}' ||
        letter == '*' || letter == '+' || letter == '?' || letter == '|' ||
        letter == '$' || letter == '^' || letter == '.' || letter == '\\')
        return true;
    else return false;
}

void print_error(char *error) {
    printf("\033[0;31m");
    printf("%s\n", error);
}

char *parse_argument_in_quotes(char *command[], int words_count) {
    bool add_space_to_end = false;
    bool add_space_to_start = false;
    if (strlen(command[words_count - 2]) == 1) add_space_to_end = true;
    if (strlen(command[1]) == 1) add_space_to_start = true;

    char *str = malloc(sizeof(char) * 256);
    memmove(command[1], command[1] + 1, strlen(command[1]));
    command[words_count - 2][strlen(command[words_count - 2]) - 1] = '\0';

    int i, j;
    int position = 0;
    int current_position = 0;
    for (i = 1; i < words_count - 1; ++i) {
        int len = strlen(command[i]);
        for (j = 0; j < len; ++j) {
            if (is_special_char(command[i][j])) {
                str[position] = '\\';
                position++;
            }
            if (current_position + 1 == len) {
                str[position] = command[i][j];
                str[position + 1] = ' ';
                current_position = 0;
                position++;
            } else {
                str[position] = command[i][j];
                current_position++;
            }
            position++;
        }
    }

    if (add_space_to_start) {
        int k;
        int len = strlen(str);
        for (k = len - 1; k >= 0; --k) {
            str[k + 1] = str[k];
        }
        str[0] = ' ';
        position++;
    }

    if (add_space_to_end) str[position - 1] = ' ';
    else str[position - 1] = '\0';

    return str;
}

void display_time() {
    time_t now;
    struct tm *now_tm;
    int hour, minute;

    now = time(NULL);
    now_tm = localtime(&now);
    hour = now_tm->tm_hour;
    minute = now_tm->tm_min;

    printf("{%d:%d}", hour, minute);
}

void builtin_grep(char *command[], int words_count) {
    if (words_count > 2) {
        char *str;
        int mode;
        if (words_count == 3) {
            mode = 1;
            if (command[1][0] == '"' && command[1][strlen(command[1]) - 1] == '"') {
                str = parse_argument_in_quotes(command, words_count);
            } else {
                str = command[1];
            }
        } else if (words_count > 3) {
            if (command[1][0] == '"' && command[words_count - 2][strlen(command[words_count - 2]) - 1] == '"') {
                mode = 2;
                str = parse_argument_in_quotes(command, words_count);
            } else {
                print_error("Wrong format for grep.");
                return;
            }
        } else {
            print_error("Wrong format for grep.");
            return;
        }

        char *file_name = mode == 1 ? command[2] : command[words_count - 1];
        char line[LINE_LENGTH];
        FILE *file;
        file = fopen(file_name, "rb");

        if (file != NULL) {
            while (fgets(line, LINE_LENGTH, file)) {
                regex_t regex;
                int value;
                int matched;

                value = regcomp(&regex, str, 0);

                if (value == 0) {
                    matched = regexec(&regex, line, 0, NULL, 0);
                    if (!matched) {
                        printf("%s", line);
                    }
                }
            }
        } else {
            print_error("No such file or directory.");
            return;
        }
    } else {
        print_error("Wrong format for grep.");
        return;
    }
}

void builtin_head(char *command[], int words_count) {
    int mode = 0;
    if (words_count < 5 && words_count > 2 && command[1][0] == '-') {
        if (words_count == 3) {
            if (isdigit(command[1][1])) {
                mode = 1;
                int i;
                for (i = 1; i < strlen(command[1]); ++i) {
                    if (!isdigit(command[1][i])) {
                        print_error("This is not a valid number.");
                        return;
                    }
                }
                memmove(command[1], command[1] + 1, strlen(command[1]));
            } else if (command[1][1] == 'n') {
                mode = 2;
                int i;
                for (i = 2; i < strlen(command[1]); ++i) {
                    if (!isdigit(command[1][i])) {
                        print_error("This is not a valid number.");
                        return;
                    }
                }
                memmove(command[1], command[1] + 2, strlen(command[1]));
            } else {
                print_error("This is not a valid number.");
                return;
            }
        } else if (words_count == 4) {
            if (!strcmp(command[1], "-n")) {
                mode = 3;
                int i;
                for (i = 0; i < strlen(command[2]); ++i) {
                    if (!isdigit(command[2][i])) {
                        print_error("This is not a valid number.");
                        return;
                    }
                }
            }
        }

        int lines_to_display = mode == 3 ? atoi(command[2]) : atoi(command[1]);
        char *file_name = mode == 3 ? command[3] : command[2];
        char line[LINE_LENGTH];
        FILE *file;
        file = fopen(file_name, "rb");
        int lines_count = 0;

        if (file != NULL) {
            while ((fgets(line, LINE_LENGTH, file)) && lines_count < lines_to_display) {
                printf("%s", line);
                lines_count++;
            }
        } else {
            print_error("No such file or directory.");
            return;
        }
    } else {
        print_error("Wrong format for head.");
        return;
    }
}

void builtin_cd(char *command[]) {
    if (command[1] == NULL || !strcmp(command[1], "~")) {
        getcwd(prev, sizeof(prev));
        chdir(getenv("HOME"));
    } else if (!strcmp(command[1], "-")) {
        char temp_prev[LINE_LENGTH];
        getcwd(temp_prev, sizeof(prev));
        chdir(prev);
        strcpy(prev, temp_prev);
    } else {
        char temp_prev[LINE_LENGTH];
        getcwd(temp_prev, sizeof(prev));
        if (chdir(command[1]) == 0) {
            strcpy(prev, temp_prev);
        } else {
            print_error("No such file or directory.");
            return;
        }
    }
}

void builtin_help() {
    printf("           _                    _          _ _\n"
           " _ __ ___ (_) ___ _ __ ___  ___| |__   ___| | |\n"
           "| '_ ` _ \\| |/ __| '__/ _ \\/ __| '_ \\ / _ \\ | |\n"
           "| | | | | | | (__| | | (_) \\__ \\ | | |  __/ | |\n"
           "|_| |_| |_|_|\\___|_|  \\___/|___/_| |_|\\___|_|_|\n\n");
    printf("My name is Szymon Bartanowicz and I'm the creator of this microshell.\n");
    printf("version: 1.0\n");
    printf("builtin commands:\n\n");
    printf("\033[1;96m");
    int i;
    for (i = 0; i < 6; ++i) {
        printf("%s\n", builtins[i]);
        if (!strcmp(builtins[i], "cd")) {
            printf("\033[0m");
            printf("\tcd <dir>\n\tcd ~\n\tcd -\n");
        }
        if (!strcmp(builtins[i], "head")) {
            printf("\033[0m");
            printf("\thead -<lines> <file>\n\thead -n<lines> <file>\n\thead -n <lines> <file>\n");
        }
        if (!strcmp(builtins[i], "grep")) {
            printf("\033[0m");
            printf("\tgrep <arg> <file>\n\tgrep \"<args>\" <file>\n");
        }
        printf("\033[1;96m");
    }
    printf("\033[0m");
    printf("\n");
}

void builtin_exit() {
    printf("Bye!\n");
    printf("Thanks for using my microshell.\n");
    exit(0);
}

void run_child_process(char *command[]) {
    int status;
    pid_t child_pid = fork();

    if (child_pid == 0) {
        execvp(command[0], command);
        printf("\033[1;91m");
        print_error("Error occurred running child process.");
    } else if (child_pid > 0) {
        waitpid(child_pid, &status, WUNTRACED);
    }
}

void run_builtin(char *command[], int words_count) {
    if (!strcmp(command[0], "cd")) builtin_cd(command);
    else if (!strcmp(command[0], "help")) builtin_help();
    else if (!strcmp(command[0], "exit")) builtin_exit();
    else if (!strcmp(command[0], "head")) builtin_head(command, words_count);
    else if (!strcmp(command[0], "grep")) builtin_grep(command, words_count);
    else if (!strcmp(command[0], "fancy_time")) builtin_fancy_time();
}

bool is_builtin(char *name) {
    int index = 0;

    while (builtins[index]) {
        if (strcmp(builtins[index], name) == 0) {
            return true;
        }

        index++;
    }

    return false;
}

void prompt() {
    char cwd[LINE_LENGTH];
    getcwd(cwd, sizeof(cwd));
    printf("\033[1;91m");
    display_time();
    printf("[");
    printf("\033[1;93m");
    printf("%s", getenv("USER"));
    printf("\033[1;95m");
    printf("\033[1;91m");
    printf("@");
    printf("\033[1;92m");
    printf("%s", cwd);
    printf("\033[1;91m");
    printf("] $");
    printf("\033[0m");
}

void parse_line(char line[]) {
    enter = false;

    char *command[LINE_LENGTH];
    char *parsed;
    int index = 0;

    parsed = strtok(line, " \t\r\n\a");

    while (parsed != NULL) {
        command[index] = parsed;
        index++;
        parsed = strtok(NULL, " \t\r\n\a");
    }

    command[index] = NULL;

    if (command[0]) {
        if (is_builtin(command[0])) {
            run_builtin(command, index);
        } else {
            run_child_process(command);
        }
    }

    prompt();

}

void read_line() {
    int c;
    c = getchar();
    int i = 0;
    char *command = malloc(sizeof(char) * LINE_LENGTH);
    if (c != '\n') {
        command[0] = c;
        i++;
    } else {
        prompt();
        return;
    }

    while (c != '\n') {
        c = getchar();
        if (c != '\n') {
            command[i] = c;
            i++;
        } else {
            enter = true;
        }
    }

    parse_line(command);
}

int main() {
    prompt();

    while (!enter) {
        read_line();
    }

    return 0;
}
