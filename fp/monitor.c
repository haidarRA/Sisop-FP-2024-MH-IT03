#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <libgen.h>
#include <crypt.h>
#include <sys/inotify.h>

#define SALT "$2a$10$KT2H8E35L3wNleZjpugbqO.Nc/TMOMEmABBs15Sxv7XleUTummLmi"
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

int directory_exists(const char *path) {
    struct stat stats;
    stat(path, &stats);
    return S_ISDIR(stats.st_mode);
}

char* get_path() {
    static char path[200];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return dirname(path);
    } else {
        perror("Error getting executable path");
        exit(EXIT_FAILURE);
    }
}

char* split_comma(char* str, int index) {
    char* token;
    char* rest = str;
    int count = 0;
    while ((token = strtok_r(rest, ",", &rest))) {
        count++;
        if (count == index) {
            return token;
        }
    }
    return NULL;
}

void remove_lastdir(char *path) {
    int length = strlen(path);
    if (length == 0 || (length == 1 && path[0] == '/')) {
        return;
    }
    int i;
    for (i = length - 1; i >= 0; i--) {
        if (path[i] == '/') {
            break;
        }
    }
    if (i < 0) {
        return;
    }
    if (i == length - 1) {
        path[i] = '\0';
    } else {
        path[i] = '\0';
    }
}

int count_spaces(char *str) {
    int count = 0;
    for(int i = 0; str[i] != '\0'; i++) {
        if(str[i] == ' ') {
            count++;
        }
    }
    return count;
}

void split_string(const char* input, char* parts[], const char* delimiter) {
    char* temp = strdup(input);
    char* token;
    int i = 0;
    token = strtok(temp, delimiter);
    while (token != NULL && i < 5) {
        parts[i] = strdup(token);
        token = strtok(NULL, delimiter);
        i++;
    }
    free(temp);
}

void split_view(char* str, char** c1, char** c2, char** c3, char** c4) {
    *c1 = strtok(str, ",");
    *c2 = strtok(NULL, ",");
    *c3 = strtok(NULL, ",");
    *c4 = strtok(NULL, ",");
}

void print_chat(const char *chatpath) {
    FILE *fchat = fopen(chatpath, "r");
    if (fchat == NULL) {
        perror("Error opening chat file");
        return;
    }

    char line[1024];
    char *chat_date, *chat_id, *chat_username, *chat_content;
    char result[2048] = "";
    char mod_line[256];

    while (fgets(line, sizeof(line), fchat)) {
        split_view(line, &chat_date, &chat_id, &chat_username, &chat_content);
        chat_content[strcspn(chat_content, "\n")] = '\0';
        sprintf(mod_line, "[%s][%s][%s] %s", chat_date, chat_id, chat_username, chat_content);
        strcat(result, mod_line);
        strcat(result, "\n");
    }

    fclose(fchat);
    printf("%s", result);
}

int main(int argc, char *argv[]) {
    char* u_path = get_path();

    char dcpath[256];
    sprintf(dcpath, "%s/DiscorIT", u_path);

    char userpath[256];
    sprintf(userpath, "%s/users.csv", dcpath);

    char line[1024];

    if (argc < 5) {
        fprintf(stderr, "Usage: %s LOGIN <username> <password>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "LOGIN") == 0) {
        FILE *fuser = fopen(userpath, "r");
        if (fuser == NULL) {
            perror("Error opening user file");
            return EXIT_FAILURE;
        }

        char *bpass1, *bpass2, rank[20], check_line[256], check_rank[256];
        int user_exists = 0;
        while (fgets(line, sizeof(line), fuser)) {
            if (strstr(line, argv[2])) {
                strcpy(check_line, line);
                user_exists = 1;
                break;
            }
        }

        fclose(fuser);

        if (!user_exists) {
            printf("%s gagal login\n", argv[2]);
            return EXIT_FAILURE;
        }

        char *check_pass = argv[4];
        check_pass[strcspn(check_pass, "\n")] = '\0';
        bpass2 = crypt(check_pass, SALT);
        bpass1 = split_comma(check_line, 3);

        if (strcmp(bpass1, bpass2) == 0) {
            printf("%s berhasil login\n", argv[2]);

            fuser = fopen(userpath, "r");
            if (fuser == NULL) {
                perror("Error opening user file");
                return EXIT_FAILURE;
            }

            while (fgets(line, sizeof(line), fuser)) {
                if (strstr(line, argv[2])) {
                    strcpy(check_rank, line);
                    sprintf(rank, "%s", split_comma(check_rank, 4));
                    break;
                }
            }

            fclose(fuser);

            char input[125], right[60];
            sprintf(right, "[%s]", argv[2]);

            int channel_exists = 0, room_exists = 0;
            char channel[40], room[40];

            while (true) {
                printf("%s ", right);
                fgets(input, 125, stdin);
                input[strcspn(input, "\n")] = '\0';

                if ((strstr(input, "-channel")) && (strstr(input, "-room"))) {
                    char *splitted[4];
                    split_string(input, splitted, " ");

                    char chpath[256];
                    sprintf(chpath, "%s/channels.csv", dcpath);

                    FILE *fch = fopen(chpath, "r");
                    if (fch == NULL) {
                        perror("Error opening channels file");
                        continue;
                    }

                    while (fgets(line, sizeof(line), fch)) {
                        if (strstr(line, splitted[1])) {
                            channel_exists = 1;
                            break;
                        }
                    }

                    fclose(fch);

                    if (!channel_exists) {
                        printf("Channel tidak ditemukan\n");
                        continue;
                    }

                    char chdir[256];
                    sprintf(chdir, "%s/%s", dcpath, splitted[1]);
                    struct dirent *de;
                    DIR *dr = opendir(chdir);

                    if (dr == NULL) {
                        perror("Could not open directory");
                        continue;
                    }

                    while ((de = readdir(dr)) != NULL) {
                        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, "admin") == 0) {
                            continue;
                        }
                        if (strcmp(de->d_name, splitted[3]) == 0) {
                            room_exists = 1;
                            break;
                        }
                    }

                    closedir(dr);

                    if (channel_exists == 1 && room_exists == 1) {
                        char chatpath[256];
                        sprintf(chatpath, "%s/%s/%s/chat.csv", dcpath, splitted[1], splitted[3]);

			                  printf("\e[1;1H\e[2J");
                        print_chat(chatpath);

                        int inotifyFd = inotify_init();
                        if (inotifyFd == -1) {
                            perror("inotify_init");
                            continue;
                        }

                        int wd = inotify_add_watch(inotifyFd, chatpath, IN_MODIFY);
                        if (wd == -1) {
                            perror("inotify_add_watch");
                            close(inotifyFd);
                            continue;
                        }

                        fd_set fds;
                        FD_ZERO(&fds);
                        FD_SET(STDIN_FILENO, &fds);
                        FD_SET(inotifyFd, &fds);
                        int maxfd = (STDIN_FILENO > inotifyFd ? STDIN_FILENO : inotifyFd) + 1;

                        while (true) {
                            fd_set rfds = fds;
                            int retval = select(maxfd, &rfds, NULL, NULL, NULL);
                            if (retval == -1) {
                                perror("select");
                                break;
                            }

                            if (FD_ISSET(STDIN_FILENO, &rfds)) {
                                fgets(input, 125, stdin);
                                input[strcspn(input, "\n")] = '\0';
                                if (strcmp(input, "EXIT") == 0) {
                                    break;
                                } 
                                else {
                                    printf("Invalid command\n");
                                }
                            }

                            if (FD_ISSET(inotifyFd, &rfds)) {
                                char buf[BUF_LEN];
                                ssize_t numRead = read(inotifyFd, buf, BUF_LEN);
                                if (numRead == -1) {
                                    perror("read");
                                    break;
                                }
				                        printf("\e[1;1H\e[2J");
                                print_chat(chatpath);
                            }
                        }

                        inotify_rm_watch(inotifyFd, wd);
                        close(inotifyFd);
                    } 
                    else {
                        printf("Channel dan/atau room tidak ditemukan\n");
                    }

                    channel_exists = 0;
                    room_exists = 0;
                } 
                else if (strcmp(input, "EXIT") == 0) {
                    break;
                } 
                else {
                    printf("Invalid command\n");
                }
            }
        } 
        else {
            printf("%s gagal login\n", argv[2]);
        }
    } 
    else {
        printf("Invalid command\n");
    }

    return 0;
}
